/*
 * Response Task
 *
 * Implement the response transmission module under control of the command module.
 * Responsible for sending responses to the connected client.  Sources of responses
 * include the command task, lepton task and file task.
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
#include "net_cmd_task.h"
#include "sif_cmd_task.h"
#include "ctrl_task.h"
#include "lep_task.h"
#include "rsp_task.h"
#include "cmd_utilities.h"
#include "json_utilities.h"
#include "sif_utilities.h"
#include "sys_utilities.h"
#include "upd_utilities.h"
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
//#define LOG_SIF_SEND


// FW update state
#define FW_UPD_IDLE    0
#define FW_UPD_REQUEST 1
#define FW_UPD_PROCESS 2



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

// cam_info json string temporary buffer
static SemaphoreHandle_t cam_info_mutex;
static char cam_info_string[JSON_MAX_RSP_TEXT_LEN];

// Command Response buffer (holds single responses from the cmd_task)
static char cmd_task_response_buffer[JSON_MAX_RSP_TEXT_LEN];

// Firmware update control
static char fw_update_version[UPD_MAX_VER_LEN+1];
static int fw_update_state;
static int fw_update_wait_timer;                // Counts down eval intervals waiting for some operation
static int fw_req_length;
static int fw_req_attempt_num;
static int fw_cur_loc;
static int fw_seg_start;
static int fw_seg_length;



//
// RSP Task Forward Declarations for internal functions
//
static void init_state();
static void eval_stream_ready();
static void handle_notifications();
static int process_image(int n);
static void send_response(char* rsp, int len, bool ser_mode);
static bool cmd_response_available();
static int get_cmd_response();
static char pop_cmd_response_buffer();
static void send_spi_image(char* rsp, int rsp_length);
static void send_get_fw();



//
// RSP Task API
//
void rsp_task()
{
	int len;
	int brd_type;
	int if_type;
	
	ESP_LOGI(TAG, "Start task");
	
	//
	// Initialize
	//
	init_state();
	
	ctrl_get_if_mode(&brd_type, &if_type);
	
	cam_info_mutex = xSemaphoreCreateMutex();
	
	//
	// Task loop
	//
	while (1) {
		// Evaluate streaming conditions for ready to send image if enabled before
		// handling notifications (of images from lep_task)
		if (stream_on) {
			eval_stream_ready();
		}
		
		// Process notifications from other tasks
		handle_notifications();
		
		// Get our current wifi connection state if necessary
		if (if_type == CTRL_IF_MODE_SIF) {
			connected = true;
		} else {
			if (net_cmd_connected()) {
				connected = true;
			} else if (connected) {
				// Clear our state since we are no longer connected
				init_state();
			}
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
					if (if_type == CTRL_IF_MODE_SIF) {
						// Configure a SPI slave response if the slave is available,
						// otherwise drop the response
						if (!system_spi_slave_busy()) {
							send_spi_image(sys_image_rsp_buffer.bufferP, sys_image_rsp_buffer.length);
						}
					} else {
						send_response(sys_image_rsp_buffer.bufferP, sys_image_rsp_buffer.length, false);
					}
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
				send_response(cmd_task_response_buffer, len, (if_type == CTRL_IF_MODE_SIF));
			}
		}
		
		if (fw_update_state != FW_UPD_IDLE) {
			// Look for timeout
			if (--fw_update_wait_timer == 0) {
				if (fw_update_state == FW_UPD_REQUEST) {
					// Request timed out without user confirming to start
					xTaskNotify(task_handle_ctrl, CTRL_NOTIFY_FW_UPD_DONE, eSetBits);
					rsp_set_cam_info_msg(RSP_INFO_UPD_STATUS, "Firmware update request timed out");
					ESP_LOGI(TAG, "Firmware update request timed out");
					fw_update_state = FW_UPD_IDLE;
				} else if (fw_update_state == FW_UPD_PROCESS) {
					if (++fw_req_attempt_num < FW_REQ_MAX_ATTEMPTS) {
						// Request the segment again
						send_get_fw();
						fw_update_wait_timer = RSP_MAX_FW_UPD_GET_WAIT_MSEC / RSP_TASK_EVAL_NORM_MSEC;
						ESP_LOGI(TAG, "Retry chunk request");
					} else {
						// Give up
						xTaskNotify(task_handle_ctrl, CTRL_NOTIFY_FW_UPD_DONE, eSetBits);
						rsp_set_cam_info_msg(RSP_INFO_UPD_STATUS, "Host did not respond to multiple chunk requests");
						ESP_LOGE(TAG, "Host did not respond to multiple chunk requests.  Done.");
						upd_early_terminate();
						fw_update_state = FW_UPD_IDLE;
					}
				}
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


void rsp_set_cam_info_msg(uint32_t info_value, char* info_string)
{
	int i;
	int len;
	
	xSemaphoreTake(cam_info_mutex, portMAX_DELAY);
	
	// Create the cam_info json string
	len = json_get_cam_info(cam_info_string, info_value, info_string);
	
	// Atomically load cmd_task_response_buffer
	xSemaphoreTake(sys_cmd_response_buffer.mutex, portMAX_DELAY);
	
	// Only load if there's room for this response
	if (len <= (CMD_RESPONSE_BUFFER_LEN - sys_cmd_response_buffer.length)) {
		for (i=0; i<len; i++) {
			// Push data
			*sys_cmd_response_buffer.pushP = cam_info_string[i];
			
			// Increment push pointer
			if (++sys_cmd_response_buffer.pushP >= (sys_cmd_response_buffer.bufferP + CMD_RESPONSE_BUFFER_LEN)) {
				sys_cmd_response_buffer.pushP = sys_cmd_response_buffer.bufferP;
			}
		}
		
		sys_cmd_response_buffer.length += len;
	}
	
	xSemaphoreGive(sys_cmd_response_buffer.mutex);
	
	xSemaphoreGive(cam_info_mutex);
}


// Called before sending RSP_NOTIFY_FW_UPD_REQ_MASK
void rsp_set_fw_upd_req_info(uint32_t length, char* version)
{
	fw_req_length = length;
	strncpy(fw_update_version, version, UPD_MAX_VER_LEN);
}


// Called before sending RSP_NOTIFY_FW_UPD_SEG_MASK
void rsp_set_fw_upd_seg_info(uint32_t start, uint32_t length)
{
	fw_seg_start = start;
	fw_seg_length = length;
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
	fw_update_state = FW_UPD_IDLE;
	
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
		//
		// Handle cmd_task notifications
		//
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
			stream_remaining_frames = next_stream_frame_num;
			
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
		
		//
		// Handle lep_task notifications
		//
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
		
		//
		// Handle firmware update notifications
		//
		if (Notification(notification_value, RSP_NOTIFY_FW_UPD_REQ_MASK)) {
			// Disable streaming if it is running
			stream_on = false;
			
			// Indicate to the user a fw udpate has been requested
			xTaskNotify(task_handle_ctrl, CTRL_NOTIFY_FW_UPD_REQ, eSetBits);
			
			// Set our state and a timer (for the user to allow the update)
			fw_update_wait_timer = RSP_MAX_FW_UPD_REQ_WAIT_MSEC / RSP_TASK_EVAL_NORM_MSEC;
			fw_update_state = FW_UPD_REQUEST;
			
			ESP_LOGI(TAG, "Request update to v%s : %d bytes", fw_update_version, fw_req_length);
		}
		
		if (Notification(notification_value, RSP_NOTIFY_FW_UPD_SEG_MASK)) {
			if (fw_update_state == FW_UPD_PROCESS) {
				// Ignore duplicate updates (ignore anything but the expected packet)
				if (fw_seg_start == fw_cur_loc) {
					if (upd_process_bytes(fw_seg_start, fw_seg_length, fw_upd_segment)) {
						// Update our count and check for termination
						fw_cur_loc += fw_seg_length;
						if (fw_cur_loc >= fw_req_length) {
							// Done: Attempt to validate and commit the update in flash
							if (upd_complete()) {
								// Flash updated: Let the host know and reboot
								rsp_set_cam_info_msg(RSP_INFO_UPD_STATUS, "Firmware update success");
								ESP_LOGI(TAG, "Firmware update success");
								xTaskNotify(task_handle_ctrl, CTRL_NOTIFY_FW_UPD_DONE, eSetBits);
								xTaskNotify(task_handle_ctrl, CTRL_NOTIFY_FW_UPD_REBOOT, eSetBits);
							} else {
								// Flash update failed: Let host know and start error indication
								rsp_set_cam_info_msg(RSP_INFO_UPD_STATUS, "Firmware update validation failed");
								ESP_LOGE(TAG, "Firmware update validation failed");
								ctrl_set_fault_type(CTRL_FAULT_FW_UPDATE);
								xTaskNotify(task_handle_ctrl, CTRL_NOTIFY_FAULT, eSetBits);
							}
							fw_update_state = FW_UPD_IDLE;
							
						} else {
							// Request the next segment
							fw_req_attempt_num = 0;
							send_get_fw();
							fw_update_wait_timer = RSP_MAX_FW_UPD_GET_WAIT_MSEC / RSP_TASK_EVAL_NORM_MSEC;
							ESP_LOGI(TAG, "Request fw chunk @ %d", fw_cur_loc);
						}
					} else {
						// Flash update failed: Let host know and start error indication
						rsp_set_cam_info_msg(RSP_INFO_UPD_STATUS, "Firmware update flash update failed");
						ESP_LOGE(TAG, "Firmware update flash update failed");
						ctrl_set_fault_type(CTRL_FAULT_FW_UPDATE);
						xTaskNotify(task_handle_ctrl, CTRL_NOTIFY_FAULT, eSetBits);
						upd_early_terminate();
						fw_update_state = FW_UPD_IDLE;
					}
				}
			}
		}
		
		if (Notification(notification_value, RSP_NOTIFY_FW_UPD_EN_MASK)) {
			if (fw_update_state == FW_UPD_REQUEST) {
				// Attempt to setup an update
				if (upd_init(fw_req_length, fw_update_version)) {
					// Indicate to the user a fw update is now in process
					xTaskNotify(task_handle_ctrl, CTRL_NOTIFY_FW_UPD_PROCESS, eSetBits);
				
					// Request first segment / setup timer
					fw_cur_loc = 0;
					fw_req_attempt_num = 0;
					send_get_fw();
					fw_update_wait_timer = RSP_MAX_FW_UPD_GET_WAIT_MSEC / RSP_TASK_EVAL_NORM_MSEC;
					fw_update_state = FW_UPD_PROCESS;
					
					ESP_LOGI(TAG, "Start update");
				} else {
					// Update init failed: Let host know and start error indication
					rsp_set_cam_info_msg(RSP_INFO_UPD_STATUS, "Firmware update flash init failed");
					ESP_LOGE(TAG, "Firmware update flash init failed");
					ctrl_set_fault_type(CTRL_FAULT_FW_UPDATE);
					xTaskNotify(task_handle_ctrl, CTRL_NOTIFY_FAULT, eSetBits);
					fw_update_state = FW_UPD_IDLE;
				}
			}
		}
		
		if (Notification(notification_value, RSP_NOTIFY_FW_UPD_END_MASK)) {
			// Stop the update
			rsp_set_cam_info_msg(RSP_INFO_UPD_STATUS, "Firmware update terminated by user");
			ESP_LOGI(TAG, "Firmware update terminated by user");
			upd_early_terminate();
			fw_update_state = FW_UPD_IDLE;
			
			// Let user know update has stopped
			xTaskNotify(task_handle_ctrl, CTRL_NOTIFY_FW_UPD_DONE, eSetBits);
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
static void send_response(char* rsp, int rsp_length, bool ser_mode)
{
	int byte_offset;
	int err;
	int len;
	int sock;
#ifdef LOG_SEND_TIMESTAMP
	int64_t tb, te;
	
	tb = esp_timer_get_time();
#endif
	
	if (ser_mode) {
#ifdef LOG_SIF_SEND
		rsp[rsp_length] = 0;
		ESP_LOGI(TAG, "TX %s", rsp);
#endif
		sif_send(rsp, rsp_length);
	} else {
		sock = net_cmd_get_socket();
		
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
 * Load our cmd_task_response_buffer and atomically update the command response buffer
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


/**
 * Setup the SPI Slave to be read with the image and send an image ready message
 * via the serial interface.
 */
static void send_spi_image(char* rsp, int rsp_length)
{
	char* cP;
	char* eP;
	int dma_length;
	uint32_t cs;
	static bool enabled = true;
	
	// Skip sending any images if the SPI Slave is no longer running
	if (!enabled) return;
	
	// Compute a 32-bit checksum (32-bit sum of all bytes in the image string)
	// and add it to the end of the image
	cP  = rsp;
	eP = rsp + rsp_length;
	cs = 0;
	while (cP < eP) {
		cs += *cP++;
	}
	*(rsp + rsp_length + 0) = (cs >> 24) & 0xFF;
	*(rsp + rsp_length + 1) = (cs >> 16) & 0xFF;
	*(rsp + rsp_length + 2) = (cs >> 8) & 0xFF;
	*(rsp + rsp_length + 3) = cs & 0xFF;
	rsp_length += 4;
	
	// Length (for DMA) must be multiple of 4 bytes
	if (rsp_length & 0x3) {
		// Round up to next 4-byte boundary
		dma_length = (rsp_length + 4) & 0xFFFFFFFC;
	} else {
		dma_length = rsp_length;
	}
	
	// Create the image ready message
	sprintf(cmd_task_response_buffer, "%c{\"image_ready\" : %d}%c", CMD_JSON_STRING_START, rsp_length, CMD_JSON_STRING_STOP);

	if (system_config_spi_slave(rsp, dma_length)) {
		// Wait for the SPI Slave to report busy indicating it is ready
		while (!system_spi_slave_busy()) {};
		
		// Load the image ready message
		send_response(cmd_task_response_buffer, strlen(cmd_task_response_buffer), true);
		
		// Wait for the SPI Slave to complete transferring the data
		if (!system_spi_wait_done()) {
			// Something went wrong with the SPI Slave - probably a timeout and
			// we couldn't successfully reset it.  So we disable its use and
			// attempt to let our user about the failure.
			enabled = false;
			ESP_LOGE(TAG, "SPI Slave restart error");
			rsp_set_cam_info_msg(RSP_INFO_INT_ERROR, "SPI Slave restart error - images disabled");
			ctrl_set_fault_type(CTRL_FAULT_NETWORK);
			xTaskNotify(task_handle_ctrl, CTRL_NOTIFY_FAULT, eSetBits);
		}
	} else {
		enabled = false;
		ESP_LOGE(TAG, "Setup SPI Slave failed");
		rsp_set_cam_info_msg(RSP_INFO_INT_ERROR, "Setup SPI Slave failed - images disabled");
		ctrl_set_fault_type(CTRL_FAULT_NETWORK);
		xTaskNotify(task_handle_ctrl, CTRL_NOTIFY_FAULT, eSetBits);
	}
}


/**
 * Push a get_fw packet into our own queue with the current segment to get
 */
static void send_get_fw()
{
	char* response_buffer;
	int i;
	uint32_t get_fw_length;
	uint32_t response_length;
	
	// Compute the length of this request
	if ((fw_req_length - fw_cur_loc) > FM_UPD_CHUNK_MAX_LEN) {
		get_fw_length = FM_UPD_CHUNK_MAX_LEN;
	} else {
		get_fw_length = fw_req_length - fw_cur_loc;
	}
	
	// Get the json string
	response_buffer = json_get_get_fw(fw_cur_loc, get_fw_length, &response_length);
	
	// Atomically load cmd_task_response_buffer
	xSemaphoreTake(sys_cmd_response_buffer.mutex, portMAX_DELAY);
	
	// Only load if there's room for this response
	if (response_length <= (CMD_RESPONSE_BUFFER_LEN - sys_cmd_response_buffer.length)) {
		for (i=0; i<response_length; i++) {
			// Push data
			*sys_cmd_response_buffer.pushP = response_buffer[i];
			
			// Increment push pointer
			if (++sys_cmd_response_buffer.pushP >= (sys_cmd_response_buffer.bufferP + CMD_RESPONSE_BUFFER_LEN)) {
				sys_cmd_response_buffer.pushP = sys_cmd_response_buffer.bufferP;
			}
		}
		
		sys_cmd_response_buffer.length += response_length;
	}
	
	xSemaphoreGive(sys_cmd_response_buffer.mutex);
}
