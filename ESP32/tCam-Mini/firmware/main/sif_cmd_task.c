/*
 * SIF Cmd Task
 *
 * Implement the command processing module including management of the Serial interface.
 * Designed to be executed if Serial communication is enabled.
 *
 * Copyright 2020-2021 Dan Julio
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
#include "sif_cmd_task.h"
#include "sif_utilities.h"
#include "cmd_utilities.h"
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



//
// SIF CMD Task variables
//
static const char* TAG = "sif_cmd_task";



//
// SIF CMD Task API
//
void sif_cmd_task()
{
	char rx_buffer[256];
	int len;
	
	ESP_LOGI(TAG, "Start task");
	
	// Initialize the serial interface
	sif_init();
	
	// Initialize the command processor
	init_command_processor();
	
	while (1) {
		// Process all incoming data
		while ((len = sif_get(rx_buffer, sizeof(rx_buffer))) != 0) {
			// Store new data
            push_rx_data(rx_buffer, len);
			      	
            // Look for and handle commands
            while (process_rx_data()) {}
		}
		
		vTaskDelay(pdMS_TO_TICKS(SIF_CMD_EVAL_MSEC));
	}
}
