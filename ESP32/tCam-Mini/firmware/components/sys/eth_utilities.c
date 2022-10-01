/*
 * Ethernet related utilities
 *
 * Contains functions to initialize and query the ethernet interface.
 *
 * Copyright 2022 Dan Julio
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
#include "eth_utilities.h"
#include "ps_utilities.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include <string.h>
#include "system_config.h"



//
// Ethernet Utilities local variables
//
static const char* TAG = "eth_utilities";

// Ethernet netif instance
static esp_netif_t *eth_netif;

// Network information - we don't use the WiFi-specific parts
static char ap_ssid_array[PS_SSID_MAX_LEN+1];
static char sta_ssid_array[PS_SSID_MAX_LEN+1];
static char ap_pw_array[PS_PW_MAX_LEN+1];
static char sta_pw_array[PS_PW_MAX_LEN+1];
static net_info_t eth_info = {
	ap_ssid_array,
	sta_ssid_array,
	ap_pw_array,
	sta_pw_array,
	0,
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

// Interface
static eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
static eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
static esp_eth_config_t eth_config;

static esp_eth_mac_t* mac;
static esp_eth_phy_t* phy;
static esp_eth_handle_t eth_handle = NULL;



//
// Ethernet Utilities Forward Declarations for internal functions
//
static void eth_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);



//
// Ethernet Utilities API
//
bool eth_init()
{
	esp_err_t ret;
	esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
	
	// Initialize the TCP/IP stack
	ret = esp_netif_init();
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Could not init netif (%d)", ret);
		return false;
	}
	
	// Create the instance of esp_netif for Ethernet
    eth_netif = esp_netif_new(&netif_cfg);
	
	// Setup the default event handlers
	ret = esp_event_loop_create_default();
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Could not create default event loop handler (%d)", ret);
		return false;
	}
	
	ret = esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Could not register eth_event_handler (%d)", ret);
		return false;
	}
	
	ret = esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &ip_event_handler, NULL);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Could not register ip_event_handler (%d)", ret);
		return false;
	}
	
	// Setup the Ethernet MAC
	mac_config.smi_mdc_gpio_num = BRD_E_ETH_MDC;
	mac_config.smi_mdio_gpio_num = BRD_E_ETH_MDIO;
	mac = esp_eth_mac_new_esp32(&mac_config);
	
	// Setup the Ethernet PHY
	phy_config.phy_addr = ETH_PHY_ADDRESS;
	phy_config.reset_gpio_num = BRD_E_PHY_RESET;
#ifdef ETH_PHY_IP101
	phy = esp_eth_phy_new_ip101(&phy_config);
#endif
#ifdef ETH_PHY_RTL8201
	phy = esp_eth_phy_new_rtl8201(&phy_config);
#endif
#ifdef ETH_PHY_LAN8720
	phy = esp_eth_phy_new_lan8720(&phy_config);
#endif
#ifdef ETH_PHY_DP83848
	phy = esp_eth_phy_new_dp83848(&phy_config);
#endif
	
	// Setup the driver configuration
	eth_config.mac = mac;
	eth_config.phy = phy;
	eth_config.check_link_period_ms = 2000;
	eth_config.stack_input = NULL;
	eth_config.on_lowlevel_init_done = NULL;
	eth_config.on_lowlevel_deinit_done = NULL;
	
	// Install the driver
	ret = esp_eth_driver_install(&eth_config, &eth_handle);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Could not install ethernet driver (%d)", ret);
		return false;
	}
	
	// Attach Ethernet driver to TCP/IP stack
	ret = esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle));
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Could not attach ethernet driver (%d)", ret);
		return false;
	}
	
	// Finally attempt to initialize the ethernet interface
	return eth_reinit();
}


bool eth_reinit()
{
	esp_err_t ret;
	esp_netif_ip_info_t ipInfo;
	
	// Shut down the old configuration if one exists
	if ((eth_info.flags & NET_INFO_FLAG_ENABLED) != 0) {
		ESP_LOGI(TAG, "Ethernet Stopping");
		esp_eth_stop(eth_handle);
		eth_info.flags &= ~NET_INFO_FLAG_ENABLED;
		eth_info.flags &= ~NET_INFO_FLAG_CONNECTED;
	}

	// Update the ethernet info (because we're called when it's updated)
	ps_get_net_info(&eth_info);
	eth_info.flags |= NET_INFO_FLAG_INITIALIZED;   // Add in the fact we're already initialized
	
	// Reconfigure the interface if enabled
	if ((eth_info.flags & NET_INFO_FLAG_STARTUP_ENABLE) != 0) {
		// Configure the IP address mechanism
		if ((eth_info.flags & NET_INFO_FLAG_CL_STATIC_IP) != 0) {
			// Static IP
			ret = esp_netif_dhcpc_stop(eth_netif);
			if ((ret != ESP_OK) && (ret != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED)) {
	    		ESP_LOGE(TAG, "Stop DHCP returned %d", ret);
	    		return false;
	    	}
	    	
			ipInfo.ip.addr = eth_info.sta_ip_addr[3] |
							 (eth_info.sta_ip_addr[2] << 8) |
							 (eth_info.sta_ip_addr[1] << 16) |
							 (eth_info.sta_ip_addr[0] << 24);
	  		ipInfo.gw.addr = esp_netif_ip4_makeu32(0, 0, 0, 0);
	  		ipInfo.netmask.addr = eth_info.sta_netmask[3] |
							     (eth_info.sta_netmask[2] << 8) |
							     (eth_info.sta_netmask[1] << 16) |
							     (eth_info.sta_netmask[0] << 24);
	  		ret = esp_netif_set_ip_info(eth_netif, &ipInfo);
			if (ret != ESP_OK) {
				ESP_LOGE(TAG, "Set IP info returned %d", ret);
			}
		} else {
			ret = esp_netif_dhcpc_start(eth_netif);
			if (ret != ESP_OK) {
	    		ESP_LOGE(TAG, "Start DHCP returned %d", ret);
	    		return false;
	    	}
		}
	}
	
	// Start the interface
	ret = esp_eth_start(eth_handle);
	if (ret == ESP_OK) {
		eth_info.flags |= NET_INFO_FLAG_ENABLED;
	} else {
		ESP_LOGE(TAG, "Could not start ethernet (%d)", ret);
		return false;
	}
	
	return true;
}


bool eth_is_connected()
{
	return ((eth_info.flags & NET_INFO_FLAG_CONNECTED) != 0);
}


net_info_t* eth_get_info()
{
	return &eth_info;
}



//
// Ethernet Utilities Internal functions
//
static void eth_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	switch (event_id) {
		case ETHERNET_EVENT_CONNECTED:
			eth_info.flags |= NET_INFO_FLAG_CONNECTED;
			ESP_LOGI(TAG, "Ethernet Link Up");
			break;
		
		case ETHERNET_EVENT_DISCONNECTED:
			eth_info.flags &= ~NET_INFO_FLAG_CONNECTED;
			ESP_LOGI(TAG, "Ethernet Link Down");
			break;
		
		case ETHERNET_EVENT_START:
			ESP_LOGI(TAG, "Ethernet Started");
			break;
		
		case ETHERNET_EVENT_STOP:
			ESP_LOGI(TAG, "Ethernet Stopped");
			break;
	}
}


static void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
	const esp_netif_ip_info_t *ip_info = &event->ip_info;
	
	ESP_LOGI(TAG, "Got IP Address:" IPSTR, IP2STR(&ip_info->ip));
    
    eth_info.cur_ip_addr[3] = ip_info->ip.addr & 0xFF;
    eth_info.cur_ip_addr[2] = (ip_info->ip.addr >> 8) & 0xFF;
    eth_info.cur_ip_addr[1] = (ip_info->ip.addr >> 16) & 0xFF;
	eth_info.cur_ip_addr[0] = (ip_info->ip.addr >> 24) & 0xFF;
}
