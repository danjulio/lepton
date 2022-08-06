/*
 * Network Command Task
 *
 * Implement the command processing module for use when either the
 * Ethernet or WiFi interfaces are active.  Enable mDNS for device
 * discovery.
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
#include "ctrl_task.h"
#include "cmd_utilities.h"
#include "net_utilities.h"
#include "system_config.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "mdns.h"



//
// Network CMD Task variables
//
static const char* TAG = "net_cmd_task";

// Client socket
static int client_sock = -1;

// Connected status
static bool connected = false;

// mDNS TXT records
#define NUM_SERVICE_TXT_ITEMS 3
static mdns_txt_item_t service_txt_data[NUM_SERVICE_TXT_ITEMS];
static char* txt_item_keys[NUM_SERVICE_TXT_ITEMS] = {
	"model",
	"interface",
	"version"
};



//
// Network CMD Forward Declarations for internal functions
//
static void net_cmd_start_mdns();



//
// Network CMD Task API
//
void net_cmd_task()
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
	
	// Wait until the network interface is connected
	if (!(*net_is_connected)()) {
		vTaskDelay(pdMS_TO_TICKS(500));
	}
	
	// Attempt to start the MDNS discovery service (we continue even if it fails)
	net_cmd_start_mdns();
	
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
            		if ((*net_is_connected)()) {
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
bool net_cmd_connected()
{
	return connected;
}


/**
 * Return socket descriptor
 */
int net_cmd_get_socket()
{
	return client_sock;
}



//
// Network CMD Internal functions
//
static void net_cmd_start_mdns()
{
	char model_type[2];     // Camera Model number "N"
	char txt_if_type[9];    // "WiFi" or "Ethernet"
	const esp_app_desc_t* app_desc;
	int brd_type;
	int if_type;
	esp_err_t ret;
	net_info_t* net_infoP;
	
	// Attempt to initialize mDNS
	ret = mdns_init();
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Could not initialize mDNS (%d)", ret);
		return;
	}
	
	// Set our hostname
	net_infoP = net_get_info();
	ret = mdns_hostname_set(net_infoP->ap_ssid);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Could not set mDNS hostname %s (%d)", net_infoP->ap_ssid, ret);
		return;
	}
	
	// Get dynamic info for TXT records
	app_desc = esp_ota_get_app_description();  // Get version info
	ctrl_get_if_mode(&brd_type, &if_type);
	model_type[0] = '0' + ((brd_type == CTRL_BRD_ETH_TYPE) ? CAMERA_MODEL_NUM_ETH : CAMERA_MODEL_NUM_WIFI);
	model_type[1] = 0;
	if (if_type == CTRL_IF_MODE_ETH) {
		strcpy(txt_if_type, "Ethernet");
	} else {
		strcpy(txt_if_type, "WiFi");
	}
	service_txt_data[0].key = txt_item_keys[0];
	service_txt_data[0].value = model_type;
	service_txt_data[1].key = txt_item_keys[1];
	service_txt_data[1].value = txt_if_type;
	service_txt_data[2].key = txt_item_keys[2];
	service_txt_data[2].value = app_desc->version;
	
	// Initialize service
	ret = mdns_service_add(NULL, "_tcam-socket", "_tcp", CMD_PORT, service_txt_data, NUM_SERVICE_TXT_ITEMS);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Could not initialize mDNS service (%d)", ret);
		return;
	}
	
	ESP_LOGI(TAG, "mDNS started");
}
