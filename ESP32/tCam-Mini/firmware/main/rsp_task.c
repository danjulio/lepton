/*
 * Response Task
 *
 * Implement the response transmission module under control of the command module.
 * Responsible for sending responses to the connected client.  Sources of responses
 * include the command task, lepton task and file task.
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
 * along with firecam.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include "cmd_task.h"
#include "ctrl_task.h"
#include "lep_task.h"
#include "rsp_task.h"
#include "json_utilities.h"
#include "sys_utilities.h"
#include "system_config.h"
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>


//
// RSP Task constants
//

// Uncomment to log various image processing timestamps
//#define LOG_IMG_TIMESTAMP
//#define LOG_PROC_TIMESTAMP
//#define LOG_SEND_TIMESTAMP



//
// RSP Task variables
//
static const char* TAG = "rsp_task";

// State
static bool connected;
static bool stream_on;
static bool image_pending;
static bool got_image_0, got_image_1;

// Stream rate/duration control
static uint32_t next_stream_frame_delay_msec;   // mSec between images; 0 = fast as possible
static uint32_t cur_stream_frame_delay_usec;
static uint32_t next_stream_frame_num;          // Number of frames to stream; 0 = infinite
static uint32_t cur_stream_frame_num;
static uint32_t stream_remaining_frames;        // Remaining frames to stream
static int64_t stream_ready_usec;               // Next ESP32 uSec timestamp to send image

// Command Response buffer (holds single responses from the cmd_task)
static char cmd_task_response_buffer[JSON_MAX_RSP_TEXT_LEN];



//
// RSP Task Forward Declarations for internal functions
//
static void init_state();
static void eval_stream_ready();
static void handle_notifications();
static int process_image(int n);
static void send_response(char* rsp, int len);
static bool cmd_response_available();
static int get_cmd_response();
static char pop_cmd_response_buffer();



//
// RSP Task API
//
void rsp_task()
{
	int len;
	
	ESP_LOGI(TAG, "Start task");
	
	init_state();
	
	while (1) {
		// Evaluate streaming conditions for ready to send image if enabled before
		// handling notifications (of images from lep_task)
		if (stream_on) {
			eval_stream_ready();
		}
		
		// Process notifications from other tasks
		handle_notifications();
		
		// Get our current connection state
		if (cmd_connected()) {
			connected = true;
		} else if (connected) {
			// Clear our state since we are no longer connected
			init_state();
		}
		
		// Look for things to send
		if (got_image_0 || got_image_1) {
			if (connected) {
				if (got_image_0) {
					len = process_image(0);
					got_image_0 = false;
#ifdef LOG_IMG_TIMESTAMP
					ESP_LOGI(TAG, "process image 0");
#endif
				} else {
					len = process_image(1);
					got_image_1 = false;
#ifdef LOG_IMG_TIMESTAMP
					ESP_LOGI(TAG, "process image 1");
#endif
				}	
					
				// Send the image
				if (len != 0) {
					send_response(sys_image_rsp_buffer.bufferP, sys_image_rsp_buffer.length);
				}
				
				// If streaming, determine if we have sent the required number of images if necessary
				if (stream_on && (cur_stream_frame_num != 0)) {
					if (--stream_remaining_frames == 0) {
						stream_on = false;
					}
				}
			}
		}
		
		if (cmd_response_available()) {
			// Get the command response and send it if possible
			len = get_cmd_response();
			if (connected && (len != 0)) {
				send_response(cmd_task_response_buffer, len);
			}
		}
		
		// Sleep task - less if we are streaming
		if (stream_on) {
			vTaskDelay(pdMS_TO_TICKS(RSP_TASK_EVAL_FAST_MSEC));
		} else {
			vTaskDelay(pdMS_TO_TICKS(RSP_TASK_EVAL_NORM_MSEC));
		}
	} 
}


// Called before sending RSP_NOTIFY_CMD_STREAM_ON_MASK
void rsp_set_stream_parameters(uint32_t delay_ms, uint32_t num_frames)
{
	next_stream_frame_delay_msec = delay_ms;
	next_stream_frame_num = num_frames;
}


//
// Internal functions
//

/**
 * (Re)Initialize
 */
static void init_state()
{
	connected = false;
	stream_on = false;
	next_stream_frame_delay_msec = 0;
	next_stream_frame_num = 0;
	image_pending = false;
	got_image_0 = false;
	got_image_1 = false;
	
	// Flush the command response buffer
	xSemaphoreTake(sys_cmd_response_buffer.mutex, portMAX_DELAY);
	sys_cmd_response_buffer.length = 0;
	sys_cmd_response_buffer.popP = sys_cmd_response_buffer.pushP;
	xSemaphoreGive(sys_cmd_response_buffer.mutex);
}


/**
 * Evaluate stream rate/duration variables to see if it's time to send an image.
 * Assumes stream_on set.
 */
static void eval_stream_ready()
{
	// Determine if we are ready to send the next available image
	if (cur_stream_frame_delay_usec == 0) {
		image_pending = true;
	} else {
		if (esp_timer_get_time() >= stream_ready_usec) {
			image_pending = true;
			stream_ready_usec = stream_ready_usec + cur_stream_frame_delay_usec;
		}
	}
}


/**
 * Handle incoming notifications
 */
static void handle_notifications()
{
	uint32_t notification_value;
	
	notification_value = 0;
	if (xTaskNotifyWait(0x00, 0xFFFFFFFF, &notification_value, 0)) {
		// Handle cmd_task notifications
		if (Notification(notification_value, RSP_NOTIFY_CMD_GET_IMG_MASK)) {
			// Note to process the next received image
			image_pending = true;
			
			// Stop any on-going streaming
			stream_on = false;
		}
		
		if (Notification(notification_value, RSP_NOTIFY_CMD_STREAM_ON_MASK)) {
			// Setup streaming
			cur_stream_frame_delay_usec = next_stream_frame_delay_msec * 1000;
			cur_stream_frame_num = next_stream_frame_num;
			if (cur_stream_frame_num != 0) {
				stream_remaining_frames = cur_stream_frame_num;
			}
			
			// First image is immediate
			stream_ready_usec = esp_timer_get_time();
			image_pending = true;
			
			// Start streaming
			stream_on = true;
		}
		
		if (Notification(notification_value, RSP_NOTIFY_CMD_STREAM_OFF_MASK)) {
			// Stop streaming
			stream_on = false;
		}
		
		// Handle lep_task notifications
		if (Notification(notification_value, RSP_NOTIFY_LEP_FRAME_MASK_0)) {
			if (image_pending) {
				got_image_0 = true;
				image_pending = false;
			}
		}
		if (Notification(notification_value, RSP_NOTIFY_LEP_FRAME_MASK_1)) {
			if (image_pending) {
				got_image_1 = true;
				image_pending = false;
			}
		}
	}
}


/**
 * Convert lepton data in the specified half of the ping-pong buffer into a json record
 * with delimitors for transmission over the network
 */
static int process_image(int n)
{
#ifdef LOG_PROC_TIMESTAMP
	int64_t tb, te;
	
	tb = esp_timer_get_time();
#endif
	
	// Convert the image into a json record
	xSemaphoreTake(rsp_lep_buffer[n].lep_mutex, portMAX_DELAY);
    sys_image_rsp_buffer.length = json_get_image_file_string(sys_image_rsp_buffer.bufferP+1, &rsp_lep_buffer[n]);
    xSemaphoreGive(rsp_lep_buffer[n].lep_mutex);
    
    if ((sys_image_rsp_buffer.length > 0) && (sys_image_rsp_buffer.length < JSON_MAX_IMAGE_TEXT_LEN-2)) {
        // Add the delimitors
        *sys_image_rsp_buffer.bufferP = CMD_JSON_STRING_START;
        *(sys_image_rsp_buffer.bufferP + sys_image_rsp_buffer.length + 1) = CMD_JSON_STRING_STOP;
        sys_image_rsp_buffer.length = sys_image_rsp_buffer.length + 2;
    } else {
        ESP_LOGE(TAG, "Illegal image_json_text for sys_image_rsp_buffer (%d bytes)", sys_image_rsp_buffer.length);
        sys_image_rsp_buffer.length = 0;
	}
	
#ifdef LOG_PROC_TIMESTAMP
	te = esp_timer_get_time();
	ESP_LOGI(TAG, "process_image took %d uSec", (int) (te - tb));
#endif

	return sys_image_rsp_buffer.length;
}


/**
 * Send a response
 */
static void send_response(char* rsp, int rsp_length)
{
	int byte_offset;
	int err;
	int len;
	int sock;
#ifdef LOG_SEND_TIMESTAMP
	int64_t tb, te;
	
	tb = esp_timer_get_time();
#endif
	
	sock = cmd_get_socket();
	
	// Write our response to the socket
    byte_offset = 0;
	while (byte_offset < rsp_length) {
		len = rsp_length - byte_offset;
		if (len > RSP_MAX_TX_PKT_LEN) len = RSP_MAX_TX_PKT_LEN;
		err = send(sock, rsp + byte_offset, len, 0);
		if (err < 0) {
			ESP_LOGE(TAG, "Error in socket send: errno %d", errno);
			break;
		}
		byte_offset += err;
	}
	
#ifdef LOG_SEND_TIMESTAMP
	te = esp_timer_get_time();
	ESP_LOGI(TAG, "send_response took %d uSec", (int) (te - tb));
#endif
}


/**
 * Atomically check if there is a response from cmd_task to transmit and load our global
 * cmd_response_length variable with its length
 */
static bool cmd_response_available()
{
	int len;
	
	xSemaphoreTake(sys_cmd_response_buffer.mutex, portMAX_DELAY);
	len = sys_cmd_response_buffer.length;
	xSemaphoreGive(sys_cmd_response_buffer.mutex);
	
	return (len != 0);
}


/**
 * Load our cmd_task_response_buffer w and atomically update the command response buffer
 * indicating we popped a response
 */
static int get_cmd_response()
{
	char c;
	int len = 0;
	
	// Pop an entire delimited json string
	do {
		c = pop_cmd_response_buffer();
		cmd_task_response_buffer[len++] = c;
	} while ((c != CMD_JSON_STRING_STOP) && (len <= JSON_MAX_RSP_TEXT_LEN));
	
	// Atomically update cmd_task_response_buffer
	xSemaphoreTake(sys_cmd_response_buffer.mutex, portMAX_DELAY);
	if (len > JSON_MAX_RSP_TEXT_LEN) {
		// Didn't find complete json string so flush the queue
		sys_cmd_response_buffer.length = 0;
		sys_cmd_response_buffer.popP = sys_cmd_response_buffer.pushP;
		len = 0;
	} else {
		// Subtract the length of the data we popped
		sys_cmd_response_buffer.length = sys_cmd_response_buffer.length - len;
	}
	xSemaphoreGive(sys_cmd_response_buffer.mutex);
	
	return len;
}


/**
 * Pop a character from the command response buffer
 */
static char pop_cmd_response_buffer()
{
	char c;
	
	c = *sys_cmd_response_buffer.popP;
	
	if (++sys_cmd_response_buffer.popP >= (sys_cmd_response_buffer.bufferP + CMD_RESPONSE_BUFFER_LEN)) {
		sys_cmd_response_buffer.popP = sys_cmd_response_buffer.bufferP;
	}
	
	return c;
}


