/*
 * Firmware Update Utilities
 *
 * Utility functions to implement a firmware update mechanism using two OTA slots
 * in flash memory.
 *
 * Copyright 2021 Dan Julio
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
#include "upd_utilities.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "esp_log.h"
#include <string.h>



//
// Update Utilities variables
//
static const char* TAG = "upd_utilities";

// State
static char exp_version[UPD_MAX_VER_LEN];
static uint32_t total_len;
static uint32_t cur_len;
static const esp_partition_t *update_partition = NULL;
static esp_ota_handle_t update_handle = 0;




//
// Update Utilities API
//
bool upd_init(uint32_t len, char* version)
{
	esp_err_t err;
	
	// Get the next update partition
	update_partition = esp_ota_get_next_update_partition(NULL);
	if (update_partition == NULL) {
		ESP_LOGE(TAG, "Could not get update partion");
		return false;
	} else {
		ESP_LOGI(TAG, "Update partion subtype %d at offset 0x%x", update_partition->subtype, update_partition->address);
	}
	
	err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
		return false;
	}
	
	// Save information about the update
	total_len = len;
	cur_len = 0;
	memset(exp_version, 0, UPD_MAX_VER_LEN);
	strncpy(exp_version, version, UPD_MAX_VER_LEN);
	
	return true;
}


bool upd_complete()
{
	esp_err_t err;
	
	err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed (%s)", esp_err_to_name(err));
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            ESP_LOGE(TAG, "   Image validation failed");
        }
        
        return false;
    }
    
    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)", esp_err_to_name(err));
        return false;
    }
    
	return true;
}


void upd_early_terminate()
{
	// Silently attempt to free up resources
	(void) esp_ota_end(update_handle);
}


bool upd_process_bytes(uint32_t start, uint32_t len, uint8_t* buf)
{
	bool success = true;
	esp_err_t err;
	
	// Validate incoming arguments
	if (start != cur_len) {
		ESP_LOGE(TAG, "upd_process_bytes exp %d does not match start %d", cur_len, start);
		return false;
	}
	
	// On the first transfer, check to make sure we're downloading the expected
	// version for our platform.  Note this assumes we get enough bytes (X) in the first
	// download to include the app description.  Otherwise we'll silently skip this test.
	// We shouldn't ever see this...
	// 
	if (start == 0) {
		const esp_app_desc_t* cur_app_infoP;
		esp_app_desc_t new_app_info;
		
		cur_app_infoP = esp_ota_get_app_description();
		
		if (len > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
			memcpy(&new_app_info, &buf[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));

			// Check starting with magic word
			if (new_app_info.magic_word != ESP_APP_DESC_MAGIC_WORD) {
				ESP_LOGE(TAG, "FW Binary does not appear to be legal ESP32 binary, got %x instead of %x",
				         new_app_info.magic_word, ESP_APP_DESC_MAGIC_WORD);
				success = false;
			}
			else if (strcmp(new_app_info.version, exp_version) != 0) {
				// Version does not match
				ESP_LOGE(TAG, "FW Binary version %s does not match expected %s", new_app_info.version, exp_version);
				success = false;
			}
			else if (strcmp(new_app_info.project_name, cur_app_infoP->project_name) != 0) {
				ESP_LOGE(TAG, "FW Binary name %s does not match expected %s", new_app_info.project_name, cur_app_infoP->project_name);
				success = false;
			}
		}
	}
	
	// Write the data to the selected flash partition
	if (success) {
		err = esp_ota_write(update_handle, (const void *)buf, len);
		if (err != ESP_OK) {
			ESP_LOGE(TAG, "esp_ota_write failed at %d for %d bytes (%s)", start, len, esp_err_to_name(err));
			success = false;
		}
	}
	
	if (!success) {
		// Silently free up allocated resources
		err = esp_ota_end(update_handle);
	}
	
	// Keep track of downloaded bytes
	cur_len += len;
	
	return success;
}

