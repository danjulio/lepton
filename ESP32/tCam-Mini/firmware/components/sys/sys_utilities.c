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
 * along with tCam.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include "esp_system.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/spi_master.h"
#include "driver/spi_slave.h"
#include "json_utilities.h"
#include "ps_utilities.h"
#include "sys_utilities.h"
#include "time_utilities.h"
#include "wifi_utilities.h"
#include "i2c.h"
#include "vospi.h"



//
// System Utilities internal constants
//
#define SPI_SLAVE_TIMEOUT_MSEC 1000



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
char* rx_circular_buffer;                          // Used by cmd_utilities for incoming json data
char* json_cmd_string;                             // Used by cmd_utilities to hold a parsed incoming json command
json_image_string_t sys_image_rsp_buffer;          // Used by rsp_task for json formatted image data
json_cmd_response_queue_t sys_cmd_response_buffer; // Loaded by cmd_task with json formatted response data

// Firmware update segment (located in internal DRAM)
uint8_t fw_upd_segment[FM_UPD_CHUNK_MAX_LEN];      // Loaded by cmd_utilities for consumption in rsp_task

//
// Flag indicating SPI slave transaction is outstanding
//
static bool spi_slave_busy = false;
static spi_slave_transaction_t spi_slave_t;


//
// Forward declarations for internal functions
//
static void IRAM_ATTR _sys_spi_slave_post_setup_cb();
static void IRAM_ATTR _sys_spi_slave_post_trans_cb();


//
// System Utilities API
//

/**
 * Initialize the ESP32 GPIO and internal peripherals
 */
bool system_esp_io_init(bool ser_mode)
{
	ESP_LOGI(TAG, "ESP32 Peripheral Initialization");	
	
	// Attempt to initialize the I2C Master
	if (i2c_master_init() != ESP_OK) {
		ESP_LOGE(TAG, "I2C Master initialization failed");
		return false;
	}
	
	// Attempt to initialize the SPI Master used by the lep_task
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
	
	if (ser_mode) {
		// Attempt to initialize the SPI Slave used by rsp_task
		spi_bus_config_t spi_buscfg2 = {
			.mosi_io_num=-1,
			.miso_io_num=HOST_MISO_IO,
			.sclk_io_num=HOST_SCK_IO,
			.quadwp_io_num=-1,
	        .quadhd_io_num=-1,
			.max_transfer_sz=JSON_MAX_IMAGE_TEXT_LEN
		};
		
		spi_slave_interface_config_t spi_slvcfg={
			.mode=HOST_SPI_MODE,
			.spics_io_num=HOST_CSN_IO,
			.queue_size=1,
			.flags=0,
			.post_setup_cb=_sys_spi_slave_post_setup_cb,
			.post_trans_cb=_sys_spi_slave_post_trans_cb
    	};
    	
    	if (spi_slave_initialize(HOST_SPI_HOST, &spi_buscfg2, &spi_slvcfg, HOST_DMA_NUM) != ESP_OK) {
    		ESP_LOGE(TAG, "SPI Slave initialization failed");
			return false;
    	}
	}
	
	return true;
}


/**
 * Initialize the board-level peripheral subsystems
 */
bool system_peripheral_init(bool ser_mode)
{
	ESP_LOGI(TAG, "System Peripheral Initialization");
	
	time_init();
	
	if (!ps_init(ser_mode)) {
		ESP_LOGE(TAG, "Persistent Storage initialization failed");
		return false;
	}

	// Get the initial lepton configuration	
	ps_get_lep_state(&lep_st);
	
	if (!ser_mode) {
		if (!wifi_init()) {
			ESP_LOGE(TAG, "WiFi initialization failed");
			return false;
		}
	}
	
	// Initialize the Lepton GPIO and then reset the Lepton
	// (reset handles potential external crystal oscillator slow start-up)
	gpio_set_direction(LEP_VSYNC_IO, GPIO_MODE_INPUT);
	gpio_set_direction(LEP_RESET_IO, GPIO_MODE_OUTPUT);
	gpio_set_level(LEP_RESET_IO, 1);
	vTaskDelay(pdMS_TO_TICKS(10));
	gpio_set_level(LEP_RESET_IO, 0);
	
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
	
	// Allocate the incoming command buffers
	rx_circular_buffer = heap_caps_malloc(JSON_MAX_CMD_TEXT_LEN, MALLOC_CAP_SPIRAM);
	if (rx_circular_buffer == NULL) {
		ESP_LOGE(TAG, "malloc rx_circular_buffer failed");
		return false;
	}
	json_cmd_string = heap_caps_malloc(JSON_MAX_CMD_TEXT_LEN, MALLOC_CAP_SPIRAM);
	if (json_cmd_string == NULL) {
		ESP_LOGE(TAG, "malloc json_cmd_string failed");
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
	
	// Allocate the json image text buffer in DMA capable internal memory           
	sys_image_rsp_buffer.bufferP = heap_caps_malloc(JSON_MAX_IMAGE_TEXT_LEN, MALLOC_CAP_DMA);
	if (sys_image_rsp_buffer.bufferP == NULL) {
		ESP_LOGE(TAG, "malloc shared json image text response buffer failed");
		return false;
	}
	
	return true;
}


bool system_config_spi_slave(char* buf, int len)
{
	// Setup the SPI Slave
	spi_slave_t.length=len*8;
    spi_slave_t.tx_buffer=buf;
    spi_slave_t.rx_buffer=NULL;
    
    if (spi_slave_queue_trans(HOST_SPI_HOST, &spi_slave_t, pdMS_TO_TICKS(SPI_SLAVE_TIMEOUT_MSEC)) == ESP_OK) {
    	return true;
    }
    
    return false;
}


bool system_spi_slave_busy()
{
	return spi_slave_busy;
}


void system_spi_wait_done()
{
	spi_slave_transaction_t *t;
	
	// Have to call driver to get the result
	t = &spi_slave_t;
	(void) spi_slave_get_trans_result(HOST_SPI_HOST, &t, 1);
}



//
// Internal functions
//
static void IRAM_ATTR _sys_spi_slave_post_setup_cb()
{
	spi_slave_busy = true;
}


static void IRAM_ATTR _sys_spi_slave_post_trans_cb()
{
	spi_slave_busy = false;
}
