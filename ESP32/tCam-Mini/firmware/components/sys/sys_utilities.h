/*
 * System related utilities
 *
 * Contains functions to initialize the system, other utility functions, a set
 * of globally available handles for the various tasks (to use for task notifications).
 *
 * Copyright 2020-2022 Dan Julio
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
#ifndef SYS_UTILITIES_H
#define SYS_UTILITIES_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "system_config.h"
#include <stdbool.h>
#include <stdint.h>


//
// System Utilities constants
//

// Gain mode
#define SYS_GAIN_HIGH 0
#define SYS_GAIN_LOW  1
#define SYS_GAIN_AUTO 2



//
// System Utilities typedefs
//
typedef struct {
	bool telem_valid;
	uint16_t lep_min_val;
	uint16_t lep_max_val;
	uint16_t* lep_bufferP;
	uint16_t* lep_telemP;
	SemaphoreHandle_t lep_mutex;
} lep_buffer_t;

typedef struct {
	uint32_t length;
	char* bufferP;
} json_image_string_t;

typedef struct {
	int length;
	char* pushP;
	char* popP;
	char* bufferP;
	SemaphoreHandle_t mutex;
} json_cmd_response_queue_t;

typedef struct {
	bool agc_set_enabled;        // Set when agc_enabled
	int emissivity;              // Integer percent 1 - 100
	int gain_mode;               // SYS_GAIN_HIGH / SYS_GAIN_LOW / SYS_GAIN_AUTO
} json_config_t;



//
// System Utilities macros
//
#define Notification(var, mask) ((var & mask) == mask)



//
// Task handle externs for use by tasks to communicate with each other
//
extern TaskHandle_t task_handle_cmd;
extern TaskHandle_t task_handle_ctrl;
extern TaskHandle_t task_handle_lep;
extern TaskHandle_t task_handle_rsp;
#ifdef INCLUDE_SYS_MON
extern TaskHandle_t task_handle_mon;
#endif



//
// Lepton configuration state
//
extern json_config_t lep_st;

//
// Global buffer pointers for allocated memory
//

// Shared memory data structures
extern lep_buffer_t rsp_lep_buffer[2];   // Ping-pong buffer loaded by lep_task for rsp_task

// Big buffers
extern char* rx_circular_buffer;                          // Used by cmd_utilities for incoming json data
extern char* json_cmd_string;                             // Used by cmd_utilities to hold a parsed incoming json command
extern json_image_string_t sys_image_rsp_buffer;          // Used by rsp_task for json formatted image data
extern json_cmd_response_queue_t sys_cmd_response_buffer; // Loaded by cmd_task with json formatted response data

// Firmware update segment
extern uint8_t fw_upd_segment[];                          // Loaded by cmd_utilities for consumption in rsp_task



//
// System Utilities API
//
bool system_esp_io_init(int brd_type, int if_mode);
bool system_peripheral_init(int brd_type, int if_mode);
bool system_buffer_init();
bool system_config_spi_slave(char* buf, int len);
bool system_spi_slave_busy();
bool system_spi_wait_done();

#define system_get_lep_st()   (&lep_st)
 
#endif /* SYS_UTILITIES_H */