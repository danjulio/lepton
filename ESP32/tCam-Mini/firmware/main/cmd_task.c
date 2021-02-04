/*
 * Cmd Task
 *
 * Implement the command processing module including management of the WiFi interface.
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
#include "rsp_task.h"
#include "json_utilities.h"
#include "lepton_utilities.h"
#include "ps_utilities.h"
#include "sys_utilities.h"
#include "time_utilities.h"
#include "wifi_utilities.h"
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
// CMD Task variables
//
static const char* TAG = "cmd_task";

// Client socket
static int client_sock = -1;

// Connected status
static bool connected = false;

// Main receive buffer for incoming packets
static char rx_circular_buffer[CMD_MAX_TCP_RX_BUFFER_LEN];
static int rx_circular_push_index;
static int rx_circular_pop_index;

// json command string buffer
static char json_cmd_string[JSON_MAX_CMD_TEXT_LEN];


//
// CMD Task Forward Declarations for internal functions
//
static void init_command_processor();
static void push_rx_data(char* data, int len);
static bool process_rx_data();
static void process_rx_packet();
static void push_response(char* buf, uint32_t len);
static void process_set_config(cJSON* cmd_args);
static void process_set_spotmeter(cJSON* cmd_args);
static void process_stream_on(cJSON* cmd_args);
static void process_set_time(cJSON* cmd_args);
static void process_set_wifi(cJSON* cmd_args);
static int in_buffer(char c);



//
// CMD Task API
//
void cmd_task()
{
	char rx_buffer[128];
    char addr_str[16];
    int err;
    int flag;
    int len;
    int listen_sock;
    struct sockaddr_in destAddr;
    struct sockaddr_in sourceAddr;
    uint32_t addrLen;
    
	ESP_LOGI(TAG, "Start task");
	
	// Loop to setup socket, wait for connection, handle connection.  Terminates
	// when client disconnects
	
	// Wait until WiFi is connected
	if (!wifi_is_connected()) {
		vTaskDelay(pdMS_TO_TICKS(500));
	}

	// Config IPV4
    destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(CMD_PORT);
    inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);
        
    // socket - bind - listen - accept
    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        goto error;
    }
    ESP_LOGI(TAG, "Socket created");

	flag = 1;
  	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    err = bind(listen_sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
         goto error;
    }
    ESP_LOGI(TAG, "Socket bound");
    
	while (1) {
		init_command_processor();
			
        err = listen(listen_sock, 1);
        if (err != 0) {
            ESP_LOGE(TAG, "Error occured during listen: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket listening");
		
        addrLen = sizeof(sourceAddr);
        client_sock = accept(listen_sock, (struct sockaddr *)&sourceAddr, &addrLen);
        if (client_sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket accepted");
        connected = 1;
		
        // Handle communication with client
        while (1) {
        	len = recv(client_sock, rx_buffer, sizeof(rx_buffer) - 1, MSG_DONTWAIT);
            // Error occured during receiving
            if (len < 0) {
            	if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            		if (wifi_is_connected()) {
            			// Nothing there to receive, so just wait before calling recv again
            			vTaskDelay(pdMS_TO_TICKS(50));
            		} else {
            			ESP_LOGI(TAG, "Closing connection");
            			break;
            		}
            	} else {
                	ESP_LOGE(TAG, "recv failed: errno %d", errno);
                	break;
                }
            }
            // Connection closed
            else if (len == 0) {
                ESP_LOGI(TAG, "Connection closed");
                break;
            }
            // Data received
            else {
            	// Store new data
            	push_rx_data(rx_buffer, len);
        	
            	// Look for and handle commands
            	while (process_rx_data()) {}
            }
        }
        
        // Close this session
        connected = false;
        if (client_sock != -1) {
            ESP_LOGI(TAG, "Shutting down socket and restarting...");
            shutdown(client_sock, 0);
            close(client_sock);
        }
	}

error:
	ESP_LOGI(TAG, "Something went seriously wrong with networking handling - bailing");
	ctrl_set_fault_type(CTRL_FAULT_NETWORK);
	vTaskDelete(NULL);
	// ??? eventually delay for something like 10 seconds to show blinks then reboot
}


/**
 * True when connected to a client
 */
bool cmd_connected()
{
	return connected;
}


/**
 * Return socket descriptor
 */
int cmd_get_socket()
{
	return client_sock;
}



//
// CMD Task internal functions
//

/**
 * Initialize variables associated with receiving and processing commands
 */
static void init_command_processor()
{
	rx_circular_push_index = 0;
	rx_circular_pop_index = 0;
}


/**
 * Push received data into our circular buffer
 */
static void push_rx_data(char* data, int len)
{	
	// Push the received data into the circular buffer
	while (len-- > 0) {
		rx_circular_buffer[rx_circular_push_index] = *data++;
		if (++rx_circular_push_index >= CMD_MAX_TCP_RX_BUFFER_LEN) rx_circular_push_index = 0;
	}
}


/**
 * See if we can find a complete json string to process
 */
static bool process_rx_data() {
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
				if (++rx_circular_pop_index >= CMD_MAX_TCP_RX_BUFFER_LEN) rx_circular_pop_index = 0;
			}
			
			// Copy up to end
			i = 0;
			while ((rx_circular_pop_index != end) && (i < CMD_MAX_TCP_RX_BUFFER_LEN)) {
				if (i < JSON_MAX_CMD_TEXT_LEN) {
					json_cmd_string[i] = rx_circular_buffer[rx_circular_pop_index];
				}
				i++;
				if (++rx_circular_pop_index >= CMD_MAX_TCP_RX_BUFFER_LEN) rx_circular_pop_index = 0;
			}
			json_cmd_string[i] = 0;               // Make sure this is a null-terminated string
			
			// Skip past end
			if (++rx_circular_pop_index >= CMD_MAX_TCP_RX_BUFFER_LEN) rx_circular_pop_index = 0;
			
			if (i < JSON_MAX_CMD_TEXT_LEN+1) {
				// Process json command string
				process_rx_packet();
				valid_string = true;
			}
		} else {
			// Unexpected end without start - skip it
			while (rx_circular_pop_index != end) {
				if (++rx_circular_pop_index >= CMD_MAX_TCP_RX_BUFFER_LEN) rx_circular_pop_index = 0;
			}
		}
	}
	
	return valid_string;
}


static void process_rx_packet()
{
	cJSON* json_obj;
	cJSON* cmd_args;
	int cmd;
	static char* response_buffer;
	static uint32_t response_length;
	
	// Create a json object to parse
	json_obj = json_get_cmd_object(json_cmd_string);
	if (json_obj != NULL) {
		if (json_parse_cmd(json_obj, &cmd, &cmd_args)) {
			switch (cmd) {
				case CMD_GET_STATUS:
					response_buffer = json_get_status(&response_length);
					push_response(response_buffer, response_length);
					break;
					
				case CMD_GET_IMAGE:
					xTaskNotify(task_handle_rsp, RSP_NOTIFY_CMD_GET_IMG_MASK, eSetBits);
					break;
					
				case CMD_SET_TIME:					
					process_set_time(cmd_args);
					break;
				
				case CMD_GET_WIFI:
					response_buffer = json_get_wifi(&response_length);
					push_response(response_buffer, response_length);
					break;
					
				case CMD_SET_WIFI:
					process_set_wifi(cmd_args);
					break;
				
				case CMD_GET_CONFIG:
					response_buffer = json_get_config(&response_length);
					push_response(response_buffer, response_length);
					break;
					
				case CMD_SET_CONFIG:
					process_set_config(cmd_args);
					break;
				
				case CMD_SET_SPOT:
					process_set_spotmeter(cmd_args);
					break;
				
				case CMD_STREAM_ON:
					process_stream_on(cmd_args);
					break;
				
				case CMD_STREAM_OFF:
					xTaskNotify(task_handle_rsp, RSP_NOTIFY_CMD_STREAM_OFF_MASK, eSetBits);
					break;
						
				case CMD_RECORD_ON:			
				case CMD_RECORD_OFF:
				case CMD_POWEROFF:
					ESP_LOGE(TAG, "Unsupported command in json string: %s", json_cmd_string);
					break;
				
				default:
					ESP_LOGE(TAG, "Unknown command in json string: %s", json_cmd_string);
			}
		} else {
			ESP_LOGE(TAG, "Unknown type of json string: %s", json_cmd_string);
		}
		
		json_free_cmd(json_obj);
	} else {
		ESP_LOGE(TAG, "Couldn't convert json string: %s", json_cmd_string);
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
static void process_set_config(cJSON* cmd_args)
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
	}
}


static void process_set_spotmeter(cJSON* cmd_args)
{
	uint16_t r1, c1, r2, c2;
	
	if (json_parse_set_spotmeter(cmd_args, &r1, &c1, &r2, &c2)) {
		lepton_spotmeter(r1, c1, r2, c2);
	}
}


static void process_stream_on(cJSON* cmd_args)
{
	uint32_t delay_ms, num_frames;
	
	if (json_parse_stream_on(cmd_args, &delay_ms, &num_frames)) {
		rsp_set_stream_parameters(delay_ms, num_frames);
		xTaskNotify(task_handle_rsp, RSP_NOTIFY_CMD_STREAM_ON_MASK, eSetBits);
	}
}


static void process_set_time(cJSON* cmd_args)
{
	tmElements_t te;
	
	if (json_parse_set_time(cmd_args, &te)) {
		time_set(te);
	}
}


static void process_set_wifi(cJSON* cmd_args)
{
	char ap_ssid[PS_SSID_MAX_LEN+1];
	char sta_ssid[PS_SSID_MAX_LEN+1];
	char ap_pw[PS_PW_MAX_LEN+1];
	char sta_pw[PS_PW_MAX_LEN+1];
	wifi_info_t new_wifi_info;
	
	new_wifi_info.ap_ssid = ap_ssid;
	new_wifi_info.sta_ssid = sta_ssid;
	new_wifi_info.ap_pw = ap_pw;
	new_wifi_info.sta_pw = sta_pw;
	
	if (json_parse_set_wifi(cmd_args, &new_wifi_info)) {
		ps_set_wifi_info(&new_wifi_info);
		if (!wifi_reinit()) {
			ESP_LOGE(TAG, "Could not restart WiFi with the new configuration");
		}
	}
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
			if (i++ >= CMD_MAX_TCP_RX_BUFFER_LEN) i = 0;
		}
	}
	
	return -1;
}
