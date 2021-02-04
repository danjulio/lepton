/*
 * Persistent Storage Module
 *
 * Manage the persistent storage kept in the ESP32 NVS and provide access
 * routines to it.
 *
 * NOTE: It is assumed that only one task will access persistent storage at a time.
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
#include "ps_utilities.h"
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

// Stored Wifi Flags bitmask
#define PS_WIFI_FLAG_MASK      (WIFI_INFO_FLAG_STARTUP_ENABLE | WIFI_INFO_FLAG_CL_STATIC_IP | WIFI_INFO_FLAG_CLIENT_MODE)

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
typedef struct ps_lep_state_t {
	uint8_t flags;
	uint8_t emissivity;
	uint8_t gain_mode;
} ps_lep_state_t;


// Stored Wifi Parameters (strings are null-terminated)
typedef struct ps_wifi_info_t {
	char ap_ssid[PS_SSID_MAX_LEN+1];
	char sta_ssid[PS_SSID_MAX_LEN+1];
	char ap_pw[PS_PW_MAX_LEN+1];
	char sta_pw[PS_PW_MAX_LEN+1];
	uint8_t flags;
	uint8_t ap_ip_addr[4];
	uint8_t sta_ip_addr[4];
	uint8_t sta_netmask[4];
} ps_wifi_info_t;



//
// PS Utilities Internal variables
//
static const char* TAG = "ps_utilities";

// NVS Keys
static const char* lep_info_key = "lep_state";
static const char* wifi_info_key = "wifi_info";

// Local copies
static ps_lep_state_t ps_lep_state;
static ps_wifi_info_t ps_wifi_info;

// NVS namespace handle
static nvs_handle_t ps_handle;



//
// PS Utilities Forward Declarations for internal functions
//
static void ps_default_lep_info();
static void ps_default_wifi_info();
static bool ps_write_lep_info();
static bool ps_read_lep_info();
static bool ps_write_wifi_info();
static bool ps_read_wifi_info();
static void ps_store_string(char* dst, char* src, uint8_t max_len);
static char ps_nibble_to_ascii(uint8_t n);



//
// PS Utilities API
//

/**
 * Initialize persistent storage
 *   - Load our local buffer
 *   - Initialize it and the NVS with valid data if necessary
 */
bool ps_init()
{
	bool success = true;
	esp_err_t err;
	size_t required_size;
	
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
	
	// Wifi Info
	err = nvs_get_blob(ps_handle, wifi_info_key, NULL, &required_size);
	if ((err != ESP_OK) && (err != ESP_ERR_NVS_NOT_FOUND)) {
		ESP_LOGE(TAG, "NVS get_blob wifi size failed with err %d", err);
		return false;
	}
	if ((required_size == 0) || (required_size != sizeof(ps_wifi_info_t))) {
		if (required_size == 0) {
			ESP_LOGI(TAG, "Initializing NVS wifi info");
		} else {
			ESP_LOGI(TAG, "Re-initializing NVS wifi info");
		}
		ps_default_wifi_info();
		success &= ps_write_wifi_info();
	} else {
		ESP_LOGI(TAG, "Reading NVS wifi info");
		success &= ps_read_wifi_info();
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
	ps_lep_state.flags = (state->agc_set_enabled ? PS_LEP_AGC_EN_MASK : 0);	             
	ps_lep_state.emissivity = (uint8_t) state->emissivity;
	ps_lep_state.gain_mode = (uint8_t) state->gain_mode;
	
	if (!ps_write_lep_info()) {
		ESP_LOGE(TAG, "Failed to save lep data to NVS Storage");
	}
}


void ps_get_wifi_info(wifi_info_t* info)
{
	int i;
	
	// Get from our local copy
	strcpy(info->ap_ssid, (const char*) ps_wifi_info.ap_ssid);
	strcpy(info->ap_pw, (const char*) ps_wifi_info.ap_pw);
	strcpy(info->sta_ssid, (const char*) ps_wifi_info.sta_ssid);
	strcpy(info->sta_pw, (const char*) ps_wifi_info.sta_pw);
	
	info->flags = ps_wifi_info.flags & PS_WIFI_FLAG_MASK;
	
	for (i=0; i<4; i++) {
		info->ap_ip_addr[i] = ps_wifi_info.ap_ip_addr[i];
		info->sta_ip_addr[i] = ps_wifi_info.sta_ip_addr[i];
		info->sta_netmask[i] = ps_wifi_info.sta_netmask[i];
	}
}


void ps_set_wifi_info(const wifi_info_t* info)
{
	int i;
	
	ps_store_string(ps_wifi_info.ap_ssid, info->ap_ssid, PS_SSID_MAX_LEN);
	ps_store_string(ps_wifi_info.ap_pw, info->ap_pw, PS_PW_MAX_LEN);
	ps_store_string(ps_wifi_info.sta_ssid, info->sta_ssid, PS_SSID_MAX_LEN);
	ps_store_string(ps_wifi_info.sta_pw, info->sta_pw, PS_PW_MAX_LEN);
	
	ps_wifi_info.flags = info->flags & PS_WIFI_FLAG_MASK;
	
	for (i=0; i<4; i++) {
		 ps_wifi_info.ap_ip_addr[i] = info->ap_ip_addr[i];
		 ps_wifi_info.sta_ip_addr[i] = info->sta_ip_addr[i];
		 ps_wifi_info.sta_netmask[i] = info->sta_netmask[i];
	}
	
	if (!ps_write_wifi_info()) {
		ESP_LOGE(TAG, "Failed to write WiFi data to NVS Storage");
	}
}


bool ps_reinit_wifi()
{
	bool success;
	
	ESP_LOGI(TAG, "Re-initializing NVS wifi info");
	
	ps_default_wifi_info();
	success = ps_write_wifi_info();
	
	return success;
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


static void ps_default_wifi_info()
{
	int i;
	uint8_t sys_mac_addr[6];
	
	// Wifi parameters
	//
	// Get the system's default MAC address and add 1 to match the "Soft AP" mode
	// (see "Miscellaneous System APIs" in the ESP-IDF documentation)
	esp_efuse_mac_get_default(sys_mac_addr);
	sys_mac_addr[5] = sys_mac_addr[5] + 1;
	
	// Construct our default AP SSID/Camera name
	for (i=0; i<PS_SSID_MAX_LEN+1; i++) {
		ps_wifi_info.ap_ssid[i] = 0;
		ps_wifi_info.ap_pw[i] = 0;
		ps_wifi_info.sta_ssid[i] = 0;
		ps_wifi_info.sta_pw[i] = 0;
	}
	sprintf(ps_wifi_info.ap_ssid, "%s%c%c%c%c", PS_DEFAULT_AP_SSID,
		    ps_nibble_to_ascii(sys_mac_addr[4] >> 4),
		    ps_nibble_to_ascii(sys_mac_addr[4]),
		    ps_nibble_to_ascii(sys_mac_addr[5] >> 4),
	 	    ps_nibble_to_ascii(sys_mac_addr[5]));
	
	ps_wifi_info.flags = WIFI_INFO_FLAG_STARTUP_ENABLE;
	
	ps_wifi_info.ap_ip_addr[3] = 192;
	ps_wifi_info.ap_ip_addr[2] = 168;
	ps_wifi_info.ap_ip_addr[1] = 4;
	ps_wifi_info.ap_ip_addr[0] = 1;
	ps_wifi_info.sta_ip_addr[3] = 192;
	ps_wifi_info.sta_ip_addr[2] = 168;
	ps_wifi_info.sta_ip_addr[1] = 4;
	ps_wifi_info.sta_ip_addr[0] = 2;
	ps_wifi_info.sta_netmask[3] = 255;
	ps_wifi_info.sta_netmask[2] = 255;
	ps_wifi_info.sta_netmask[1] = 255;
	ps_wifi_info.sta_netmask[0] = 0;
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
		ESP_LOGE(TAG, "Get lep info blob incorrect size %d (expected %d)", required_size, sizeof(ps_wifi_info_t));
		return false;
	}
	
	return true;
}


static bool ps_write_wifi_info()
{
	size_t required_size;
	esp_err_t err;
	
	required_size = sizeof(ps_wifi_info_t);
	err = nvs_set_blob(ps_handle, wifi_info_key, &ps_wifi_info, required_size);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Set wifi info blob failed with %d", err);
		return false;
	}
	
	err = nvs_commit(ps_handle);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Commit wifi info failed with %d", err);
		return false;
	}
	
	return true;
}


static bool ps_read_wifi_info()
{
	size_t required_size;
	esp_err_t err;
	
	required_size = sizeof(ps_wifi_info_t);
	err = nvs_get_blob(ps_handle, wifi_info_key, &ps_wifi_info, &required_size);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Get wifi info blob failed with %d", err);
		return false;
	}
	if (required_size != sizeof(ps_wifi_info_t)) {
		ESP_LOGE(TAG, "Get wifi info blob incorrect size %d (expected %d)", required_size, sizeof(ps_wifi_info_t));
		return false;
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


/**
 * Return an ASCII character version of a 4-bit hexadecimal number
 */
static char ps_nibble_to_ascii(uint8_t n)
{
	n = n & 0x0F;
	
	if (n < 10) {
		return '0' + n;
	} else {
		return 'A' + n - 10;
	}
}
