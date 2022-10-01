/*
 * JSON related utilities
 *
 * Contains functions to generate json text objects and parse text objects into the
 * json objects used by tCam.  Uses the cjson library.  Image data is formatted
 * using Base64 encoding.
 *
 * This module uses two pre-allocated buffers for the json text objects.  One for image
 * data (that can be stored as a file or sent to the host) and one for smaller responses
 * to the host.
 *
 * Be sure to read the requirements about freeing allocated buffers or objects in
 * the function description.  Or BOOM.
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
#include "json_utilities.h"
#include "ps_utilities.h"
#include "lepton_utilities.h"
#include "time_utilities.h"
#include "cmd_utilities.h"
#include "ps_utilities.h"
#include "upd_utilities.h"
#include "ctrl_task.h"
#include "system_config.h"
#include "vospi.h"
#include "mbedtls/base64.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <string.h>


//
// JSON Utilities internal constants
//
#define CCI_BUF_LEN 1024



//
// Command parser
//
typedef struct {
	const char* cmd_name;
	int cmd_index;
} cmd_name_t;

const cmd_name_t command_list[CMD_NUM] = {
	{CMD_GET_STATUS_S, CMD_GET_STATUS},
	{CMD_GET_IMAGE_S, CMD_GET_IMAGE},
	{CMD_GET_CONFIG_S, CMD_GET_CONFIG},
	{CMD_SET_CONFIG_S, CMD_SET_CONFIG},
	{CMD_SET_TIME_S, CMD_SET_TIME},
	{CMD_GET_WIFI_S, CMD_GET_WIFI},
	{CMD_SET_WIFI_S, CMD_SET_WIFI},
	{CMD_SET_SPOT_S, CMD_SET_SPOT},
	{CMD_STREAM_ON_S, CMD_STREAM_ON},
	{CMD_STREAM_OFF_S, CMD_STREAM_OFF},
	{CMD_TAKE_PIC_S, CMD_TAKE_PIC},
	{CMD_RECORD_ON_S, CMD_RECORD_ON},
	{CMD_RECORD_OFF_S, CMD_RECORD_OFF},
	{CMD_POWEROFF_S, CMD_POWEROFF},
	{CMD_RUN_FFC_S, CMD_RUN_FFC},
	{CMD_GET_FS_LIST_S, CMD_GET_FS_LIST},
	{CMD_GET_FS_FILE_S, CMD_GET_FS_FILE},
	{CMD_DEL_FS_OBJ_S, CMD_DEL_FS_OBJ},
	{CMD_GET_LEP_CCI_S, CMD_GET_LEP_CCI},
	{CMD_SET_LEP_CCI_S, CMD_SET_LEP_CCI},
	{CMD_FW_UPD_REQ_S, CMD_FW_UPD_REQ},
	{CMD_FW_UPD_SEG_S, CMD_FW_UPD_SEG},
	{CMD_DUMP_SCREEN_S, CMD_DUMP_SCREEN}
};



//
// JSON Utilities variables
//
static const char* TAG = "json_utilities";

static char* json_response_text;    // Loaded for response data

static unsigned char* base64_lep_data;
static unsigned char* base64_lep_telem_data;
static unsigned char* base64_cci_reg_data;

static uint16_t* cci_buf;           // Used to hold Lepton CCI data from cmd or for rsp



//
// JSON Utilities Forward Declarations for internal functions
//
static bool json_add_lep_image_object(cJSON* parent, lep_buffer_t* lep_buffer);
static void json_free_lep_base64_image();
static bool json_add_lep_telem_object(cJSON* parent, lep_buffer_t* lep_buffer);
static void json_free_lep_base64_telem();
static bool json_add_cci_reg_base64_data(cJSON* parent, int len, uint16_t* buf);
static void json_free_cci_reg_base64_data();
static bool json_add_metadata_object(cJSON* parent);
static uint32_t json_generate_response_string(cJSON* root, char* json_string);
static bool json_ip_string_to_array(uint8_t* ip_array, char* ip_string);



//
// JSON Utilities API
//

/**
 * Pre-allocate buffers
 */
bool json_init()
{
	// Get memory for the json text output strings
	json_response_text = heap_caps_malloc(JSON_MAX_RSP_TEXT_LEN, MALLOC_CAP_8BIT);
	if (json_response_text == NULL) {
		ESP_LOGE(TAG, "Could not allocate json_response_text buffer");
		return false;
	}
	
	cci_buf = heap_caps_malloc(CCI_BUF_LEN, MALLOC_CAP_SPIRAM);
	if (cci_buf == NULL) {
		ESP_LOGE(TAG, "Could not allocate cci data buffer");
		return false;
	}
	
	return true;
}


/**
 * Create a json command object from a string, returns NULL if it fails.  The object
 * will need to be freed using json_free_cmd when it is no longer necessary.
 */
cJSON* json_get_cmd_object(char* json_string)
{
	cJSON* ret;
	
	ret = cJSON_Parse(json_string);
	if (ret == NULL) {
		ESP_LOGE(TAG, "Parse error at %d", cJSON_GetErrorPtr() - json_string);
	}
	return ret;
}


/**
 * Update a formatted json string in a pre-allocated json text image buffer containing
 * three json objects for a lepton image buffer.  Returns a non-zero length for a successful
 * operation.
 *   - Image meta-data
 *   - Base64 encoded raw image from the Lepton
 *   - Base64 encoded telemetry from the Lepton
 *
 * This function handles its own memory management.
 */
uint32_t json_get_image_file_string(char* json_image_text, lep_buffer_t* lep_buffer)
{
	bool success;
	int len = 0;
	cJSON* root;
	
	root = cJSON_CreateObject();
	if (root == NULL) return 0;
	
	// Construct the json object
	success = json_add_metadata_object(root);
	if (success) {
		success = json_add_lep_image_object(root, lep_buffer);
		if (success) {
			success = json_add_lep_telem_object(root, lep_buffer);
			if (!success) {
				// Free lep_image that was already allocated
				json_free_lep_base64_image();
			}
		}
	}
	
	// Tightly print the object to our buffer
	if (success) {
		if (cJSON_PrintPreallocated(root, json_image_text, JSON_MAX_IMAGE_TEXT_LEN, false) == 0) {
			len = 0;
		} else {
			len = strlen(json_image_text);
		}
		
		// Free the base-64 converted image strings
		json_free_lep_base64_image();
		json_free_lep_base64_telem();
	} else {
		ESP_LOGE(TAG, "failed to create json image text");
	}
	
	cJSON_Delete(root);
	
	return len;
}


/**
 * Return a formatted json string containing the camera's operating parameters in
 * response to the get_config commmand.  Include the delimitors since this string
 * will be sent via the socket interface
 */
char* json_get_config(uint32_t* len)
{
	cJSON* root;
	cJSON* config;
	json_config_t* lep_stP;
	
	// Get state
	lep_stP = system_get_lep_st();
	
	// Create and add to the config object
	root=cJSON_CreateObject();
	if (root == NULL) return NULL;
	
	cJSON_AddItemToObject(root, "config", config=cJSON_CreateObject());
	
	cJSON_AddNumberToObject(config, "agc_enabled", (const double) lep_stP->agc_set_enabled);
	cJSON_AddNumberToObject(config, "emissivity", (const double) lep_stP->emissivity);
	cJSON_AddNumberToObject(config, "gain_mode", (const double) lep_stP->gain_mode);
	
	// Tightly print the object into our buffer with delimitors
	*len = json_generate_response_string(root, json_response_text);
	
	cJSON_Delete(root);
	
	return json_response_text;
}


/**
 * Return a formatted json string containing the system status in response to the
 * get_status command.  Include the delimitors since this string will be sent via
 * the socket interface.
 */
char* json_get_status(uint32_t* len)
{
	char buf[80];
	cJSON* root;
	cJSON* status;
	int brd_type;
	int if_type;
	int model_field;
	net_info_t* net_info;
	uint8_t sys_mac_addr[6];
	const esp_app_desc_t* app_desc;
	tmElements_t te;
	
	// Get system information
	app_desc = esp_ota_get_app_description();	
	time_get(&te);
	
	// Create and add to the metadata object
	root=cJSON_CreateObject();
	if (root == NULL) return NULL;
	
	cJSON_AddItemToObject(root, "status", status=cJSON_CreateObject());
	
	ctrl_get_if_mode(&brd_type, &if_type);
	if (if_type == CTRL_IF_MODE_SIF) {
		// Get the system's default MAC address and add 1 to match the "Soft AP" mode
		// (see "Miscellaneous System APIs" in the ESP-IDF documentation)
		esp_efuse_mac_get_default(sys_mac_addr);
		sys_mac_addr[5] = sys_mac_addr[5] + 1;
		sprintf(buf, "%s%c%c%c%c", PS_DEFAULT_AP_SSID,
		    ps_nibble_to_ascii(sys_mac_addr[4] >> 4),
		    ps_nibble_to_ascii(sys_mac_addr[4]),
		    ps_nibble_to_ascii(sys_mac_addr[5] >> 4),
	 	    ps_nibble_to_ascii(sys_mac_addr[5]));
	 	cJSON_AddStringToObject(status, "Camera", buf);
	} else {
		net_info = net_get_info();
		cJSON_AddStringToObject(status, "Camera", net_info->ap_ssid);
	} 
	
	model_field = CAMERA_CAP_MASK_CORE;
	model_field |= (brd_type == CTRL_BRD_ETH_TYPE) ? CAMERA_MODEL_NUM_ETH : CAMERA_MODEL_NUM_WIFI;
	switch (if_type) {
		case CTRL_IF_MODE_ETH:
			model_field |= CAMERA_CAP_MASK_IF_ETH;
			break;
		case CTRL_IF_MODE_SIF:
			model_field |= CAMERA_CAP_MASK_IF_SIF;
			break;
		default:
			model_field |= CAMERA_CAP_MASK_IF_WIFI;
	}
	switch (lepton_get_model()) {
		case LEP_TYPE_3_5:
			model_field |= CAMERA_CAP_MASK_LEP3_5;
			break;
		case LEP_TYPE_3_0:
			model_field |= CAMERA_CAP_MASK_LEP3_0;
			break;
		case LEP_TYPE_3_1:
			model_field |= CAMERA_CAP_MASK_LEP3_1;
			break;
		default:
			model_field |= CAMERA_CAP_MASK_LEP_UNK;
	}
	cJSON_AddNumberToObject(status, "Model", model_field);
	
	cJSON_AddStringToObject(status, "Version", app_desc->version);
	
	sprintf(buf, "%d:%02d:%02d.%d", te.Hour, te.Minute, te.Second, te.Millisecond);
	cJSON_AddStringToObject(status, "Time", buf);
	sprintf(buf, "%d/%d/%02d", te.Month, te.Day, te.Year-30); // Year starts at 1970
	cJSON_AddStringToObject(status, "Date", buf);
	
	// Tightly print the object into our buffer with delimitors
	*len = json_generate_response_string(root, json_response_text);
	
	cJSON_Delete(root);
	
	return json_response_text;
}


/**
 * Return a formatted json string containing the wifi setup (minus password) in response
 * to the get_wifi command.  Include the delimitors since this string will be sent via
 * the socket interface.
 */
char* json_get_wifi(uint32_t* len)
{
	char ip_string[16];  // "XXX:XXX:XXX:XXX" + null
	cJSON* root;
	cJSON* wifi;
	net_info_t* net_infoP;
	
	// Get wifi information
	net_infoP = net_get_info();
	
	// Create and add to the metadata object
	root=cJSON_CreateObject();
	if (root == NULL) return NULL;
	
	cJSON_AddItemToObject(root, "wifi", wifi=cJSON_CreateObject());
	
	cJSON_AddStringToObject(wifi, "ap_ssid", net_infoP->ap_ssid);
	cJSON_AddStringToObject(wifi, "sta_ssid", net_infoP->sta_ssid);
	cJSON_AddNumberToObject(wifi, "flags", (const double) net_infoP->flags);
	
	sprintf(ip_string, "%d.%d.%d.%d", net_infoP->ap_ip_addr[3],
			                          net_infoP->ap_ip_addr[2],
			                          net_infoP->ap_ip_addr[1],
			                          net_infoP->ap_ip_addr[0]);
	cJSON_AddStringToObject(wifi, "ap_ip_addr", ip_string);
	
	sprintf(ip_string, "%d.%d.%d.%d", net_infoP->sta_ip_addr[3],
			                          net_infoP->sta_ip_addr[2],
			                          net_infoP->sta_ip_addr[1],
			                          net_infoP->sta_ip_addr[0]);
	cJSON_AddStringToObject(wifi, "sta_ip_addr", ip_string);
	
	sprintf(ip_string, "%d.%d.%d.%d", net_infoP->sta_netmask[3],
			                          net_infoP->sta_netmask[2],
			                          net_infoP->sta_netmask[1],
			                          net_infoP->sta_netmask[0]);
	cJSON_AddStringToObject(wifi, "sta_netmask", ip_string);
	
	sprintf(ip_string, "%d.%d.%d.%d", net_infoP->cur_ip_addr[3],
			                          net_infoP->cur_ip_addr[2],
			                          net_infoP->cur_ip_addr[1],
			                          net_infoP->cur_ip_addr[0]);
	cJSON_AddStringToObject(wifi, "cur_ip_addr", ip_string);
	
	// Tightly print the object into our buffer with delimitors
	*len = json_generate_response_string(root, json_response_text);
	
	cJSON_Delete(root);
	
	return json_response_text;
}


/**
 * Return a formatted json string containing the cmd and specified length of base64-encoded
 * CCI register data in response to the get_lep_cci/set_lep_cci command.  Include the delimiters
 * since this string will be sent via the socket interface.
 */
char* json_get_cci_response(uint16_t cmd, int cci_len, uint16_t status, uint16_t* buf, uint32_t* len)
{
	bool success = true;
	cJSON* root;
	cJSON* cci_reg;
	
	// Create and add to the cci_reg object
	root = cJSON_CreateObject();
	if (root == NULL) return NULL;
	
	cJSON_AddItemToObject(root, "cci_reg", cci_reg=cJSON_CreateObject());
	
	cJSON_AddNumberToObject(cci_reg, "command", cmd);
	cJSON_AddNumberToObject(cci_reg, "length", cci_len);
	cJSON_AddNumberToObject(cci_reg, "status", status);
	
	if (cci_len != 0) {
		success = json_add_cci_reg_base64_data(cci_reg, cci_len, buf);
	}
	
	// Tightly print the object into our buffer with delimiters
	if (success) {
		*len = json_generate_response_string(root, json_response_text);
		if (cci_len != 0) {
			json_free_cci_reg_base64_data();
		}
	}
	cJSON_Delete(root);
	
	return json_response_text;
}


/**
 * Return a formatted json string containing the start location and length for a chunk
 * of firmware to return as part of a OTA firmware update.  Include the delimiters
 * since this string will be sent via the socket interface.
 */
char* json_get_get_fw(uint32_t fw_start, uint32_t fw_len, uint32_t* len)
{
	cJSON* root;
	cJSON* get_fw;
	
	// Create and add to the cci_reg object
	root = cJSON_CreateObject();
	if (root == NULL) return NULL;
	
	cJSON_AddItemToObject(root, "get_fw", get_fw=cJSON_CreateObject());
	
	cJSON_AddNumberToObject(get_fw, "start", fw_start);
	cJSON_AddNumberToObject(get_fw, "length", fw_len);
	
	// Tightly print the object into our buffer with delimitors
	*len = json_generate_response_string(root, json_response_text);
	
	cJSON_Delete(root);
	
	return json_response_text;
}


/**
 * Generate a formatted json string containing the numeric and string info fields.  Add
 * delimiters for transmission over the network.  Returns string length.
 *
 * Note: Because this function is designed to be used by rsp_task, a valid buffer
 *       must be passed in for json_string.
 */
int json_get_cam_info(char* json_string, uint32_t info_value, char* info_string)
{
	cJSON* root;
	cJSON* response;
	uint32_t len = 0;
	
	// Create and add to the metadata object
	root=cJSON_CreateObject();
	if (root != NULL) {	
		// Create and add to the metadata object
		cJSON_AddItemToObject(root, "cam_info", response=cJSON_CreateObject());
		
		cJSON_AddNumberToObject(response, "info_value", info_value);
		cJSON_AddStringToObject(response, "info_string", info_string);
		
		// Tightly print the object into the buffer with delimiters
		len = json_generate_response_string(root, json_string);
		
		cJSON_Delete(root);
	}
	
	return (int) len;
}


/**
 * Parse a top level command object, returning the command number and a pointer to 
 * a json object containing "args".  The pointer is set to NULL if there are no args.
 */
bool json_parse_cmd(cJSON* cmd_obj, int* cmd, cJSON** cmd_args)
{
	 cJSON *cmd_type = cJSON_GetObjectItem(cmd_obj, "cmd");
	 char* cmd_name;
	 int i;
	 
	 if (cmd_type != NULL) {
	 	cmd_name = cJSON_GetStringValue(cmd_type);

	 	if (cmd_name != NULL) {
	 		*cmd = CMD_UNKNOWN;
	 		
	 		for (i=0; i<CMD_NUM; i++) {
	 			if (strcmp(cmd_name, command_list[i].cmd_name) == 0) {
	 				*cmd = command_list[i].cmd_index;
	 				break;
	 			}
	 		}
	 		
	 		*cmd_args = cJSON_GetObjectItem(cmd_obj, "args");
	 		
	 		return true;
	 	}
	 }

	 return false;
}


/**
 * Fill in a json_config_t struct with arguments from a set_config command, preserving
 * unmodified elements
 */
bool json_parse_set_config(cJSON* cmd_args, json_config_t* new_st)
{
	int item_count = 0;
	json_config_t* lep_stP;
	
	// Get existing settings to be possibly overwritten by the command
	lep_stP = system_get_lep_st();
	new_st->agc_set_enabled = lep_stP->agc_set_enabled;
	new_st->emissivity = lep_stP->emissivity;
	new_st->gain_mode = lep_stP->gain_mode;
	
	if (cmd_args != NULL) {
		if (cJSON_HasObjectItem(cmd_args, "agc_enabled")) {
			new_st->agc_set_enabled = cJSON_GetObjectItem(cmd_args, "agc_enabled")->valueint > 0 ? true : false;
			item_count++;
		}
		
		if (cJSON_HasObjectItem(cmd_args, "emissivity")) {
			new_st->emissivity = cJSON_GetObjectItem(cmd_args, "emissivity")->valueint;
			if (new_st->emissivity < 1) new_st->emissivity = 1;
			if (new_st->emissivity > 100) new_st->emissivity = 100;
			item_count++;
		}
		
		if (cJSON_HasObjectItem(cmd_args, "gain_mode")) {
			new_st->gain_mode = cJSON_GetObjectItem(cmd_args, "gain_mode")->valueint;
			if (new_st->gain_mode > SYS_GAIN_AUTO) new_st->gain_mode = SYS_GAIN_AUTO;
			item_count++;
		}
		
		return (item_count > 0);
	}
	
	return false;
}


/**
 * Get spotmeter coordinates
 */
bool json_parse_set_spotmeter(cJSON* cmd_args, uint16_t* r1, uint16_t* c1, uint16_t* r2, uint16_t* c2)
{
	int item_count = 0;
	int i;
	
	if (cmd_args != NULL) {
		if (cJSON_HasObjectItem(cmd_args, "r1")) {
			i = cJSON_GetObjectItem(cmd_args, "r1")->valueint;
			if (i < 0) i = 0;
			if (i > (LEP_HEIGHT-2)) i = LEP_HEIGHT - 2;
			*r1 = i;
			item_count++;
		}
		
		if (cJSON_HasObjectItem(cmd_args, "c1")) {
			i = cJSON_GetObjectItem(cmd_args, "c1")->valueint;
			if (i < 0) i = 0;
			if (i > (LEP_WIDTH-2)) i = LEP_WIDTH - 2;
			*c1 = i;
			item_count++;
		}
		
		if (cJSON_HasObjectItem(cmd_args, "r2")) {
			i = cJSON_GetObjectItem(cmd_args, "r2")->valueint;
			if (i < (*r1+1)) i = *r1 + 1;
			if (i > (LEP_HEIGHT-1)) i = LEP_HEIGHT - 1;
			*r2 = i;
			item_count++;
		}
		
		if (cJSON_HasObjectItem(cmd_args, "c2")) {
			i = cJSON_GetObjectItem(cmd_args, "c2")->valueint;
			if (i < (*c1+1)) i = *c1 + 1;
			if (i > (LEP_WIDTH-1)) i = LEP_WIDTH - 1;
			*c2 = i;
			item_count++;
		}
		
		return (item_count == 4);
	}
	
	return false;
}


/**
 * Fill in a tmElements object with arguments from a set_time command
 */
bool json_parse_set_time(cJSON* cmd_args, tmElements_t* te)
{
	int item_count = 0;
	
	if (cmd_args != NULL) {
		if (cJSON_HasObjectItem(cmd_args, "sec")) {
			te->Second = cJSON_GetObjectItem(cmd_args, "sec")->valueint; // 0 - 59
			item_count++;
		}
		
		if (cJSON_HasObjectItem(cmd_args, "min")) {
			te->Minute = cJSON_GetObjectItem(cmd_args, "min")->valueint; // 0 - 59
			item_count++;
		}
		
		if (cJSON_HasObjectItem(cmd_args, "hour")) {
			te->Hour   = cJSON_GetObjectItem(cmd_args, "hour")->valueint; // 0 - 23
			item_count++;
		}
		
		if (cJSON_HasObjectItem(cmd_args, "dow")) {
			te->Wday   = cJSON_GetObjectItem(cmd_args, "dow")->valueint; // 1 - 7
			item_count++;
		}
		
		if (cJSON_HasObjectItem(cmd_args, "day")) {
			te->Day    = cJSON_GetObjectItem(cmd_args, "day")->valueint; // 1 - 31
			item_count++;
		}
		
		if (cJSON_HasObjectItem(cmd_args, "mon")) {
			te->Month  = cJSON_GetObjectItem(cmd_args, "mon")->valueint; // 1 - 12
			item_count++;
		}
		
		if (cJSON_HasObjectItem(cmd_args, "year")) {
			te->Year   = cJSON_GetObjectItem(cmd_args, "year")->valueint; // offset from 1970
			item_count++;
		}
		
		return (item_count == 7);
	}
	
	return false;
}


/**
 * Fill in a net_info_t object with arguments from a set_wifi command, preserving
 * unmodified elements
 */
bool json_parse_set_wifi(cJSON* cmd_args, net_info_t* new_net_info)
{
	char* s;
	int i;
	int item_count = 0;
	net_info_t* net_infoP;
	
	// Get existing settings
	net_infoP = net_get_info();
	
	if (cmd_args != NULL) {
		if (cJSON_HasObjectItem(cmd_args, "ap_ssid")) {
			s = cJSON_GetObjectItem(cmd_args, "ap_ssid")->valuestring;
			i = strlen(s);
			if (i == 0) {
				ESP_LOGE(TAG, "set_wifi zero length ap_ssid");
				return false;
			} else if (i <= PS_SSID_MAX_LEN) {
				strcpy(new_net_info->ap_ssid, s);
				item_count++;
			} else {
				ESP_LOGE(TAG, "set_wifi ap_ssid: %s too long", s);
				return false;
			}
		} else {
			strcpy(new_net_info->ap_ssid, net_infoP->ap_ssid);
		}
		
		if (cJSON_HasObjectItem(cmd_args, "sta_ssid")) {
			s = cJSON_GetObjectItem(cmd_args, "sta_ssid")->valuestring;
			i = strlen(s);
			if (i == 0) {
				ESP_LOGE(TAG, "set_wifi zero length sta_ssid");
				return false;
			} else if (i <= PS_SSID_MAX_LEN) {
				strcpy(new_net_info->sta_ssid, s);
				item_count++;
			} else {
				ESP_LOGE(TAG, "set_wifi sta_ssid: %s too long", s);
				return false;
			}
		} else {
			strcpy(new_net_info->sta_ssid, net_infoP->sta_ssid);
		}
		
		if (cJSON_HasObjectItem(cmd_args, "ap_pw")) {
			s = cJSON_GetObjectItem(cmd_args, "ap_pw")->valuestring;
			i = strlen(s);
			if ((i >= 8) && (i <= PS_PW_MAX_LEN)) {
				strcpy(new_net_info->ap_pw, s);
				item_count++;
			} else {
				ESP_LOGE(TAG, "set_wifi ap_pw: %s must be between 8 and %d characters", s, PS_PW_MAX_LEN);
				return false;
			}
		} else {
			strcpy(new_net_info->ap_pw, net_infoP->ap_pw);
		}
		
		if (cJSON_HasObjectItem(cmd_args, "sta_pw")) {
			s = cJSON_GetObjectItem(cmd_args, "sta_pw")->valuestring;
			i = strlen(s);
			if ((i >= 8) && (i <= PS_PW_MAX_LEN)) {
				strcpy(new_net_info->sta_pw, s);
				item_count++;
			} else {
				ESP_LOGE(TAG, "set_wifi sta_pw: %s must be between 8 and %d characters", s, PS_PW_MAX_LEN);
				return false;
			}
		} else {
			strcpy(new_net_info->sta_pw, net_infoP->sta_pw);
		}
		
		if (cJSON_HasObjectItem(cmd_args, "flags")) {
			new_net_info->flags = (uint8_t) cJSON_GetObjectItem(cmd_args, "flags")->valueint;
			item_count++;
		} else {
			new_net_info->flags = net_infoP->flags;
		}
		
		if (cJSON_HasObjectItem(cmd_args, "ap_ip_addr")) {
			s = cJSON_GetObjectItem(cmd_args, "ap_ip_addr")->valuestring;
			if (json_ip_string_to_array(new_net_info->ap_ip_addr, s)) {
				item_count++;
			} else {
				ESP_LOGE(TAG, "Illegal set_wifi ap_ip_addr: %s", s);
				return false;
			}
		} else {
			for (i=0; i<4; i++) new_net_info->ap_ip_addr[i] = net_infoP->ap_ip_addr[i];
		}
		
		if (cJSON_HasObjectItem(cmd_args, "sta_ip_addr")) {
			s = cJSON_GetObjectItem(cmd_args, "sta_ip_addr")->valuestring;
			if (json_ip_string_to_array(new_net_info->sta_ip_addr, s)) {
				item_count++;
			} else {
				ESP_LOGE(TAG, "Illegal set_wifi sta_ip_addr: %s", s);
				return false;
			}
		} else {
			for (i=0; i<4; i++) new_net_info->sta_ip_addr[i] = net_infoP->sta_ip_addr[i];
		}
		
		if (cJSON_HasObjectItem(cmd_args, "sta_netmask")) {
			s = cJSON_GetObjectItem(cmd_args, "sta_netmask")->valuestring;
			if (json_ip_string_to_array(new_net_info->sta_netmask, s)) {
				item_count++;
			} else {
				ESP_LOGE(TAG, "Illegal set_wifi sta_netmask: %s", s);
				return false;
			}
		} else {
			for (i=0; i<4; i++) new_net_info->sta_netmask[i] = net_infoP->sta_netmask[i];
		}
		
		// Just copy existing address over
		for (i=0; i<4; i++) new_net_info->cur_ip_addr[i] = net_infoP->cur_ip_addr[i];
		
		return (item_count > 0);
	}
	
	return false;
}


/**
 * Get the stream_on arguments
 */
bool json_parse_stream_on(cJSON* cmd_args, uint32_t* delay_ms, uint32_t* num_frames)
{
	int i;
	
	if (cmd_args != NULL) {
		if (cJSON_HasObjectItem(cmd_args, "delay_msec")) {
			i = cJSON_GetObjectItem(cmd_args, "delay_msec")->valueint;
			if (i < 0) i = 0;
			*delay_ms = i;
		} else {
			*delay_ms = 0;
		}
		
		if (cJSON_HasObjectItem(cmd_args, "num_frames")) {
			i = cJSON_GetObjectItem(cmd_args, "num_frames")->valueint;
			if (i < 0) i = 0;
			*num_frames = i;
		} else {
			*num_frames = 0;
		}
	} else {
		// Assume old-style command and setup fastest possible streaming
		*delay_ms = 0;
		*num_frames = 0;
	}
	
	return true;
}


/**
 * Get the get_lep_cci arguments.  Pass our cci_buf back to the calling code to hold
 * the read data.
 */
bool json_parse_get_lep_cci(cJSON* cmd_args, uint16_t* cmd, int* len, uint16_t** buf)
{
	int i;
	int item_count = 0;
	
	if (cmd_args != NULL) {
		if (cJSON_HasObjectItem(cmd_args, "command")) {
			i = cJSON_GetObjectItem(cmd_args, "command")->valueint;
			*cmd = (uint16_t) i;
			item_count++;
		}
		
		if (cJSON_HasObjectItem(cmd_args, "length")) {
			i = cJSON_GetObjectItem(cmd_args, "length")->valueint;
			*len = i;
			item_count++;
		}
		
		*buf = cci_buf;
		
		return(item_count == 2);
	}
	
	return false;
}


/**
 * Get the set_lep_cci arguments.  Fill our cci_buf with register data and pass it back
 * to the calling code.
 */
bool json_parse_set_lep_cci(cJSON* cmd_args, uint16_t* cmd, int* len, uint16_t** buf)
{
	char* data;
	int i;
	int item_count = 0;
	size_t dec_len;
	
	if (cmd_args != NULL) {
		if (cJSON_HasObjectItem(cmd_args, "command")) {
			i = cJSON_GetObjectItem(cmd_args, "command")->valueint;
			*cmd = (uint16_t) i;
			item_count++;
		}
		
		if (cJSON_HasObjectItem(cmd_args, "length")) {
			i = cJSON_GetObjectItem(cmd_args, "length")->valueint;
			*len = i;
			item_count++;
		}
		
		if (item_count == 2) {
			if (cJSON_HasObjectItem(cmd_args, "data")) {
				data = cJSON_GetObjectItem(cmd_args, "data")->valuestring;
				
				// Decode
				i = mbedtls_base64_decode((unsigned char*) cci_buf, CCI_BUF_LEN, &dec_len, (const unsigned char*) data, strlen(data));
				if (i != 0) {
					ESP_LOGE(TAG, "Base 64 CCI Register data decode failed - %d (%d bytes decoded)", i, dec_len);
					return false;
				}
			}
		}
		
		*buf = cci_buf;
		
		return(item_count == 2);
	}
	
	return false;
}


bool json_parse_fw_upd_request(cJSON* cmd_args, uint32_t* len, char* ver)
{
	char* v;
	int i;
	int item_count = 0;
	
	if (cmd_args != NULL) {
		if (cJSON_HasObjectItem(cmd_args, "length")) {
			i = cJSON_GetObjectItem(cmd_args, "length")->valueint;
			*len = (uint32_t) i;
			item_count++;
		}
		
		if (cJSON_HasObjectItem(cmd_args, "version")) {
			v = cJSON_GetObjectItem(cmd_args, "version")->valuestring;
			strncpy(ver, v, UPD_MAX_VER_LEN);
			item_count++;
		}
		
		return(item_count == 2);
	}
	
	return false;
}


bool json_parse_fw_segment(cJSON* cmd_args, uint32_t* start, uint32_t* len, uint8_t* buf)
{
	char* data;
	int i;
	int item_count = 0;
	size_t dec_len;
	size_t enc_len;
	
	if (cmd_args != NULL) {
		if (cJSON_HasObjectItem(cmd_args, "start")) {
			i = cJSON_GetObjectItem(cmd_args, "start")->valueint;
			*start = (uint32_t) i;
			item_count++;
		}
		
		if (cJSON_HasObjectItem(cmd_args, "length")) {
			i = cJSON_GetObjectItem(cmd_args, "length")->valueint;
			*len = (uint32_t) i;
			item_count++;
		}
		
		if (item_count == 2) {
			if (cJSON_HasObjectItem(cmd_args, "data")) {
				data = cJSON_GetObjectItem(cmd_args, "data")->valuestring;
				
				// Decode
				enc_len = strlen(data);
				i = mbedtls_base64_decode(buf, FM_UPD_CHUNK_MAX_LEN, &dec_len, (const unsigned char*) data, enc_len);
				if (i != 0) {
					ESP_LOGE(TAG, "Base 64 FW segment data decode failed - %d (%d bytes decoded)", i, dec_len);
					return false;
				}
				
				item_count++;
			}
		}
		
		return(item_count == 3);
	}
	
	return false;
}



/**
 * Free the json command object
 */
void json_free_cmd(cJSON* cmd)
{
	if (cmd != NULL) cJSON_Delete(cmd);
}



/**
 * Return a pointer to the name for a known cmd
 */
const char* json_get_cmd_name(int cmd)
{
	int i;
	
	for (i=0; i<CMD_NUM; i++) {
		if (command_list[i].cmd_index == cmd) {
			return command_list[i].cmd_name;
		}
	}
	
	return "Unknown";
}



//
// JSON Utilities internal functions
//

/**
 * Add a child object containing base64 encoded lepton image from the shared buffer
 *
 * Note: The encoded image string is held in an array that must be freed with
 * json_free_lep_base64_image() after the json object is converted to a string.
 */
static bool json_add_lep_image_object(cJSON* parent, lep_buffer_t* lep_buffer)
{
	size_t base64_obj_len;
	
	// Get the necessary length and allocate a buffer
	(void) mbedtls_base64_encode(base64_lep_data, 0, &base64_obj_len, 
								 (const unsigned char *) lep_buffer->lep_bufferP, LEP_NUM_PIXELS*2);
	base64_lep_data = heap_caps_malloc(base64_obj_len, MALLOC_CAP_SPIRAM);
	
	if (base64_lep_data != NULL) {
		// Base-64 encode the camera data
		if (mbedtls_base64_encode(base64_lep_data, base64_obj_len, &base64_obj_len, 
							      (const unsigned char *) lep_buffer->lep_bufferP,
	    	                       LEP_NUM_PIXELS*2) != 0) {
	                           
			ESP_LOGE(TAG, "failed to encode lepton image base64 text");
			free(base64_lep_data);
			return false;
		}
	} else {
		ESP_LOGE(TAG, "failed to allocate %d bytes for lepton image base64 text", base64_obj_len);
		return false;
	}
	
	// Add the encoded data as a reference since we're managing the buffer
	cJSON_AddItemToObject(parent, "radiometric", cJSON_CreateStringReference((char*) base64_lep_data));
	
	return true;
}


/**
 * Free the base64-encoded Lepton image.  Call this routine after printing the 
 * image json object.
 */
static void json_free_lep_base64_image()
{
	free(base64_lep_data);
}


/**
 * Add a child object containing base64 encoded lepton telemetry array from the shared buffer
 *
 * Note: The encoded telemetry string is held in an array that must be freed with
 * json_free_lep_base64_telem() after the json object is converted to a string.
 */
static bool json_add_lep_telem_object(cJSON* parent, lep_buffer_t* lep_buffer)
{
	size_t base64_obj_len;
	
	// Get the necessary length and allocate a buffer
	(void) mbedtls_base64_encode(base64_lep_telem_data, 0, &base64_obj_len, 
								 (const unsigned char *) lep_buffer->lep_telemP, LEP_TEL_WORDS*2);
	base64_lep_telem_data = heap_caps_malloc(base64_obj_len, MALLOC_CAP_SPIRAM);
	
	
	if (base64_lep_data != NULL) {
		// Base-64 encode the telemetry array
		if (mbedtls_base64_encode(base64_lep_telem_data, base64_obj_len, &base64_obj_len, 
							      (const unsigned char *) lep_buffer->lep_telemP,
	    	                       LEP_TEL_WORDS*2) != 0) {
	                           
			ESP_LOGE(TAG, "failed to encode lepton telemetry base64 text");
			free(base64_lep_telem_data);
			return false;
		}
	} else {
		ESP_LOGE(TAG, "failed to allocate %d bytes for lepton telemetry base64 text", base64_obj_len);
		return false;
	}
	
	// Add the encoded data as a reference since we're managing the buffer
	cJSON_AddItemToObject(parent, "telemetry", cJSON_CreateStringReference((char*) base64_lep_telem_data));
	
	return true;
}


/**
 * Free the base64-encoded Lepton telemetry string.  Call this routine after printing the 
 * telemetry json object.
 */
static void json_free_lep_base64_telem()
{
	free(base64_lep_telem_data);
}


/**
 * Add a child object containing base64 encoded CCI Register data from buf.
 *
 * Note: The encoded data is held in an array that must be freed with
 * json_free_cci_reg_base64_data() after the json object is converted to a string.
 */
static bool json_add_cci_reg_base64_data(cJSON* parent, int len, uint16_t* buf)
{
	size_t base64_obj_len;
	
	// Get the necessary length and allocate a buffer
	(void) mbedtls_base64_encode(base64_cci_reg_data, 0, &base64_obj_len, 
								 (const unsigned char *) buf, len*2);
	base64_cci_reg_data = heap_caps_malloc(base64_obj_len, MALLOC_CAP_SPIRAM);
	
	
	if (base64_cci_reg_data != NULL) {
		// Base-64 encode the CCI data
		if (mbedtls_base64_encode(base64_cci_reg_data, base64_obj_len, &base64_obj_len, 
							      (const unsigned char *) buf, len*2) != 0) {
	                           
			ESP_LOGE(TAG, "failed to encode CCI Register data base64 text");
			free(base64_cci_reg_data);
			return false;
		}
	} else {
		ESP_LOGE(TAG, "failed to allocate %d bytes for CCI Register base64 text", base64_obj_len);
		return false;
	}
	
	// Add the encoded data as a reference since we're managing the buffer
	cJSON_AddItemToObject(parent, "data", cJSON_CreateStringReference((char*) base64_cci_reg_data));
	
	return true;
}


/**
 * Free the base64-encoded CCI Register data string.  Call this routine after printing the
 * cci_reg json object.
 */
static void json_free_cci_reg_base64_data()
{
	free(base64_cci_reg_data);
}


/**
 * Add a child object containing image metadata to the parent.
 */
static bool json_add_metadata_object(cJSON* parent)
{
	int brd_type;
	int if_type;
	char buf[80];
	cJSON* meta;
	int model_field;
	net_info_t* net_info;
	uint8_t sys_mac_addr[6];
	const esp_app_desc_t* app_desc;
	tmElements_t te;
	
	// Get system information
	ctrl_get_if_mode(&brd_type, &if_type);
	app_desc = esp_ota_get_app_description();
	time_get(&te);
	
	// Create and add to the metadata object
	cJSON_AddItemToObject(parent, "metadata", meta=cJSON_CreateObject());
	
	if (if_type == CTRL_IF_MODE_SIF) {
		// Get the system's default MAC address and add 1 to match the "Soft AP" mode
		// (see "Miscellaneous System APIs" in the ESP-IDF documentation)
		esp_efuse_mac_get_default(sys_mac_addr);
		sys_mac_addr[5] = sys_mac_addr[5] + 1;
		sprintf(buf, "%s%c%c%c%c", PS_DEFAULT_AP_SSID,
		    ps_nibble_to_ascii(sys_mac_addr[4] >> 4),
		    ps_nibble_to_ascii(sys_mac_addr[4]),
		    ps_nibble_to_ascii(sys_mac_addr[5] >> 4),
	 	    ps_nibble_to_ascii(sys_mac_addr[5]));
	 	cJSON_AddStringToObject(meta, "Camera", buf);
	} else {
		net_info = net_get_info();
		cJSON_AddStringToObject(meta, "Camera", net_info->ap_ssid);
	}
	
	model_field = CAMERA_CAP_MASK_CORE;
	model_field |= (brd_type == CTRL_BRD_ETH_TYPE) ? CAMERA_MODEL_NUM_ETH : CAMERA_MODEL_NUM_WIFI;
	switch (if_type) {
		case CTRL_IF_MODE_ETH:
			model_field |= CAMERA_CAP_MASK_IF_ETH;
			break;
		case CTRL_IF_MODE_SIF:
			model_field |= CAMERA_CAP_MASK_IF_SIF;
			break;
		default:
			model_field |= CAMERA_CAP_MASK_IF_WIFI;
	}
	switch (lepton_get_model()) {
		case LEP_TYPE_3_5:
			model_field |= CAMERA_CAP_MASK_LEP3_5;
			break;
		case LEP_TYPE_3_0:
			model_field |= CAMERA_CAP_MASK_LEP3_0;
			break;
		case LEP_TYPE_3_1:
			model_field |= CAMERA_CAP_MASK_LEP3_1;
			break;
		default:
			model_field |= CAMERA_CAP_MASK_LEP_UNK;
	}
	cJSON_AddNumberToObject(meta, "Model", model_field);
	
	cJSON_AddStringToObject(meta, "Version", app_desc->version);
	
	sprintf(buf, "%d:%02d:%02d.%d", te.Hour, te.Minute, te.Second, te.Millisecond);
	cJSON_AddStringToObject(meta, "Time", buf);
	sprintf(buf, "%d/%d/%02d", te.Month, te.Day, te.Year-30);  // Year starts at 1970
	cJSON_AddStringToObject(meta, "Date", buf);
	
	return true;
}


/**
 * Tightly print a response into a string with delimitors for transmission over the network.
 * Returns length of the string.
 */
static uint32_t json_generate_response_string(cJSON* root, char* json_string)
{
	uint32_t len;
	
	json_string[0] = CMD_JSON_STRING_START;
	if (cJSON_PrintPreallocated(root, &json_string[1], JSON_MAX_RSP_TEXT_LEN, false) == 0) {
		len = 0;
	} else {
		len = (uint32_t) strlen(json_string);
		json_string[len] = CMD_JSON_STRING_STOP;
		json_string[len+1] = 0;
		len += 1;
	}
	
	return len;
}


/**
 * Convert a string in the form of "XXX.XXX.XXX.XXX" into a 4-byte array for net_info_t
 */
static bool json_ip_string_to_array(uint8_t* ip_array, char* ip_string)
{
	char c;;
	int i = 3;
	
	ip_array[i] = 0;
	while ((c = *ip_string++) != 0) {
		if (c == '.') {
			if (i == 0) {
				// Too many '.' characters
				return false;
			} else {
				// Setup for next byte
				ip_array[--i] = 0;
			}
		} else if ((c >= '0') && (c <= '9')) {
			// Add next numeric digit
			ip_array[i] = (ip_array[i] * 10) + (c - '0');
		} else {
			// Illegal character in string
			return false;
		}
	}
	
	return true;
}
