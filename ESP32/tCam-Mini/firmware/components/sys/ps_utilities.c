/*
 * Persistent Storage Module
 *
 * Manage the persistent storage kept in the ESP32 NVS and provide access
 * routines to it.
 *
 * NOTE:
 *  1. It is assumed that only one task will access persistent storage at a time.
 *  2. Some internal naming is reflective of the fact that this module existed first
 *     for a Wifi-only system with ethernet support added later.
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
#include "ps_utilities.h"
#include "ctrl_task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "system_config.h"
#include <stdbool.h>
#include <string.h>


//
// PS Utilities internal constants
//

// Uncomment the following directive to force an NVS memory erase (but only do it for one execution)
//#define PS_ERASE_NVS

// Lepton state boolean flags
#define PS_LEP_AGC_EN_MASK     0x01

// Stored Network Flags bitmask
#define PS_NET_FLAG_MASK      (NET_INFO_FLAG_STARTUP_ENABLE | NET_INFO_FLAG_CL_STATIC_IP | NET_INFO_FLAG_CLIENT_MODE)

// NVS namespace
#define STORAGE_NAMESPACE "storage"




//
// PS Utilities enums and structs
//

//
// Persistent Storage data structures
// Note: Careful attention must be paid to make sure these are packed structures
//

// Stored Lepton parameters
typedef struct {
	uint8_t flags;
	uint8_t emissivity;
	uint8_t gain_mode;
} ps_lep_state_t;


// Stored Network Parameters (strings are null-terminated)
//   For ethernet the fields have the following meaning:
//     ap_ssid - Camera name
//     ap_ip_addr - Device address if it is serving DHCP addresses (stand-alone network)
//     stap_ip_addr - Device address when it has a fixed IP address on a network
//                    (sta_netmask applies as well)
typedef struct {
	char ap_ssid[PS_SSID_MAX_LEN+1];
	char sta_ssid[PS_SSID_MAX_LEN+1];
	char ap_pw[PS_PW_MAX_LEN+1];
	char sta_pw[PS_PW_MAX_LEN+1];
	uint8_t flags;
	uint8_t ap_ip_addr[4];
	uint8_t sta_ip_addr[4];
	uint8_t sta_netmask[4];
} ps_net_info_t;

// Previous version Stored Network Parameters (32 character max password)
// Used to automatically update the NVS after an OTA firmware update
typedef struct {
	char ap_ssid[PS_SSID_MAX_LEN+1];
	char sta_ssid[PS_SSID_MAX_LEN+1];
	char ap_pw[PS_OLD_PW_MAX_LEN+1];
	char sta_pw[PS_OLD_PW_MAX_LEN+1];
	uint8_t flags;
	uint8_t ap_ip_addr[4];
	uint8_t sta_ip_addr[4];
	uint8_t sta_netmask[4];
} ps_old_net_info_t;



//
// PS Utilities Internal variables
//
static const char* TAG = "ps_utilities";

// NVS Keys
static const char* lep_info_key = "lep_state";
static const char* wifi_info_key = "wifi_info";
static const char* eth_info_key = "eth_info";

// Local copies
static ps_lep_state_t ps_lep_state;
static ps_net_info_t ps_wifi_info;
static ps_net_info_t ps_eth_info;


// NVS namespace handle
static nvs_handle_t ps_handle;

// Board type (Ethernet, WiFi)
static int brd_type;

// Interface type (Ethernet, Serial, WiFi)
static int if_type;



//
// PS Utilities Forward Declarations for internal functions
//
static void ps_default_lep_info();
static void ps_default_net_info(int iface);
static bool ps_write_lep_info();
static bool ps_read_lep_info();
static bool ps_write_net_info(int iface);
static bool ps_read_net_info(int iface);
static bool ps_read_old_net_info(int iface);
static void ps_store_string(char* dst, char* src, uint8_t max_len);



//
// PS Utilities API
//

/**
 * Initialize persistent storage
 *   - Load our local buffer
 *   - Initialize it and the NVS with valid data if necessary
 */
bool ps_init(int brd, int iface)
{
	bool success = true;
	esp_err_t err;
	size_t required_size;
	
	brd_type = brd;
	if_type = iface;
	
	// Initialize the NVS Storage system
	err = nvs_flash_init();
#ifdef PS_ERASE_NVS
	ESP_LOGE(TAG, "NVS Erase");
	err = nvs_flash_erase();	
	err = nvs_flash_init();
#endif
	
	if ((err == ESP_ERR_NVS_NO_FREE_PAGES) || (err == ESP_ERR_NVS_NEW_VERSION_FOUND)) {
		// NVS partition was truncated and needs to be erased
		err = nvs_flash_erase();
		if (err != ESP_OK) {
			ESP_LOGE(TAG, "NVS Erase failed with err %d", err);
			return false;
		}
		
		// Retry init
		err = nvs_flash_init();
		if (err != ESP_OK) {
			ESP_LOGE(TAG, "NVS Init failed with err %d", err);
			return false;
		}
	}
	
	// Open NVS Storage
	err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &ps_handle);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "NVS Open failed with err %d", err);
		return false;
	}
	
	//
	// Initialize our local copies
	//   - Attempt to find the entry in NVS storage and initialize our local copy from that
	//   - Initialize our local copy with default values and use those to initialize NVS
	//     storage if it does not exist or is invalid.
	//
	
	// Lepton Info
	err = nvs_get_blob(ps_handle, lep_info_key, NULL, &required_size);
	if ((err != ESP_OK) && (err != ESP_ERR_NVS_NOT_FOUND)) {
		ESP_LOGE(TAG, "NVS get_blob lep size failed with err %d", err);
		return false;
	}
	if ((required_size == 0) || (required_size != sizeof(ps_lep_state_t))) {
		if (required_size == 0) {
			ESP_LOGI(TAG, "Initializing NVS lep info");
		} else {
			ESP_LOGI(TAG, "Re-initializing NVS lep info");
		}
		ps_default_lep_info();
		success &= ps_write_lep_info();
	} else {
		ESP_LOGI(TAG, "Reading NVS lep info");
		success &= ps_read_lep_info();
	}
	
	// WiFi Info (common to both board types)
	err = nvs_get_blob(ps_handle, wifi_info_key, NULL, &required_size);
	if ((err != ESP_OK) && (err != ESP_ERR_NVS_NOT_FOUND)) {
		ESP_LOGE(TAG, "NVS get_blob WiFi size failed with err %d", err);
		return false;
	}
	if (required_size == sizeof(ps_old_net_info_t)) {
		ESP_LOGI(TAG, "Updating NVS WiFi info");
		success &= ps_read_old_net_info(CTRL_IF_MODE_WIFI);
		if (success) {
			err = nvs_erase_key(ps_handle, wifi_info_key);
			if (err != ESP_OK) {
				ESP_LOGE(TAG, "NVS WiFi info erase failed with err %d", err);
				// We'll try to write the new blob anyway...
			}
			success &= ps_write_net_info(CTRL_IF_MODE_WIFI);
		}
	} else if ((required_size == 0) || (required_size != sizeof(ps_net_info_t))) {
		if (required_size == 0) {
			ESP_LOGI(TAG, "Initializing NVS WiFi info");
		} else {
			ESP_LOGI(TAG, "Re-initializing NVS WiFi info");
		}
		ps_default_net_info(CTRL_IF_MODE_WIFI);
		success &= ps_write_net_info(CTRL_IF_MODE_WIFI);
	} else {
		ESP_LOGI(TAG, "Reading NVS WiFi info");
		success &= ps_read_net_info(CTRL_IF_MODE_WIFI);
	}
	
	// Ethernet Info only loaded on the ethernet board type
	if (brd_type == CTRL_BRD_ETH_TYPE) {
		err = nvs_get_blob(ps_handle, eth_info_key, NULL, &required_size);
		if ((err != ESP_OK) && (err != ESP_ERR_NVS_NOT_FOUND)) {
			ESP_LOGE(TAG, "NVS get_blob ethernet size failed with err %d", err);
			return false;
		}
		if (required_size == sizeof(ps_old_net_info_t)) {
			ESP_LOGI(TAG, "Updating NVS ethernet info");
			success &= ps_read_old_net_info(CTRL_IF_MODE_ETH);
			if (success) {
				err = nvs_erase_key(ps_handle, eth_info_key);
				if (err != ESP_OK) {
					ESP_LOGE(TAG, "NVS ethernet info erase failed with err %d", err);
					// We'll try to write the new blob anyway...
				}
				success &= ps_write_net_info(CTRL_IF_MODE_ETH);
			}
		} else if ((required_size == 0) || (required_size != sizeof(ps_net_info_t))) {
			if (required_size == 0) {
				ESP_LOGI(TAG, "Initializing NVS ethernet info");
			} else {
				ESP_LOGI(TAG, "Re-initializing NVS ethernet info");
			}
			ps_default_net_info(CTRL_IF_MODE_ETH);
			success &= ps_write_net_info(CTRL_IF_MODE_ETH);
		} else {
			ESP_LOGI(TAG, "Reading NVS ethernet info");
			success &= ps_read_net_info(CTRL_IF_MODE_ETH);
		}
	}
		
	return success;
}


void ps_get_lep_state(json_config_t* state)
{
	// Get from our local copy
	state->agc_set_enabled = (ps_lep_state.flags & PS_LEP_AGC_EN_MASK) != 0;	
	state->emissivity = (int) ps_lep_state.emissivity;
	state->gain_mode = (int) ps_lep_state.gain_mode;
}


void ps_set_lep_state(const json_config_t* state)
{
	// We don't store changes when operating in serial mode
	if (if_type != CTRL_IF_MODE_SIF) {
		ps_lep_state.flags = (state->agc_set_enabled ? PS_LEP_AGC_EN_MASK : 0);	             
		ps_lep_state.emissivity = (uint8_t) state->emissivity;
		ps_lep_state.gain_mode = (uint8_t) state->gain_mode;
		
		if (!ps_write_lep_info()) {
			ESP_LOGE(TAG, "Failed to save lep data to NVS Storage");
		}
	}
}


void ps_get_net_info(net_info_t* info)
{
	int i;
	ps_net_info_t* local;
	
	local = (if_type == CTRL_IF_MODE_ETH) ? &ps_eth_info : &ps_wifi_info;
	
	// Get from our local copy
	strcpy(info->ap_ssid, (const char*) local->ap_ssid);
	strcpy(info->ap_pw, (const char*) local->ap_pw);
	strcpy(info->sta_ssid, (const char*) local->sta_ssid);
	strcpy(info->sta_pw, (const char*) local->sta_pw);
	
	info->flags = local->flags & PS_NET_FLAG_MASK;
	
	for (i=0; i<4; i++) {
		info->ap_ip_addr[i] = local->ap_ip_addr[i];
		info->sta_ip_addr[i] = local->sta_ip_addr[i];
		info->sta_netmask[i] = local->sta_netmask[i];
	}
}


void ps_set_net_info(const net_info_t* info)
{
	int i;
	
	// We don't store changes when operating in serial mode
	if (if_type == CTRL_IF_MODE_ETH) {
		ps_store_string(ps_eth_info.ap_ssid, info->ap_ssid, PS_SSID_MAX_LEN);
		
		ps_eth_info.flags = info->flags & PS_NET_FLAG_MASK;
		
		for (i=0; i<4; i++) {
			ps_eth_info.ap_ip_addr[i] = info->ap_ip_addr[i];
		 	ps_eth_info.sta_ip_addr[i] = info->sta_ip_addr[i];
		 	ps_eth_info.sta_netmask[i] = info->sta_netmask[i];
		}
		
		if (!ps_write_net_info(CTRL_IF_MODE_ETH)) {
			ESP_LOGE(TAG, "Failed to write ethernet data to NVS Storage");
		}
	} else if (if_type == CTRL_IF_MODE_WIFI) {
		ps_store_string(ps_wifi_info.ap_ssid, info->ap_ssid, PS_SSID_MAX_LEN);
		ps_store_string(ps_wifi_info.ap_pw, info->ap_pw, PS_PW_MAX_LEN);
		ps_store_string(ps_wifi_info.sta_ssid, info->sta_ssid, PS_SSID_MAX_LEN);
		ps_store_string(ps_wifi_info.sta_pw, info->sta_pw, PS_PW_MAX_LEN);
		
		ps_wifi_info.flags = info->flags & PS_NET_FLAG_MASK;
		
		for (i=0; i<4; i++) {
			ps_wifi_info.ap_ip_addr[i] = info->ap_ip_addr[i];
		 	ps_wifi_info.sta_ip_addr[i] = info->sta_ip_addr[i];
		 	ps_wifi_info.sta_netmask[i] = info->sta_netmask[i];
		}
		
		if (!ps_write_net_info(CTRL_IF_MODE_WIFI)) {
			ESP_LOGE(TAG, "Failed to write WiFi data to NVS Storage");
		}
	}
}


bool ps_reinit_net()
{
	bool success;
	
	if ((brd_type == CTRL_BRD_ETH_TYPE) && (if_type == CTRL_IF_MODE_ETH)) {
		ESP_LOGI(TAG, "Re-initializing NVS ethernet info");
		ps_default_net_info(CTRL_IF_MODE_ETH);
		success = ps_write_net_info(CTRL_IF_MODE_ETH);
	} else {
		ESP_LOGI(TAG, "Re-initializing NVS WiFi info");
		ps_default_net_info(CTRL_IF_MODE_WIFI);
		success = ps_write_net_info(CTRL_IF_MODE_WIFI);
	}
	
	return success;
}


bool ps_has_new_cam_name(const net_info_t* info)
{
	if (if_type == CTRL_IF_MODE_ETH) {
		return(strncmp(ps_eth_info.ap_ssid, info->ap_ssid, PS_SSID_MAX_LEN) != 0);
	} else if (if_type == CTRL_IF_MODE_WIFI) {
		return(strncmp(ps_wifi_info.ap_ssid, info->ap_ssid, PS_SSID_MAX_LEN) != 0);
	} else {
		// Shouldn't ever see this for serial mode but return something anyway
		return false;
	}
}


/**
 * Return an ASCII character version of a 4-bit hexadecimal number
 */
char ps_nibble_to_ascii(uint8_t n)
{
	n = n & 0x0F;
	
	if (n < 10) {
		return '0' + n;
	} else {
		return 'A' + n - 10;
	}
}



//
// PS Utilities internal functions
//

static void ps_default_lep_info()
{
	// LEP parameters
	ps_lep_state.flags = 0;
	ps_lep_state.emissivity = 100;
	ps_lep_state.gain_mode = SYS_GAIN_HIGH;
}


static void ps_default_net_info(int iface)
{
	int i;
	uint8_t sys_mac_addr[6];
	ps_net_info_t* local;
	
	local = (iface == CTRL_IF_MODE_ETH) ? &ps_eth_info : &ps_wifi_info;
	
	// Network parameters
	//
	// Get the system's default MAC address and 
	//   - For WiFi add 1 to match the "Soft AP" mode
	//   - For Ethernet add 3 to match the ethernet MAC
	// (see "Miscellaneous System APIs" in the ESP-IDF documentation)
	esp_efuse_mac_get_default(sys_mac_addr);
	if (iface == CTRL_IF_MODE_ETH) {
		sys_mac_addr[5] = sys_mac_addr[5] + 3;
	} else {
		sys_mac_addr[5] = sys_mac_addr[5] + 1;
	}
	
	// Construct our default AP SSID/Camera name
	sprintf(local->ap_ssid, "%s%c%c%c%c", PS_DEFAULT_AP_SSID,
		    ps_nibble_to_ascii(sys_mac_addr[4] >> 4),
		    ps_nibble_to_ascii(sys_mac_addr[4]),
		    ps_nibble_to_ascii(sys_mac_addr[5] >> 4),
	 	    ps_nibble_to_ascii(sys_mac_addr[5]));
	 	    
	// Other text fields start empty
	for (i=0; i<PS_SSID_MAX_LEN+1; i++) {
		local->ap_pw[i] = 0;
		local->sta_ssid[i] = 0;
		local->sta_pw[i] = 0;
	}
	
	// Network is always enabled to start
	local->flags = NET_INFO_FLAG_STARTUP_ENABLE;
	
	// Ethernet is configured to be a DHCP-served client
	if (iface == CTRL_IF_MODE_ETH) {
		local->flags |= NET_INFO_FLAG_CLIENT_MODE;
	}
	
	local->ap_ip_addr[3] = 192;
	local->ap_ip_addr[2] = 168;
	local->ap_ip_addr[1] = 4;
	local->ap_ip_addr[0] = 1;
	local->sta_ip_addr[3] = 192;
	local->sta_ip_addr[2] = 168;
	local->sta_ip_addr[1] = 4;
	local->sta_ip_addr[0] = 2;
	local->sta_netmask[3] = 255;
	local->sta_netmask[2] = 255;
	local->sta_netmask[1] = 255;
	local->sta_netmask[0] = 0;
}


static bool ps_write_lep_info()
{
	size_t required_size;
	esp_err_t err;
	
	required_size = sizeof(ps_lep_state_t);
	err = nvs_set_blob(ps_handle, lep_info_key, &ps_lep_state, required_size);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Set lep info blob failed with %d", err);
		return false;
	}
	
	err = nvs_commit(ps_handle);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Commit lep info failed with %d", err);
		return false;
	}
	
	return true;
}


static bool ps_read_lep_info()
{
	size_t required_size;
	esp_err_t err;
	
	required_size = sizeof(ps_lep_state_t);
	err = nvs_get_blob(ps_handle, lep_info_key, &ps_lep_state, &required_size);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Get lep info blob failed with %d", err);
		return false;
	}
	if (required_size != sizeof(ps_lep_state_t)) {
		ESP_LOGE(TAG, "Get lep info blob incorrect size %d (expected %d)", required_size, sizeof(ps_net_info_t));
		return false;
	}
	
	return true;
}


static bool ps_write_net_info(int iface)
{
	size_t required_size;
	esp_err_t err;
	
	required_size = sizeof(ps_net_info_t);
	if (iface == CTRL_IF_MODE_ETH) {
		err = nvs_set_blob(ps_handle, eth_info_key, &ps_eth_info, required_size);
	} else {
		err = nvs_set_blob(ps_handle, wifi_info_key, &ps_wifi_info, required_size);
	}
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Set network info blob failed with %d", err);
		return false;
	}
	
	err = nvs_commit(ps_handle);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Commit network info failed with %d", err);
		return false;
	}
	
	return true;
}


static bool ps_read_net_info(int iface)
{
	size_t required_size;
	esp_err_t err;
	
	required_size = sizeof(ps_net_info_t);
	if (iface == CTRL_IF_MODE_ETH) {
		err = nvs_get_blob(ps_handle, eth_info_key, &ps_eth_info, &required_size);
	} else {
		err = nvs_get_blob(ps_handle, wifi_info_key, &ps_wifi_info, &required_size);
	}
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Get network info blob failed with %d", err);
		return false;
	}
	if (required_size != sizeof(ps_net_info_t)) {
		ESP_LOGE(TAG, "Get network info blob incorrect size %d (expected %d)", required_size, sizeof(ps_net_info_t));
		return false;
	}
	
	return true;
}


static bool ps_read_old_net_info(int iface)
{
	size_t required_size;
	esp_err_t err;
	ps_old_net_info_t old_info;
	ps_net_info_t* new_infoP;
	
	// Read the old version sized blob
	required_size = sizeof(ps_old_net_info_t);
	if (iface == CTRL_IF_MODE_ETH) {
		err = nvs_get_blob(ps_handle, eth_info_key, &old_info, &required_size);
	} else {
		err = nvs_get_blob(ps_handle, wifi_info_key, &old_info, &required_size);
	}
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Get network old info blob failed with %d", err);
		return false;
	}
	if (required_size != sizeof(ps_old_net_info_t)) {
		ESP_LOGE(TAG, "Get network old info blob incorrect size %d (expected %d)", required_size, sizeof(ps_net_info_t));
		return false;
	}
	
	// Copy the old sized data into our current sized structure
	if (iface == CTRL_IF_MODE_ETH) {
		new_infoP = &ps_eth_info;
	} else {
		new_infoP = &ps_wifi_info;
	}
	strcpy(new_infoP->ap_ssid, old_info.ap_ssid);
	strcpy(new_infoP->sta_ssid, old_info.sta_ssid);
	strcpy(new_infoP->ap_pw, old_info.ap_pw);
	strcpy(new_infoP->sta_pw, old_info.sta_pw);
	new_infoP->flags = old_info.flags;
	for (int i=0; i<4; i++) {
		new_infoP->ap_ip_addr[i] = old_info.ap_ip_addr[i];
		new_infoP->sta_ip_addr[i] = old_info.sta_ip_addr[i];
		new_infoP->sta_netmask[i] = old_info.sta_netmask[i];
	}
	
	return true;
}


/**
 * Store a string at the specified location in our local buffer making sure it does
 * not exceed the available space and is terminated with a null character.
 */
static void ps_store_string(char* dst, char* src, uint8_t max_len)
{
	char c;
	int i = 0;
	bool saw_s_end = false;
	
	while (i < max_len) {
		if (!saw_s_end) {
			// Copy string data
			c = *(src+i);
			*(dst+i) = c;
			if (c == 0) saw_s_end = true;
		} else {
			// Pad with nulls
			*(dst+i) = 0;
		}
		i++;
	}
	
	// One final null in case the string was max_len long
	*(dst+i) = 0;
}
