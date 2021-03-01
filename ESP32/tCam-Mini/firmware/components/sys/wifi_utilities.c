/*
 * WiFi related utilities
 *
 * Contains functions to initialize the wifi interface, utility functions, and a set
 * of interface functions.  Also includes the system event handler for use by the wifi
 * system.
 *
 * Note: Currently only 1 station is allowed to connect at a time.
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
#include "wifi_utilities.h"
#include "ps_utilities.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_event_loop.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include <lwip/sockets.h>
#include <string.h>



//
// Wifi Utilities local variables
//
static const char* TAG = "wifi_utilities";

// Wifi information
static char wifi_ap_ssid_array[PS_SSID_MAX_LEN+1];
static char wifi_sta_ssid_array[PS_SSID_MAX_LEN+1];
static char wifi_ap_pw_array[PS_PW_MAX_LEN+1];
static char wifi_sta_pw_array[PS_PW_MAX_LEN+1];
static wifi_info_t wifi_info = {
	wifi_ap_ssid_array,
	wifi_sta_ssid_array,
	wifi_ap_pw_array,
	wifi_sta_pw_array,
	0,
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

static bool sta_connected = false; // Set when we connect to an AP so we can disconnect if we restart
static int sta_retry_num = 0;

static wifi_ap_record_t ap_info[WIFI_MAX_SCAN_LIST_SIZE];
static bool scan_in_progress = false;
static bool got_scan_done_event = false; // Set when an AP Scan is complete

// FreeRTOS event group to signal when we are connected
static EventGroupHandle_t wifi_event_group;



//
// WiFi Utilities Forward Declarations for internal functions
//
static bool init_esp_wifi();
static bool enable_esp_wifi_ap();
static bool enable_esp_wifi_client();
static esp_err_t sys_event_handler(void *ctx, system_event_t* event);


//
// WiFi Utilities API
//

/**
 * Power-on initialization of the WiFi system.  It is enabled based on start-up
 * information from persistent storage.  Returns false if any part of the initialization
 * fails.
 */
bool wifi_init()
{
	esp_err_t ret;
	
	// Setup the event handler
	wifi_event_group = xEventGroupCreate();
	ret = esp_event_loop_init(sys_event_handler, NULL);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Could not initialize event loop handler (%d)", ret);
		return false;
	}
	
	// Initialize the TCP/IP stack
	tcpip_adapter_init();
	
	// Initialize NVS
	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ret = nvs_flash_erase();
		if (ret != ESP_OK) {
			ESP_LOGI(TAG, "nvs_flash_erase failed (%d)", ret);
			return false;
		}
		ret = nvs_flash_init();
	}
	if (ret != ESP_OK) {
		ESP_LOGI(TAG, "nvs_flash_init failed (%d)", ret);
		return false;
	}
	
	// Get our wifi info
	ps_get_wifi_info(&wifi_info);
	
	// Initialize the WiFi interface
	if (init_esp_wifi()) {
		wifi_info.flags |= WIFI_INFO_FLAG_INITIALIZED;
		
		// Configure the WiFi interface if enabled
		if ((wifi_info.flags & WIFI_INFO_FLAG_STARTUP_ENABLE) != 0) {
			if ((wifi_info.flags & WIFI_INFO_FLAG_CLIENT_MODE) != 0) {
				if (enable_esp_wifi_client()) {
					wifi_info.flags |= WIFI_INFO_FLAG_ENABLED;
					ESP_LOGI(TAG, "WiFi Station starting");
				} else {
					return false;
				}
			} else {
				if (enable_esp_wifi_ap()) {
					wifi_info.flags |= WIFI_INFO_FLAG_ENABLED;
					ESP_LOGI(TAG, "WiFi AP %s enabled", wifi_info.ap_ssid);
				} else {
					return false;
				}
			}
		}
	} else {
		return false;
	}
	
	return true;
}


/**
 * Re-initialize the WiFi system when information such as the SSID, password or enabe-
 * state have changed.  Returns false if anything fails.
 */
bool wifi_reinit()
{
	// Attempt to disconnect from an AP if we were previously connected
	if (sta_connected) {
		ESP_LOGI(TAG, "Attempting to disconnect from AP");
		esp_wifi_disconnect();
		sta_connected = false;
	}
	
	// Shut down the old configuration
	if ((wifi_info.flags & WIFI_INFO_FLAG_ENABLED) != 0) {
		ESP_LOGI(TAG, "WiFi stopping");
		esp_wifi_stop();
		wifi_info.flags &= ~WIFI_INFO_FLAG_ENABLED;
	}

	if ((wifi_info.flags & WIFI_INFO_FLAG_INITIALIZED) == 0) {
		// Attempt to initialize the wifi interface again
		if (!init_esp_wifi()) {
			return false;
		}
	}
	
	// Update the wifi info because we're called when it's updated
	ps_get_wifi_info(&wifi_info);
	wifi_info.flags |= WIFI_INFO_FLAG_INITIALIZED;   // Add in the fact we're already initialized
	
	// Reconfigure the interface if enabled
	if ((wifi_info.flags & WIFI_INFO_FLAG_STARTUP_ENABLE) != 0) {
		if ((wifi_info.flags & WIFI_INFO_FLAG_CLIENT_MODE) != 0) {
			if (enable_esp_wifi_client()) {
				wifi_info.flags |= WIFI_INFO_FLAG_ENABLED;
				ESP_LOGI(TAG, "WiFi Station starting");
			} else {
				return false;
			}
		} else {
			if (enable_esp_wifi_ap()) {
				wifi_info.flags |= WIFI_INFO_FLAG_ENABLED;
				ESP_LOGI(TAG, "WiFi AP %s enabled", wifi_info.ap_ssid);
			} else {
				return false;
			}
		}
	}
	
	// Nothing should be connected now
	wifi_info.flags &= ~WIFI_INFO_FLAG_CONNECTED;
	
	return true;
}


/**
 * Setup for and start an AP scan
 *   - Disables WiFi and disconnects from an AP if currently connected
 */
bool wifi_setup_scan()
{
	esp_err_t ret;
	wifi_config_t wifi_config = {
		.sta = {
			.ssid[0] = 0,
			.password[0] = 0,
			.scan_method = WIFI_ALL_CHANNEL_SCAN,
			.bssid_set = 0,
			.channel = 0,
			.listen_interval = 0,
			.sort_method = WIFI_CONNECT_AP_BY_SIGNAL			
		}
	};
	
	got_scan_done_event = false;
	
	// Attempt to disconnect from an AP if we were previously connected
	if (sta_connected) {
		ESP_LOGI(TAG, "Attempting to disconnect from AP");
		esp_wifi_disconnect();
		sta_connected = false;
	}
	
	// Shut down the old configuration
	if ((wifi_info.flags & WIFI_INFO_FLAG_ENABLED) != 0) {
		ESP_LOGI(TAG, "WiFi stopping");
		esp_wifi_stop();
		wifi_info.flags &= ~WIFI_INFO_FLAG_ENABLED;
	}

	if ((wifi_info.flags & WIFI_INFO_FLAG_INITIALIZED) == 0) {
		// Attempt to initialize the wifi interface again
		if (!init_esp_wifi()) {
			return false;
		}
	}
	
	// Configure into STA mode and start a scan
    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) {
    	ESP_LOGE(TAG, "Could not set Station mode for scan (%d)", ret);
    	return false;
    }
    
    ret = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    if (ret != ESP_OK) {
    	ESP_LOGE(TAG, "Could not set Station configuration for scan (%d)", ret);
    	return false;
    }
    
    ret = esp_wifi_start();
    if (ret != ESP_OK) {
    	ESP_LOGE(TAG, "Could not start Station for scan (%d)", ret);
    	return false;
    }
    
    scan_in_progress = true;
    ret = esp_wifi_scan_start(NULL, false);
    if (ret != ESP_OK) {
    	ESP_LOGE(TAG, "Could not start scan (%d)", ret);
    	scan_in_progress = false;
    	return false;
    }
    wifi_info.flags |= WIFI_INFO_FLAG_ENABLED;
    
    ESP_LOGI(TAG, "Start scan");
    
    return true;
}


/**
 * Stop a scan if it is in progress
 */
void wifi_stop_scan()
{
	// Since this should only be called if we've started a scan, don't sweat the return
	// code
	(void) esp_wifi_scan_stop();
	scan_in_progress = false;
	got_scan_done_event = false;
}


/**
 * Return connected to client status
 */
bool wifi_is_connected()
{
	return ((wifi_info.flags & WIFI_INFO_FLAG_CONNECTED) != 0);
}


/**
 * Return scan completion status
 */
bool wifi_scan_is_complete()
{
	return got_scan_done_event;
}


/**
 * Return current WiFi configuration and state
 */
wifi_info_t* wifi_get_info()
{
	return &wifi_info;
}


/**
 * Return a pointer to an array of wifi_ap_record_t structures
 */
int wifi_get_scan_records(wifi_ap_record_t **ap)
{
	esp_err_t ret;
	uint16_t number = WIFI_MAX_SCAN_LIST_SIZE;
	uint16_t ap_count = 0;
	
	*ap = &ap_info[0];
	
	if (got_scan_done_event) {
    	memset(ap_info, 0, sizeof(ap_info));
    	ret = esp_wifi_scan_get_ap_records(&number, ap_info);
    	if (ret != ESP_OK) {
			ESP_LOGE(TAG, "Could not get ap_info (%d)", ret);
			return 0;
		}
		
    	ret = esp_wifi_scan_get_ap_num(&ap_count);
    	if (ret != ESP_OK) {
			ESP_LOGE(TAG, "Could not get ap_num (%d)", ret);
			return 0;
		}
		
		ESP_LOGI(TAG, "Total APs found = %u", ap_count);
		return ap_count;
	} else {
		return 0;
	}
}


//
// WiFi Utilities internal functions
//

/**
 * Initialize the WiFi interface resources
 */
static bool init_esp_wifi()
{
	esp_err_t ret;
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	
	ret = esp_wifi_init(&cfg);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Could not allocate wifi resources (%d)", ret);
		return false;
	}
	
	// We don't need the NVS configuration storage for the WiFi configuration since we
	// are managing persistent storage ourselves
	ret = esp_wifi_set_storage(WIFI_STORAGE_RAM);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Could not set RAM storage for configuration (%d)", ret);
		return false;
	}
	
	return true;
}


/**
 * Enable this device as a Soft AP
 */
static bool enable_esp_wifi_ap()
{
	esp_err_t ret;
	int i;
	
	// Enable the AP
	wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen(wifi_info.ap_ssid),
            .max_connection = 1,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        }
    };
    strcpy((char*) wifi_config.ap.ssid, wifi_info.ap_ssid);
    strcpy((char*) wifi_config.ap.password, wifi_info.ap_pw);
    if (strlen(wifi_info.ap_pw) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    
    ret = esp_wifi_set_mode(WIFI_MODE_AP);
    if (ret != ESP_OK) {
    	ESP_LOGE(TAG, "Could not set Soft AP mode (%d)", ret);
    	return false;
    }
    
    ret = esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
    if (ret != ESP_OK) {
    	ESP_LOGE(TAG, "Could not set Soft AP configuration (%d)", ret);
    	return false;
    }
    
    ret = esp_wifi_start();
    if (ret != ESP_OK) {
    	ESP_LOGE(TAG, "Could not start Soft AP (%d)", ret);
    	return false;
    }
    
    // For now, since we are using the default IP address, copy it to the current here
    for (i=0; i<4; i++) {
    	wifi_info.cur_ip_addr[i] = wifi_info.ap_ip_addr[i];
    }
    	
    return true;
}


/**
 * Enable this device as a Client
 */
static bool enable_esp_wifi_client()
{
	esp_err_t ret;
	tcpip_adapter_ip_info_t ipInfo;
	
	// Configure the IP address mechanism
	if ((wifi_info.flags & WIFI_INFO_FLAG_CL_STATIC_IP) != 0) {
		// Static IP
		ret = tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
		if (ret != ESP_OK) {
    		ESP_LOGW(TAG, "Stop Station DHCP returned %d", ret);
    	}
    	
		ipInfo.ip.addr = wifi_info.sta_ip_addr[3] |
						 (wifi_info.sta_ip_addr[2] << 8) |
						 (wifi_info.sta_ip_addr[1] << 16) |
						 (wifi_info.sta_ip_addr[0] << 24);
  		inet_pton(AF_INET, "0.0.0.0", &ipInfo.gw);
  		ipInfo.netmask.addr = wifi_info.sta_netmask[3] |
						     (wifi_info.sta_netmask[2] << 8) |
						     (wifi_info.sta_netmask[1] << 16) |
						     (wifi_info.sta_netmask[0] << 24);
  		tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
	} else {
		ret = tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_STA);
		if (ret != ESP_OK) {
    		ESP_LOGE(TAG, "Start Station DHCP returned %d", ret);
    	}
	}
	
	// Enable the Client
	wifi_config_t wifi_config = {
		.sta = {
			.scan_method = WIFI_FAST_SCAN,
			.bssid_set = 0,
			.channel = 0,
			.listen_interval = 0,
			.sort_method = WIFI_CONNECT_AP_BY_SIGNAL			
		}
	};	
    strcpy((char*) wifi_config.sta.ssid, wifi_info.sta_ssid);
    if (strlen(wifi_info.sta_pw) == 0) {
        strcpy((char*) wifi_config.sta.password, "");
    } else {
    	strcpy((char*) wifi_config.sta.password, wifi_info.sta_pw);
    }
    
    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) {
    	ESP_LOGE(TAG, "Could not set Station mode (%d)", ret);
    	return false;
    }
    
    ret = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    if (ret != ESP_OK) {
    	ESP_LOGE(TAG, "Could not set Station configuration (%d)", ret);
    	return false;
    }
    
    ret = esp_wifi_start();
    if (ret != ESP_OK) {
    	ESP_LOGE(TAG, "Could not start Station (%d)", ret);
    	return false;
    }
    
    return true;
}


/**
 * Handle system events that we care about from the WiFi task
 */
static esp_err_t sys_event_handler(void *ctx, system_event_t* event)
{
	switch(event->event_id) {
		case SYSTEM_EVENT_AP_STACONNECTED:
			wifi_info.flags |= WIFI_INFO_FLAG_CONNECTED;
			ESP_LOGI(TAG, "station:"MACSTR" join, AID=%d",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
			break;
		
		case SYSTEM_EVENT_AP_STADISCONNECTED:
			wifi_info.flags &= ~WIFI_INFO_FLAG_CONNECTED;
			ESP_LOGI(TAG, "station:"MACSTR" leave, AID=%d",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);
			break;
			
		case SYSTEM_EVENT_STA_START:
			if (scan_in_progress) {
				ESP_LOGI(TAG, "Station started for scan");
			} else {
				ESP_LOGI(TAG, "Station started, trying to connect to %s", wifi_info.sta_ssid);
				esp_wifi_connect();
			}
			sta_retry_num = 0;
        	break;
        
        case SYSTEM_EVENT_STA_STOP:
        	ESP_LOGI(TAG, "Station stopped");
        	break;
        	
        case SYSTEM_EVENT_STA_GOT_IP:
        	wifi_info.flags |= WIFI_INFO_FLAG_CONNECTED;
        	uint32_t ip = event->event_info.got_ip.ip_info.ip.addr;
        	ESP_LOGI(TAG, "Connected. Got ip: %s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        	wifi_info.cur_ip_addr[3] = ip & 0xFF;
        	wifi_info.cur_ip_addr[2] = (ip >> 8) & 0xFF;
        	wifi_info.cur_ip_addr[1] = (ip >> 16) & 0xFF;
			wifi_info.cur_ip_addr[0] = (ip >> 24) & 0xFF;
			sta_connected = true;
        	sta_retry_num = 0;
        	break;
        	
        case SYSTEM_EVENT_STA_DISCONNECTED:
        	wifi_info.flags &= ~WIFI_INFO_FLAG_CONNECTED;
        	if (!scan_in_progress) {
        		if (sta_retry_num > WIFI_FAST_RECONNECT_ATTEMPTS) {
        			vTaskDelay(pdMS_TO_TICKS(1000));
        		} else {
        			sta_retry_num++;
        		}
                esp_wifi_connect();
                ESP_LOGI(TAG, "Retry connection to %s", wifi_info.sta_ssid);
            }
        	break;
        
        case SYSTEM_EVENT_SCAN_DONE:
        	ESP_LOGI(TAG, "Scan done");
        	scan_in_progress = false;
        	got_scan_done_event = true;
        	break;
        
		default:
			break;
	}
	
	return ESP_OK;
}
