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
 * along with tCam.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include "wifi_cmd_task.h"
#include "ctrl_task.h"
#include "cmd_utilities.h"
#include "wifi_utilities.h"
#include "system_config.h"
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>



//
// WiFi CMD Task variables
//
static const char* TAG = "wifi_cmd_task";

// Client socket
static int client_sock = -1;

// Connected status
static bool connected = false;



//
// WiFi CMD Task API
//
void wifi_cmd_task()
{
	char rx_buffer[256];
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
        	len = recv(client_sock, rx_buffer, sizeof(rx_buffer), MSG_DONTWAIT);
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
bool wifi_cmd_connected()
{
	return connected;
}


/**
 * Return socket descriptor
 */
int wifi_cmd_get_socket()
{
	return client_sock;
}
