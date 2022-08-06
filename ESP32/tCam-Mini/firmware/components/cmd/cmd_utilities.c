/*
 * Command related utilities for use by either wifi_cmd_task or ser_cmd_task.
 *
 * Includes functions to decode and execute commands.
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

#include "cmd_utilities.h"
#include "cci.h"
#include "rsp_task.h"
#include "json_utilities.h"
#include "lepton_utilities.h"
#include "net_utilities.h"
#include "ps_utilities.h"
#include "sys_utilities.h"
#include "time_utilities.h"
#include "upd_utilities.h"
#include "system_config.h"
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "mdns.h"


//
// CMD Utilities private constants
//

// Uncomment to print commands
//#define DEBUG_CMD


//
// CMD Utilities variables
//
static const char* TAG = "cmd_utilities";

// Main receive buffer indicies
static int rx_circular_push_index;
static int rx_circular_pop_index;



//
// CMD Utilities Forward Declarations for internal functions
//
static void process_rx_packet();
static void push_response(char* buf, uint32_t len);
static bool process_set_config(cJSON* cmd_args);
static bool process_set_spotmeter(cJSON* cmd_args);
static bool process_stream_on(cJSON* cmd_args);
static bool process_set_time(cJSON* cmd_args);
static bool process_set_wifi(cJSON* cmd_args);
static bool process_get_lep_cci(cJSON* cmd_args);
static bool process_set_lep_cci(cJSON* cmd_args);
static bool process_fw_upd_request(cJSON* cmd_args);
static bool process_fw_segment(cJSON* cmd_args);
static int in_buffer(char c);



//
// CMD Utilities API
//

/**
 * Initialize variables associated with receiving and processing commands
 */
void init_command_processor()
{
	rx_circular_push_index = 0;
	rx_circular_pop_index = 0;
}


/**
 * Push received data into our circular buffer
 */
void push_rx_data(char* data, int len)
{	
	// Push the received data into the circular buffer
	while (len-- > 0) {
		rx_circular_buffer[rx_circular_push_index] = *data++;
		if (++rx_circular_push_index >= JSON_MAX_CMD_TEXT_LEN) rx_circular_push_index = 0;
	}
}


/**
 * See if we can find a complete json string to process
 */
bool process_rx_data() {
	bool valid_string = false;
	int begin, end, i;
	
	// See if we can find an entire json string
	end = in_buffer(CMD_JSON_STRING_STOP);
	if (end >= 0) {
		// Found end of packet, look for beginning
		begin = in_buffer(CMD_JSON_STRING_START);
		if (begin >= 0) {
			// Found packet - copy it, without delimiters to json_cmd_string
			//
			// Skip past start
			while (rx_circular_pop_index != begin) {
				if (++rx_circular_pop_index >= JSON_MAX_CMD_TEXT_LEN) rx_circular_pop_index = 0;
			}
			
			// Copy up to end
			i = 0;
			while ((rx_circular_pop_index != end) && (i < JSON_MAX_CMD_TEXT_LEN)) {
				if (i < JSON_MAX_CMD_TEXT_LEN) {
					json_cmd_string[i] = rx_circular_buffer[rx_circular_pop_index];
				}
				i++;
				if (++rx_circular_pop_index >= JSON_MAX_CMD_TEXT_LEN) rx_circular_pop_index = 0;
			}
			json_cmd_string[i] = 0;               // Make sure this is a null-terminated string
			
			// Skip past end
			if (++rx_circular_pop_index >= JSON_MAX_CMD_TEXT_LEN) rx_circular_pop_index = 0;
			
			if (i < JSON_MAX_CMD_TEXT_LEN+1) {
				// Process json command string
				process_rx_packet();
				valid_string = true;
			}
		} else {
			// Unexpected end without start - skip it
			while (rx_circular_pop_index != end) {
				if (++rx_circular_pop_index >= JSON_MAX_CMD_TEXT_LEN) rx_circular_pop_index = 0;
			}
		}
	}
	
	return valid_string;
}



//
// CMD Task internal functions
//
static void process_rx_packet()
{
	cJSON* json_obj;
	cJSON* cmd_args;
	int cmd;
	int cmd_success = -1;  // -1: response sent (as ACK), 0: determined elsewhere, 1: success,
						   //  2: fail, 3: unimplemented, 4: unknown cmd, 5: unknown json, 6: bad json
	char cmd_st_buf[80];
	static char* response_buffer;
	static uint32_t response_length;
		
	// Create a json object to parse
	json_obj = json_get_cmd_object(json_cmd_string);
#ifdef DEBUG_CMD
	ESP_LOGI(TAG, "RX %s", json_cmd_string); 
#endif
	if (json_obj != NULL) {
		if (json_parse_cmd(json_obj, &cmd, &cmd_args)) {
#ifdef DEBUG_CMD
			ESP_LOGI(TAG, "cmd %s", json_get_cmd_name(cmd));
#endif
			switch (cmd) {
				case CMD_GET_STATUS:
					response_buffer = json_get_status(&response_length);
					if (response_length != 0) {
						push_response(response_buffer, response_length);
					} else {
						cmd_success = 2;
					}
					break;
					
				case CMD_GET_IMAGE:
					xTaskNotify(task_handle_rsp, RSP_NOTIFY_CMD_GET_IMG_MASK, eSetBits);
					break;
					
				case CMD_SET_TIME:					
					if (process_set_time(cmd_args)) {
						cmd_success = 1;
					} else {
						cmd_success = 2;
					}
					break;
				
				case CMD_GET_WIFI:
					response_buffer = json_get_wifi(&response_length);
					if (response_length != 0) {
						push_response(response_buffer, response_length);
					} else {
						cmd_success = 2;
					}
					break;
					
				case CMD_SET_WIFI:
					if (process_set_wifi(cmd_args)) {
						if ((*net_reinit)()) {
							cmd_success = 1;
						} else {
							ESP_LOGE(TAG, "Could not restart network with the new configuration");
							rsp_set_cam_info_msg(RSP_INFO_CMD_NACK, "Could not restart network with the new configuration");
						}
					} else {
						cmd_success = 2;
					}
					break;
				
				case CMD_GET_CONFIG:
					response_buffer = json_get_config(&response_length);
					if (response_length != 0) {
						push_response(response_buffer, response_length);
					} else {
						cmd_success = 2;
					}
					break;
					
				case CMD_SET_CONFIG:
					if (process_set_config(cmd_args)) {
						cmd_success = 1;
					} else {
						cmd_success = 2;
					}
					break;
				
				case CMD_SET_SPOT:
					if (process_set_spotmeter(cmd_args)) {
						cmd_success = 1;
					} else {
						cmd_success = 2;
					}
					break;
				
				case CMD_STREAM_ON:
					if (process_stream_on(cmd_args)) {
						cmd_success = 1;
					} else {
						cmd_success = 2;
					}
					break;
				
				case CMD_STREAM_OFF:
					xTaskNotify(task_handle_rsp, RSP_NOTIFY_CMD_STREAM_OFF_MASK, eSetBits);
					cmd_success = 1;
					break;
				
				case CMD_RUN_FFC:
					cci_run_ffc();
					cmd_success = 1;
					break;
				
				case CMD_GET_LEP_CCI:
					if (!process_get_lep_cci(cmd_args)) {
						cmd_success = 2;
					}
					break;
				
				case CMD_SET_LEP_CCI:
					if (!process_set_lep_cci(cmd_args)) {
						cmd_success = 2;
					}
					break;
					
				case CMD_FW_UPD_REQ:
					if (process_fw_upd_request(cmd_args)) {
						cmd_success = 1;
					} else {
						cmd_success = 2;
					}
					break;
				
				case CMD_FW_UPD_SEG:
					if (process_fw_segment(cmd_args)) {
						cmd_success = 0; // rsp_task will load success/failed cam_info response
					} else {
						cmd_success = 2;
					}
					break;
				
				case CMD_TAKE_PIC:
				case CMD_RECORD_ON:			
				case CMD_RECORD_OFF:
				case CMD_POWEROFF:
				case CMD_GET_FS_LIST:
				case CMD_GET_FS_FILE:
				case CMD_DEL_FS_OBJ:
				case CMD_DUMP_SCREEN:
					cmd_success = 3;
					break;
				
				default:
					cmd_success = 4;
			}
		} else {
			cmd_success = 5;
		}
		
		json_free_cmd(json_obj);
	} else {
		cmd_success = 6;
	}
	
	// Determine command response to send
	switch (cmd_success) {
		// case -1 does not send message (command response is message)
		// case 0 "determined later" does not send a message at this point
		case 1:
			sprintf(cmd_st_buf, "%s success", json_get_cmd_name(cmd));
			rsp_set_cam_info_msg(RSP_INFO_CMD_ACK, cmd_st_buf);
			break;
		case 2:
			sprintf(cmd_st_buf, "%s failed", json_get_cmd_name(cmd));
			rsp_set_cam_info_msg(RSP_INFO_CMD_NACK, cmd_st_buf);
			break;
		case 3:
			sprintf(cmd_st_buf, "Unsupported command in json string");
			rsp_set_cam_info_msg(RSP_INFO_CMD_UNIMPL, cmd_st_buf);
			break;
		case 4:
			sprintf(cmd_st_buf, "Unknown command in json string");
			rsp_set_cam_info_msg(RSP_INFO_CMD_UNIMPL, cmd_st_buf);
			break;
		case 5:
			sprintf(cmd_st_buf, "Json string wasn't command");
			rsp_set_cam_info_msg(RSP_INFO_CMD_UNIMPL, cmd_st_buf);
			break;
		case 6:
			sprintf(cmd_st_buf, "Couldn't convert json string");
			rsp_set_cam_info_msg(RSP_INFO_CMD_BAD, cmd_st_buf);
			break;
	}
}


/**
 * Push a response into the cmd_task_response_buffer if there is room, otherwise
 * just drop it (up to the external host to make sure this doesn't happen)
 */
static void push_response(char* buf, uint32_t len)
{
	int i;
	
	// Atomically load cmd_task_response_buffer
	xSemaphoreTake(sys_cmd_response_buffer.mutex, portMAX_DELAY);
	
	// Only load if there's room for this response
	if (len <= (CMD_RESPONSE_BUFFER_LEN - sys_cmd_response_buffer.length)) {
		for (i=0; i<len; i++) {
			// Push data
			*sys_cmd_response_buffer.pushP = *(buf+i);
			
			// Increment push pointer
			if (++sys_cmd_response_buffer.pushP >= (sys_cmd_response_buffer.bufferP + CMD_RESPONSE_BUFFER_LEN)) {
				sys_cmd_response_buffer.pushP = sys_cmd_response_buffer.bufferP;
			}
		}
		
		sys_cmd_response_buffer.length += len;
	}
	
	xSemaphoreGive(sys_cmd_response_buffer.mutex);
}


/**
 * Routines to process commands
 */
static bool process_set_config(cJSON* cmd_args)
{
	json_config_t new_config_st;
	
	if (json_parse_set_config(cmd_args, &new_config_st)) {
		// Look for changed items that require updating other modules
		if (new_config_st.agc_set_enabled != lep_st.agc_set_enabled) {
			lepton_agc(new_config_st.agc_set_enabled);
			lep_st.agc_set_enabled = new_config_st.agc_set_enabled;
		}
		if (new_config_st.emissivity != lep_st.emissivity) {
			lepton_emissivity(new_config_st.emissivity);
			lep_st.emissivity = new_config_st.emissivity;
		}
		if (new_config_st.gain_mode != lep_st.gain_mode) {
			lepton_gain_mode(new_config_st.gain_mode);
			lep_st.gain_mode = new_config_st.gain_mode;
		}
		ps_set_lep_state(&lep_st);
		return true;
	}
	
	return false;
}


static bool process_set_spotmeter(cJSON* cmd_args)
{
	uint16_t r1, c1, r2, c2;
	
	if (json_parse_set_spotmeter(cmd_args, &r1, &c1, &r2, &c2)) {
		lepton_spotmeter(r1, c1, r2, c2);
		return true;
	}
	
	return false;
}


static bool process_stream_on(cJSON* cmd_args)
{
	uint32_t delay_ms, num_frames;
	
	if (json_parse_stream_on(cmd_args, &delay_ms, &num_frames)) {
		rsp_set_stream_parameters(delay_ms, num_frames);
		xTaskNotify(task_handle_rsp, RSP_NOTIFY_CMD_STREAM_ON_MASK, eSetBits);
		return true;
	}
	
	return false;
}


static bool process_set_time(cJSON* cmd_args)
{
	tmElements_t te;
	
	if (json_parse_set_time(cmd_args, &te)) {
		time_set(te);
		return true;
	}
	
	return false;
}


static bool process_set_wifi(cJSON* cmd_args)
{
	char ap_ssid[PS_SSID_MAX_LEN+1];
	char sta_ssid[PS_SSID_MAX_LEN+1];
	char ap_pw[PS_PW_MAX_LEN+1];
	char sta_pw[PS_PW_MAX_LEN+1];
	esp_err_t ret;
	net_info_t new_wifi_info;
	
	new_wifi_info.ap_ssid = ap_ssid;
	new_wifi_info.sta_ssid = sta_ssid;
	new_wifi_info.ap_pw = ap_pw;
	new_wifi_info.sta_pw = sta_pw;
	
	if (json_parse_set_wifi(cmd_args, &new_wifi_info)) {
		// Update the mDNS server if we got a new name (ap_ssid)
		if (ps_has_new_cam_name(&new_wifi_info)) {
			ret = mdns_hostname_set(ap_ssid);
			if (ret != ESP_OK) {
				ESP_LOGE(TAG, "Could not set new mDNS hostname %s (%d)", ap_ssid, ret);
			}
		}
		
		// Then update persistent storage
		ps_set_net_info(&new_wifi_info);
		return true;
	}
	
	return false;
}


static bool process_get_lep_cci(cJSON* cmd_args)
{
	char* response_buffer;
	int len;
	uint16_t cmd;
	uint16_t status;
	uint16_t* cci_buf;
	uint32_t response_length;
	
	if (json_parse_get_lep_cci(cmd_args, &cmd, &len, &cci_buf)) {
		cci_get_reg(cmd, len, cci_buf);
		if (cci_command_success(&status)) {
			response_buffer = json_get_cci_response(cmd, len, status, cci_buf, &response_length);
			push_response(response_buffer, response_length);
			return true;
		}
	}
	
	return false;
}


static bool process_set_lep_cci(cJSON* cmd_args)
{
	char* response_buffer;
	int len;
	uint16_t cmd;
	uint16_t status;
	uint16_t* cci_buf;
	uint32_t response_length;
	
	if (json_parse_set_lep_cci(cmd_args, &cmd, &len, &cci_buf)) {
		cci_set_reg(cmd, len, cci_buf);
		
		if (cci_command_success(&status)) {
			// Just return status by setting len = 0
			response_buffer = json_get_cci_response(cmd, 0, status, cci_buf, &response_length);
			push_response(response_buffer, response_length);
			return true;
		}
	}
	
	return false;
}


static bool process_fw_upd_request(cJSON* cmd_args)
{
	char fw_version[UPD_MAX_VER_LEN];
	uint32_t fw_length;
	
	if (json_parse_fw_upd_request(cmd_args, &fw_length, fw_version)) {		
		// Setup rsp_task for an update
		rsp_set_fw_upd_req_info(fw_length, fw_version);
		
		// Notify rsp_task
		xTaskNotify(task_handle_rsp, RSP_NOTIFY_FW_UPD_REQ_MASK, eSetBits);
		
		return true;
	}
	
	return false;
}


static bool process_fw_segment(cJSON* cmd_args)
{
	uint32_t seg_start;
	uint32_t seg_length;
	
	if (json_parse_fw_segment(cmd_args, &seg_start, &seg_length, fw_upd_segment)) {
		// Setup rsp_task for the segment
		rsp_set_fw_upd_seg_info(seg_start, seg_length);
		
		// Notify rsp_task
		xTaskNotify(task_handle_rsp, RSP_NOTIFY_FW_UPD_SEG_MASK, eSetBits);
		
		return true;
	}
	
	return false;
}


/**
 * Look for c in the rx_circular_buffer and return its location if found, -1 otherwise
 */
static int in_buffer(char c)
{
	int i;
	
	i = rx_circular_pop_index;
	while (i != rx_circular_push_index) {
		if (c == rx_circular_buffer[i]) {
			return i;
		} else {
			if (i++ >= JSON_MAX_CMD_TEXT_LEN) i = 0;
		}
	}
	
	return -1;
}


