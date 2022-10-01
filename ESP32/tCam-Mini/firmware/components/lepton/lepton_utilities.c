/*
 * Lepton related utilities
 *
 * Contains utility and access functions for the Lepton.
 *
 * Note: I noticed that on occasion, the first time some commands run on the lepton
 * will fail either silently or with an error response code.  The access routines in
 * this module attempt to detect and retry the commands if necessary.
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
 * along with tCam.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include "lepton_utilities.h"
#include "cci.h"
#include "i2c.h"
#include "esp_system.h"
#include "esp_log.h"
#include "sys_utilities.h"
#include "vospi.h"
#include "system_config.h"
#include <string.h>



//
// Lepton Utilities variables
//
static const char* TAG = "lepton_utilities";

static bool lep_is_radiometric = false;
static int lep_type;



//
// Lepton Utilities API
//

bool lepton_init()
{
	char pn[33];
	uint32_t val, rsp;
	json_config_t* lep_stP = system_get_lep_st();
  
  	// Attempt to ping the Lepton to validate communication
  	// If this is successful, we assume further communication will be successful
  	rsp = cci_run_ping();
  	if (rsp != 0) {
  		ESP_LOGE(TAG, "Lepton communication failed (%d)", rsp);
  		return false;
	}
	
	// Get the Lepton type (part number) to determine if it's a Lepton 3.0 or 3.5
	cci_get_part_number(pn);
	ESP_LOGI(TAG, "Found Lepton, part number: %s", pn);
	if (strncmp(pn, "500-0771-01", 32) == 0) {
		ESP_LOGI(TAG, "  Radiometric Lepton 3.5");
		lep_is_radiometric = true;
		lep_type = LEP_TYPE_3_5;
	} else if (strncmp(pn, "500-0758-99", 32) == 0) {
		ESP_LOGI(TAG, "  Radiometric Lepton 3.1");
		lep_is_radiometric = true;
		lep_type = LEP_TYPE_3_1;
	} else if (strncmp(pn, "500-0726-01", 32) == 0) {
		ESP_LOGI(TAG, "  Non-radiometric Lepton 3.0");
		lep_is_radiometric = false;
		lep_type = LEP_TYPE_3_0;
	} else {
		ESP_LOGI(TAG, "  Unsupported Lepton");
		lep_is_radiometric = true;
		lep_type = LEP_TYPE_UNK;
	}
	
	if (lep_is_radiometric) {
		// Configure Radiometry for TLinear enabled, auto-resolution
		cci_set_radiometry_enable_state(CCI_RADIOMETRY_ENABLED);
		rsp = cci_get_radiometry_enable_state();
		ESP_LOGI(TAG, "Lepton Radiometry = %d", rsp);
		if (rsp != CCI_RADIOMETRY_ENABLED) {
			// Make one more effort
			vTaskDelay(pdMS_TO_TICKS(10));
			ESP_LOGI(TAG, "Retry Set Lepton Radiometry");
			cci_set_radiometry_enable_state(CCI_RADIOMETRY_ENABLED);
			rsp = cci_get_radiometry_enable_state();
			ESP_LOGI(TAG, "Lepton Radiometry = %d", rsp);
			if (rsp != CCI_RADIOMETRY_ENABLED) {
				ESP_LOGE(TAG, "Lepton communication failed (%d)", rsp);
				return false;
			}
		}
	
		// TLinear depends on AGC
		val = (lep_stP->agc_set_enabled) ? CCI_RADIOMETRY_TLINEAR_DISABLED : CCI_RADIOMETRY_TLINEAR_ENABLED;
		cci_set_radiometry_tlinear_enable_state(val);
		rsp = cci_get_radiometry_tlinear_enable_state();
		ESP_LOGI(TAG, "Lepton Radiometry TLinear = %d", rsp);
		if (rsp != val) {
			ESP_LOGE(TAG, "Lepton communication failed (%d)", rsp);
	  		return false;
		}
		
		cci_set_radiometry_tlinear_auto_res(CCI_RADIOMETRY_AUTO_RES_ENABLED);
		rsp = cci_get_radiometry_tlinear_auto_res();
		ESP_LOGI(TAG, "Lepton Radiometry Auto Resolution = %d", rsp);
		if (rsp != CCI_RADIOMETRY_AUTO_RES_ENABLED) {
			ESP_LOGE(TAG, "Lepton communication failed (%d)", rsp);
	  		return false;
		}
	}
	
	// Enable AGC calcs for a smooth transition between modes
	cci_set_agc_calc_enable_state(CCI_AGC_ENABLED);
	rsp = cci_get_agc_calc_enable_state();
	ESP_LOGI(TAG, "Lepton AGC Calcs = %d", rsp);
	if (rsp != CCI_AGC_ENABLED) {
		ESP_LOGE(TAG, "Lepton communication failed (%d)", rsp);
  		return false;
	}
	
	// AGC
	val = (lep_stP->agc_set_enabled) ? CCI_AGC_ENABLED : CCI_AGC_DISABLED;
	cci_set_agc_enable_state(val);
	rsp = cci_get_agc_enable_state();
	ESP_LOGI(TAG, "Lepton AGC = %d", rsp);
	if (rsp != val) {
		ESP_LOGE(TAG, "Lepton communication failed (%d)", rsp);
  		return false;
	}
	
	// Enable telemetry
	cci_set_telemetry_enable_state(CCI_TELEMETRY_ENABLED);
	rsp = cci_get_telemetry_enable_state();
	ESP_LOGI(TAG, "Lepton Telemetry = %d", rsp);
	if (rsp != CCI_TELEMETRY_ENABLED) {
		ESP_LOGE(TAG, "Lepton communication failed (%d)", rsp);
  		return false;
	}
	vospi_include_telem(true);
	
	// GAIN
	switch (lep_stP->gain_mode) {
		case SYS_GAIN_HIGH:
			val = LEP_SYS_GAIN_MODE_HIGH;
			break;
		case SYS_GAIN_LOW:
			val = LEP_SYS_GAIN_MODE_LOW;
			break;
		default:
			val = LEP_SYS_GAIN_MODE_AUTO;
	}
	cci_set_gain_mode(val);
	rsp = cci_get_gain_mode();
	ESP_LOGI(TAG, "Lepton Gain Mode = %d", rsp);
	if (rsp != val) {
		ESP_LOGE(TAG, "Lepton communication failed (%d)", rsp);
  		return false;
	}
	
	// Emissivity
	if (lep_is_radiometric) {
		lepton_emissivity(lep_stP->emissivity);
		ESP_LOGI(TAG, "Lepton Emissivity = %d%%", lep_stP->emissivity);
	}
  	
	// Finally enable VSYNC on Lepton GPIO3
	cci_set_gpio_mode(LEP_OEM_GPIO_MODE_VSYNC);
	rsp = cci_get_gpio_mode();
	ESP_LOGI(TAG, "Lepton GPIO Mode = %d", rsp);
	if (rsp != LEP_OEM_GPIO_MODE_VSYNC) {
		ESP_LOGE(TAG, "Lepton communication failed (%d)", rsp);
  		return false;
	}
	
	return true;
}


bool lepton_is_radiometric()
{
	return lep_is_radiometric;
}


int lepton_get_model()
{
	return lep_type;
}


void lepton_agc(bool en)
{
	if (en) {
		if (lep_is_radiometric) {
			cci_set_radiometry_tlinear_enable_state(CCI_RADIOMETRY_TLINEAR_DISABLED);
		}
		cci_set_agc_enable_state(CCI_AGC_ENABLED);
	} else {
		if (lep_is_radiometric) {
			cci_set_radiometry_tlinear_enable_state(CCI_RADIOMETRY_TLINEAR_ENABLED);
		}
		cci_set_agc_enable_state(CCI_AGC_DISABLED);
	}
}


void lepton_ffc()
{
	cci_run_ffc();
}


void lepton_gain_mode(uint8_t mode)
{
	cc_gain_mode_t gain_mode;
	
	if (lep_is_radiometric) {
		switch (mode) {
			case SYS_GAIN_HIGH:
				gain_mode = LEP_SYS_GAIN_MODE_HIGH;
				break;
			case SYS_GAIN_LOW:
				gain_mode = LEP_SYS_GAIN_MODE_LOW;
				break;
			default:
				gain_mode = LEP_SYS_GAIN_MODE_AUTO;
		}
		cci_set_gain_mode(gain_mode);
	}
}


void lepton_spotmeter(uint16_t r1, uint16_t c1, uint16_t r2, uint16_t c2)
{
	if (lep_is_radiometric) {
		cci_set_radiometry_spotmeter(r1, c1, r2, c2);
	}
}


void lepton_emissivity(uint16_t e)
{
	cci_rad_flux_linear_params_t set_flux_values;
	
	if (lep_is_radiometric) {
		// Scale percentage e into Lepton scene emissivity values (1-100% -> 82-8192)
		if (e < 1) e = 1;
		if (e > 100) e = 100;
		set_flux_values.sceneEmissivity = e * 8192 / 100;
		
		// Set default (no lens) values for the remaining parameters
		set_flux_values.TBkgK      = 29515;
		set_flux_values.tauWindow  = 8192;
		set_flux_values.TWindowK   = 29515;
		set_flux_values.tauAtm     = 8192;
		set_flux_values.TAtmK      = 29515;
		set_flux_values.reflWindow = 0;
		set_flux_values.TReflK     = 29515;
		
		cci_set_radiometry_flux_linear_params(&set_flux_values);
	}
}


uint32_t lepton_get_tel_status(uint16_t* tel_buf)
{
	return (tel_buf[LEP_TEL_STATUS_HIGH] << 16) | tel_buf[LEP_TEL_STATUS_LOW];
}


/**
 * Convert a temperature reading from the lepton (in units of K * 100) to C
 */
float lepton_kelvin_to_C(uint32_t k, float lep_res)
{
	return (((float) k) * lep_res) - 273.15;
}
