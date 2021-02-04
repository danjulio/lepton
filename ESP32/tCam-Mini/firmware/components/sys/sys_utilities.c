/*
 * System related utilities
 *
 * Contains functions to initialize the system, other utility functions and a set
 * of globally available handles for the various tasks (to use for task notifications).
 *
 * Copyright 2020 Dan Julio
 *
 * This file is part of tCam.
 *
 * tCam is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * tCam is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with firecam.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include "esp_system.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/spi_master.h"
#include "json_utilities.h"
#include "ps_utilities.h"
#include "sys_utilities.h"
#include "time_utilities.h"
#include "wifi_utilities.h"
#include "i2c.h"
#include "vospi.h"



//
// System Utilities variables
//
static const char* TAG = "sys";


//
// Task handle externs for use by tasks to communicate with each other
//
TaskHandle_t task_handle_cmd;
TaskHandle_t task_handle_ctrl;
TaskHandle_t task_handle_lep;
TaskHandle_t task_handle_rsp;
#ifdef INCLUDE_SYS_MON
TaskHandle_t task_handle_mon;
#endif


//
// Lepton configuration state
//
json_config_t lep_st;


//
// Global buffer pointers for memory allocated in the external SPIRAM
//

// Shared memory data structures
lep_buffer_t rsp_lep_buffer[2];   // Ping-pong buffer loaded by lep_task for rsp_task

// Big buffers
json_image_string_t sys_image_file_buffer;   // Used by file_task for json formatted image data
json_image_string_t sys_image_rsp_buffer;    // Used by rsp_task for json formatted image data

json_cmd_response_queue_t sys_cmd_response_buffer; // Loaded by cmd_task with json formatted response data



//
// System Utilities API
//

/**
 * Initialize the ESP32 GPIO and internal peripherals
 */
bool system_esp_io_init()
{
	ESP_LOGI(TAG, "ESP32 Peripheral Initialization");	
	
	// Attempt to initialize the I2C Master
	if (i2c_master_init() != ESP_OK) {
		ESP_LOGE(TAG, "I2C Master initialization failed");
		return false;
	}
	
	// Attempt to initialize the SPI Master used by the Lepton
	spi_bus_config_t spi_buscfg1 = {
		.miso_io_num=LEP_MISO_IO,
		.mosi_io_num=-1,
		.sclk_io_num=LEP_SCK_IO,
		.max_transfer_sz=LEP_PKT_LENGTH,
		.quadwp_io_num=-1,
		.quadhd_io_num=-1
	};
	if (spi_bus_initialize(LEP_SPI_HOST, &spi_buscfg1, LEP_DMA_NUM) != ESP_OK) {
		ESP_LOGE(TAG, "Lepton Master initialization failed");
		return false;
	}
	
	return true;
}


/**
 * Initialize the board-level peripheral subsystems
 */
bool system_peripheral_init()
{
	ESP_LOGI(TAG, "System Peripheral Initialization");
	
	time_init();
	
	if (!ps_init()) {
		ESP_LOGE(TAG, "Persistent Storage initialization failed");
		return false;
	}

	// Get the initial lepton configuration	
	ps_get_lep_state(&lep_st);
	
	if (!wifi_init()) {
		ESP_LOGE(TAG, "WiFi initialization failed");
		return false;
	}
	
	return true;
}


/**
 * Allocate shared buffers for use by tasks for image data in the external RAM
 */
bool system_buffer_init()
{
	ESP_LOGI(TAG, "Buffer Allocation");
	
	// Allocate the LEP/RSP task lepton frame and telemetry ping-pong buffers
	rsp_lep_buffer[0].lep_bufferP = heap_caps_malloc(LEP_NUM_PIXELS*2, MALLOC_CAP_SPIRAM);
	if (rsp_lep_buffer[0].lep_bufferP == NULL) {
		ESP_LOGE(TAG, "malloc RSP lepton shared image buffer 0 failed");
		return false;
	}
	rsp_lep_buffer[0].lep_telemP = heap_caps_malloc(LEP_TEL_WORDS*2, MALLOC_CAP_SPIRAM);
	if (rsp_lep_buffer[0].lep_telemP == NULL) {
		ESP_LOGE(TAG, "malloc RSP lepton shared telemetry buffer 0 failed");
		return false;
	}
	rsp_lep_buffer[1].lep_bufferP = heap_caps_malloc(LEP_NUM_PIXELS*2, MALLOC_CAP_SPIRAM);
	if (rsp_lep_buffer[1].lep_bufferP == NULL) {
		ESP_LOGE(TAG, "malloc RSP lepton shared image buffer 1 failed");
		return false;
	}
	rsp_lep_buffer[1].lep_telemP = heap_caps_malloc(LEP_TEL_WORDS*2, MALLOC_CAP_SPIRAM);
	if (rsp_lep_buffer[1].lep_telemP == NULL) {
		ESP_LOGE(TAG, "malloc RSP lepton shared telemetry buffer 1 failed");
		return false;
	}
	
	// Create the ping-pong buffer access mutexes
	rsp_lep_buffer[0].lep_mutex = xSemaphoreCreateMutex();
	if (rsp_lep_buffer[0].lep_mutex == NULL) {
		ESP_LOGE(TAG, "create RSP lepton mutex 0 failed");
		return false;
	}
	rsp_lep_buffer[1].lep_mutex = xSemaphoreCreateMutex();
	if (rsp_lep_buffer[1].lep_mutex == NULL) {
		ESP_LOGE(TAG, "create RSP lepton mutex 1 failed");
		return false;
	}
	
	// Allocate the json buffers
	if (!json_init()) {
		ESP_LOGE(TAG, "malloc json buffers failed");
		return false;
	}
	
	// Allocate the outgoing command response json buffer
	sys_cmd_response_buffer.mutex = xSemaphoreCreateMutex();
	sys_cmd_response_buffer.bufferP = heap_caps_malloc(CMD_RESPONSE_BUFFER_LEN, MALLOC_CAP_SPIRAM);
	if (sys_cmd_response_buffer.bufferP == NULL) {
		ESP_LOGE(TAG, "malloc cmd response buffer failed");
		return false;
	}
	sys_cmd_response_buffer.pushP = sys_cmd_response_buffer.bufferP;
	sys_cmd_response_buffer.popP = sys_cmd_response_buffer.bufferP;
	sys_cmd_response_buffer.length = 0;
	
	// Allocate the json image text buffer               
	sys_image_rsp_buffer.bufferP = heap_caps_malloc(JSON_MAX_IMAGE_TEXT_LEN, MALLOC_CAP_SPIRAM);
	if (sys_image_rsp_buffer.bufferP == NULL) {
		ESP_LOGE(TAG, "malloc shared json image text response buffer failed");
		return false;
	}
	
	return true;
}


