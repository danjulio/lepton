/*
 * Lepton CCI Module
 *
 * Contains the functions to configure the Lepton via I2C.
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
#include "cci.h"
#include "i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <stdbool.h>
#include <stdio.h>



//
// CCI Variables
//
static const char* TAG = "cci";
static bool cci_last_status_error;
static uint16_t cci_last_status;

// Mutex to protect control routines for multi-task access
static SemaphoreHandle_t cci_mutex;

// Statically allocated burst read/write I2C buffer sized for up to 512 16-bit words
// plus register starting address
static uint8_t burst_buf[1026];



//
// Forward declarations for primitive access methods
//
static int cci_write_register(uint16_t reg, uint16_t value);
static int cci_write_burst(uint16_t start, uint16_t word_len, uint16_t* buf);
static uint16_t cci_read_register(uint16_t reg);
static int cci_read_burst(uint16_t start, uint16_t word_len, uint16_t* buf);
static uint32_t cci_wait_busy_clear();
static void cci_wait_busy_clear_check(char* cmd);



//
// CCI API
//

/**
 * Initialise the CCI.
 */
bool cci_init()
{
	cci_mutex = xSemaphoreCreateMutex();
	
	return (cci_mutex != NULL);
}


/**
 * Write 0 (equivalent to run_cmd) to 512 16-bit words to the lepton and issue
 * the specified command.  Lengths > 16 words are written to the BLOCK data buffer.
 */
void cci_set_reg(uint16_t cmd, int len, uint16_t* buf)
{
	char cmd_buf[11];   // sized for 'cmd 0xNNNN<NULL>'
	int ret = 1;
	
	cci_last_status_error = false;
	
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	
	if ((len > 0) && (len <= 16)) {
		ret = cci_write_burst(CCI_REG_DATA_0, len, buf);
	} else if ((len > 16) && (len <= 512)) {
		ret = cci_write_burst(CCI_BLOCK_BUF_0, len, buf);
	} else if (len > 512) {
		ret = 0;
	}
	
	if (ret == 1) {
		if (len > 0) {
			sprintf(cmd_buf, "CMD 0x%4x", cmd);
			cci_write_register(CCI_REG_DATA_LENGTH, len);
		} else {
			sprintf(cmd_buf, "RUN 0x%4x", cmd);
		}
		
		cci_write_register(CCI_REG_COMMAND, cmd);
		cci_wait_busy_clear_check(cmd_buf);
	} else {
		cci_last_status = 0;
		cci_last_status_error = true;
	}
	xSemaphoreGive(cci_mutex);
}


/**
 * Read up to 512 16-bit words form the lepton with the specified command.  Lengths > 16
 * words are read from the BLOCK data buffer.
 */
void cci_get_reg(uint16_t cmd, int len, uint16_t* buf)
{
	char cmd_buf[11];   // sized for 'cmd 0xNNNN<NULL>'
	
	sprintf(cmd_buf, "CMD 0x%4x", cmd);
	
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_LENGTH, len);
	cci_write_register(CCI_REG_COMMAND, cmd);
	cci_wait_busy_clear_check(cmd_buf);
	if ((len > 0) && (len <= 16)) {
		(void) cci_read_burst(CCI_REG_DATA_0, len, buf);
		
	} else if ((len > 16) && (len <= 512)) {
		(void) cci_read_burst(CCI_BLOCK_BUF_0, len, buf);
	}
	xSemaphoreGive(cci_mutex);
}


/**
 * Return true if previous command succeeded as detected by cci_wait_busy_clear_check
 */
bool cci_command_success(uint16_t* status)
{
	*status = cci_last_status;
	return !cci_last_status_error;
}


/**
 * Ping the camera.
 *   Returns 0 for a successful ping
 *   Returns the absolute (postive) 8-bit non-zero LEP_RESULT for a failure
 *   Returns 0x100 (256) for a communications failure
 */
uint32_t cci_run_ping()
{
	uint32_t res;
	uint8_t lep_res;
	
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_SYS_RUN_PING);
	res = cci_wait_busy_clear();
	xSemaphoreGive(cci_mutex);
	
	lep_res = (res & 0x000FF00) >> 8;  // 8-bit Response Error Code: 0=LEP_OK
	if (res == 0x00010000) {
		return 0x100;
	} else if (lep_res == 0x00) {
		return 0;
	} else {
		// Convert negative Lepton Response Error Code to a positive number to return
		lep_res = ~lep_res + 1;
		return lep_res;
	}
}


/**
 * Request that a flat field correction occur immediately.
 */
void cci_run_ffc()
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_SYS_RUN_FFC);
	cci_wait_busy_clear_check("CCI_CMD_SYS_RUN_FFC");
	xSemaphoreGive(cci_mutex);
}


/**
 * Get the system uptime.
 */
uint32_t cci_get_uptime()
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_SYS_GET_UPTIME);
	cci_wait_busy_clear_check("CCI_CMD_SYS_GET_UPTIME");
	xSemaphoreGive(cci_mutex);
	
	uint16_t ls_word = cci_read_register(CCI_REG_DATA_0);
	uint16_t ms_word = cci_read_register(CCI_REG_DATA_1);
	return ms_word << 16 | ls_word;
}


/**
 * Get the AUX (case) temperature in Kelvin x 100 (16-bit result).
 */
uint32_t cci_get_aux_temp()
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_SYS_GET_AUX_TEMP);
	cci_wait_busy_clear_check("CCI_CMD_SYS_GET_AUX_TEMP");
	xSemaphoreGive(cci_mutex);
	
	uint16_t ls_word = cci_read_register(CCI_REG_DATA_0);
	uint16_t ms_word = cci_read_register(CCI_REG_DATA_1);
	return ms_word << 16 | ls_word;
}


/**
 * Get the FPA (sensor) temperature in Kelvin x 100 (16-bit result).
 */
uint32_t cci_get_fpa_temp()
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_SYS_GET_FPA_TEMP);
	cci_wait_busy_clear_check("CCI_CMD_SYS_GET_FPA_TEMP");
	xSemaphoreGive(cci_mutex);
	
	uint16_t ls_word = cci_read_register(CCI_REG_DATA_0);
	uint16_t ms_word = cci_read_register(CCI_REG_DATA_1);
	return ms_word << 16 | ls_word;
}


/**
 * Change the telemetry enable state.
 */
void cci_set_telemetry_enable_state(cci_telemetry_enable_state_t state)
{
	uint32_t value = state;
	
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_0, value & 0xffff);
	cci_write_register(CCI_REG_DATA_1, value >> 16 & 0xffff);
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_SYS_SET_TELEMETRY_ENABLE_STATE);
	cci_wait_busy_clear_check("CCI_CMD_SYS_SET_TELEMETRY_ENABLE_STATE");
	xSemaphoreGive(cci_mutex);
}


/**
 * Get the telemetry enable state.
 */
uint32_t cci_get_telemetry_enable_state()
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_SYS_GET_TELEMETRY_ENABLE_STATE);
	cci_wait_busy_clear_check("CCI_CMD_SYS_GET_TELEMETRY_ENABLE_STATE");
	xSemaphoreGive(cci_mutex);
	
	uint16_t ls_word = cci_read_register(CCI_REG_DATA_0);
	uint16_t ms_word = cci_read_register(CCI_REG_DATA_1);
	return ms_word << 16 | ls_word;
}


/**
 * Change the telemetry location.
 */
void cci_set_telemetry_location(cci_telemetry_location_t location)
{
	uint32_t value = location;
	
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_0, value & 0xffff);
	cci_write_register(CCI_REG_DATA_1, value >> 16 & 0xffff);
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_SYS_SET_TELEMETRY_LOCATION);
	cci_wait_busy_clear_check("CCI_CMD_SYS_SET_TELEMETRY_LOCATION");
	xSemaphoreGive(cci_mutex);
}


/**
 * Get the telemetry location.
 */
uint32_t cci_get_telemetry_location()
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_SYS_GET_TELEMETRY_LOCATION);
	cci_wait_busy_clear_check("CCI_CMD_SYS_GET_TELEMETRY_LOCATION");
	xSemaphoreGive(cci_mutex);
	
	uint16_t ls_word = cci_read_register(CCI_REG_DATA_0);
	uint16_t ms_word = cci_read_register(CCI_REG_DATA_1);
	return ms_word << 16 | ls_word;
}


/**
 * Get the Gain Mode
 */
void cci_set_gain_mode(cc_gain_mode_t mode)
{
	uint32_t value = mode;
	
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_0, value & 0xffff);
	cci_write_register(CCI_REG_DATA_1, value >> 16 & 0xffff);
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_SYS_SET_GAIN_MODE);
	cci_wait_busy_clear_check("CCI_CMD_SYS_SET_GAIN_MODE");
	xSemaphoreGive(cci_mutex);
}


/**
 * Set the gain mode
 */
uint32_t cci_get_gain_mode()
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_SYS_GET_GAIN_MODE);
	cci_wait_busy_clear_check("CCI_CMD_SYS_GET_GAIN_MODE");
	xSemaphoreGive(cci_mutex);
	
	uint16_t ls_word = cci_read_register(CCI_REG_DATA_0);
	uint16_t ms_word = cci_read_register(CCI_REG_DATA_1);
	return ms_word << 16 | ls_word;
}


/**
 * Change the radiometry enable state.
 */
void cci_set_radiometry_enable_state(cci_radiometry_enable_state_t state)
{
	uint32_t value = state;
	
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_0, value & 0xffff);
	cci_write_register(CCI_REG_DATA_1, value >> 16 & 0xffff);
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_RAD_SET_RADIOMETRY_ENABLE_STATE);
	cci_wait_busy_clear_check("CCI_CMD_RAD_SET_RADIOMETRY_ENABLE_STATE");
	xSemaphoreGive(cci_mutex);
}


/**
 * Get the radiometry enable state.
 */
uint32_t cci_get_radiometry_enable_state()
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_RAD_GET_RADIOMETRY_ENABLE_STATE);
	cci_wait_busy_clear_check("CCI_CMD_RAD_GET_RADIOMETRY_ENABLE_STATE");
	xSemaphoreGive(cci_mutex);
	
	uint16_t ls_word = cci_read_register(CCI_REG_DATA_0);
	uint16_t ms_word = cci_read_register(CCI_REG_DATA_1);
	return ms_word << 16 | ls_word;
}


/**
 * Set the radiometry flux parameters
 */
void cci_set_radiometry_flux_linear_params(cci_rad_flux_linear_params_t* params)
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_0, params->sceneEmissivity);
	cci_write_register(CCI_REG_DATA_1, params->TBkgK);
	cci_write_register(CCI_REG_DATA_2, params->tauWindow);
	cci_write_register(CCI_REG_DATA_3, params->TWindowK);
	cci_write_register(CCI_REG_DATA_4, params->tauAtm);
	cci_write_register(CCI_REG_DATA_5, params->TAtmK);
	cci_write_register(CCI_REG_DATA_6, params->reflWindow);
	cci_write_register(CCI_REG_DATA_7, params->TReflK);
	cci_write_register(CCI_REG_DATA_LENGTH, 8);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_RAD_SET_RADIOMETRY_FLUX_LINEAR_PARAMS);
	cci_wait_busy_clear_check("CCI_CMD_RAD_SET_RADIOMETRY_FLUX_LINEAR_PARAMS");
	xSemaphoreGive(cci_mutex);
}


/**
 * Get the radiometry flux parameters
 */
bool cci_get_radiometry_flux_linear_params(cci_rad_flux_linear_params_t* params)
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_LENGTH, 8);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_RAD_GET_RADIOMETRY_FLUX_LINEAR_PARAMS);
	cci_wait_busy_clear_check("CCI_CMD_RAD_GET_RADIOMETRY_FLUX_LINEAR_PARAMS");
	params->sceneEmissivity = cci_read_register(CCI_REG_DATA_0);
	params->TBkgK = cci_read_register(CCI_REG_DATA_1);
	params->tauWindow = cci_read_register(CCI_REG_DATA_2);
	params->TWindowK = cci_read_register(CCI_REG_DATA_3);
	params->tauAtm = cci_read_register(CCI_REG_DATA_4);
	params->TAtmK = cci_read_register(CCI_REG_DATA_5);
	params->reflWindow = cci_read_register(CCI_REG_DATA_6);
	params->TReflK = cci_read_register(CCI_REG_DATA_7);
	xSemaphoreGive(cci_mutex);
	
	return !cci_last_status_error;
}


/**
 * Change the radiometry TLinear enable state.
 */
void cci_set_radiometry_tlinear_enable_state(cci_radiometry_tlinear_enable_state_t state)
{
	uint32_t value = state;
	
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_0, value & 0xffff);
	cci_write_register(CCI_REG_DATA_1, value >> 16 & 0xffff);
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_RAD_SET_RADIOMETRY_TLINEAR_ENABLE_STATE);
	cci_wait_busy_clear_check("CCI_CMD_RAD_SET_RADIOMETRY_TLINEAR_ENABLE_STATE");
	xSemaphoreGive(cci_mutex);
}


/**
 * Get the radiometry TLinear enable state.
 */
uint32_t cci_get_radiometry_tlinear_enable_state()
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_RAD_GET_RADIOMETRY_TLINEAR_ENABLE_STATE);
	cci_wait_busy_clear_check("CCI_CMD_RAD_GET_RADIOMETRY_TLINEAR_ENABLE_STATE");
	xSemaphoreGive(cci_mutex);
	
	uint16_t ls_word = cci_read_register(CCI_REG_DATA_0);
	uint16_t ms_word = cci_read_register(CCI_REG_DATA_1);
	return ms_word << 16 | ls_word;
}


/**
 * Set the radiometry TLinear Auto Resolution
 */
void cci_set_radiometry_tlinear_auto_res(cci_radiometry_tlinear_auto_res_state_t state)
{
	uint32_t value = state;
	
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_0, value & 0xffff);
	cci_write_register(CCI_REG_DATA_1, value >> 16 & 0xffff);
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_RAD_SET_RADIOMETRY_TLINEAR_AUTO_RES);
	cci_wait_busy_clear_check("CCI_CMD_RAD_SET_RADIOMETRY_TLINEAR_AUTO_RES");
	xSemaphoreGive(cci_mutex);
}


/**
 * Get the radiometry TLinear Auto Resolution
 */
uint32_t cci_get_radiometry_tlinear_auto_res()
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_RAD_GET_RADIOMETRY_TLINEAR_AUTO_RES);
	cci_wait_busy_clear_check("CCI_CMD_RAD_GET_RADIOMETRY_TLINEAR_AUTO_RES");
	xSemaphoreGive(cci_mutex);
	
	uint16_t ls_word = cci_read_register(CCI_REG_DATA_0);
	uint16_t ms_word = cci_read_register(CCI_REG_DATA_1);
	return ms_word << 16 | ls_word;
}


/**
 * Set the Radiometry Spotmeter Region-of-interest
 */
void cci_set_radiometry_spotmeter(uint16_t r1, uint16_t c1, uint16_t r2, uint16_t c2)
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_0, r1);
	cci_write_register(CCI_REG_DATA_1, c1);
	cci_write_register(CCI_REG_DATA_2, r2);
	cci_write_register(CCI_REG_DATA_3, c2);
	cci_write_register(CCI_REG_DATA_LENGTH, 4);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_RAD_SET_RADIOMETRY_SPOT_ROI);
	cci_wait_busy_clear_check("CCI_CMD_RAD_SET_RADIOMETRY_SPOT_ROI");
	xSemaphoreGive(cci_mutex);
}


/**
 * Get the Radiometry Spotmeter Region-of-interest
 */
bool cci_get_radiometry_spotmeter(uint16_t* r1, uint16_t* c1, uint16_t* r2, uint16_t* c2)
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_LENGTH, 4);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_RAD_GET_RADIOMETRY_SPOT_ROI);
	cci_wait_busy_clear_check("CCI_CMD_RAD_GET_RADIOMETRY_SPOT_ROI");
	*r1 = cci_read_register(CCI_REG_DATA_0);
	*c1 = cci_read_register(CCI_REG_DATA_1);
	*r2 = cci_read_register(CCI_REG_DATA_2);
	*c2 = cci_read_register(CCI_REG_DATA_3);
	xSemaphoreGive(cci_mutex);
	
	return !cci_last_status_error;
}


/**
 * Get the AGC enable state.
 */
uint32_t cci_get_agc_enable_state()
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_AGC_GET_AGC_ENABLE_STATE);
	cci_wait_busy_clear_check("CCI_CMD_AGC_GET_AGC_ENABLE_STATE");
	xSemaphoreGive(cci_mutex);
	
	uint16_t ls_word = cci_read_register(CCI_REG_DATA_0);
	uint16_t ms_word = cci_read_register(CCI_REG_DATA_1);
	return ms_word << 16 | ls_word;
}


/**
 * Set the AGC enable state.
 */
void cci_set_agc_enable_state(cci_agc_enable_state_t state)
{
	uint32_t value = state;
	
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_0, value & 0xffff);
	cci_write_register(CCI_REG_DATA_1, value >> 16 & 0xffff);
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_AGC_SET_AGC_ENABLE_STATE);
	cci_wait_busy_clear_check("CCI_CMD_AGC_SET_AGC_ENABLE_STATE");
	xSemaphoreGive(cci_mutex);
}


/**
 * Get the AGC calc enable state.
 */
uint32_t cci_get_agc_calc_enable_state()
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_AGC_GET_CALC_ENABLE_STATE);
	cci_wait_busy_clear_check("CCI_CMD_AGC_GET_CALC_ENABLE_STATE");
	xSemaphoreGive(cci_mutex);
	
	uint16_t ls_word = cci_read_register(CCI_REG_DATA_0);
	uint16_t ms_word = cci_read_register(CCI_REG_DATA_1);
	return ms_word << 16 | ls_word;
}


/**
 * Set the AGC calc enable state.
 */
void cci_set_agc_calc_enable_state(cci_agc_enable_state_t state)
{
	uint32_t value = state;
	
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_0, value & 0xffff);
	cci_write_register(CCI_REG_DATA_1, value >> 16 & 0xffff);
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_AGC_SET_CALC_ENABLE_STATE);
	cci_wait_busy_clear_check("CCI_CMD_AGC_SET_CALC_ENABLE_STATE");
	xSemaphoreGive(cci_mutex);
}

/**
 * Run the Reboot command
 */
void cc_run_oem_reboot()
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_OEM_RUN_REBOOT);
	// Sleep to allow camera to reboot and run FFC
	vTaskDelay(pdMS_TO_TICKS(6000));
	cci_wait_busy_clear_check("CCI_CMD_OEM_RUN_REBOOT");
	xSemaphoreGive(cci_mutex);
}


/**
 * Get the GPIO mode.
 */
uint32_t cci_get_gpio_mode()
{
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
 	cci_write_register(CCI_REG_COMMAND, CCI_CMD_OEM_GET_GPIO_MODE);
	cci_wait_busy_clear_check("CCI_CMD_OEM_GET_GPIO_MODE");
	xSemaphoreGive(cci_mutex);
	
	uint16_t ls_word = cci_read_register(CCI_REG_DATA_0);
	uint16_t ms_word = cci_read_register(CCI_REG_DATA_1);
	return ms_word << 16 | ls_word;
}


/**
 * Set the GPIO mode.
 */
void cci_set_gpio_mode(cci_gpio_mode_t mode)
{
	uint32_t value = mode;
	
	xSemaphoreTake(cci_mutex, portMAX_DELAY);
	cci_wait_busy_clear();
	cci_write_register(CCI_REG_DATA_0, value & 0xffff);
	cci_write_register(CCI_REG_DATA_1, value >> 16 & 0xffff);
	cci_write_register(CCI_REG_DATA_LENGTH, 2);
	cci_write_register(CCI_REG_COMMAND, CCI_CMD_OEM_SET_GPIO_MODE);
	cci_wait_busy_clear_check("CCI_CMD_OEM_SET_GPIO_MODE");
	xSemaphoreGive(cci_mutex);
}


/**
 * Get the FLIR systems part number
 *   - call with a 32-character buffer
 */
void cci_get_part_number(char* pn)
{
	bool low_half = true;
	int i = 0;
	uint16_t cci_buf[16];
	
	cci_get_reg(CCI_CMD_OEM_GET_PART_NUM, 16, cci_buf);
	
	*pn = (char) (cci_buf[0] & 0xFF);
	while ((*pn != 0) && (i<16)) {
		low_half = !low_half;
		if (low_half) {
			*(++pn) = (char) (cci_buf[i] & 0xFF);
		} else {
			*(++pn) = (char) (cci_buf[i] >> 8);
			i++;
		}
	}
	*(++pn) = 0;
}



//
// Primitive access methods
//

/**
 * Write a CCI register.
 */
static int cci_write_register(uint16_t reg, uint16_t value)
{
	// Write the register address and value
	uint8_t write_buf[4] = {
		reg >> 8 & 0xff,
		reg & 0xff,
		value >> 8 & 0xff,
		value & 0xff
	};

	i2c_lock();
	if (i2c_master_write_slave(CCI_ADDRESS, write_buf, sizeof(write_buf)) != ESP_OK) {
		i2c_unlock();
		ESP_LOGE(TAG, "failed to write CCI register %02x with value %02x", reg, value);
		return -1;
	};
	i2c_unlock();

	return 1;
}


/**
 * Burst write a group of CCI data registers
 */
static int cci_write_burst(uint16_t start, uint16_t word_len, uint16_t* buf)
{
	int i;
	
	// Create the i2c transaction buffer 
	burst_buf[0] = start >> 8;
	burst_buf[1] = start & 0xFF;
	for (i=1; i<=word_len; i++) {
		burst_buf[i*2] = *buf >> 8;
		burst_buf[i*2 + 1] = *buf++ & 0xFF;
	}
	
	// Execute the burst
	i2c_lock();
	if (i2c_master_write_slave(CCI_ADDRESS, burst_buf, word_len*2 + 2) != ESP_OK) {
		i2c_unlock();
		ESP_LOGE(TAG, "failed to burst write CCI register %02x with length %d", start, word_len);
		return -1;
	};
	i2c_unlock();
	
	return 1;
}


/**
 * Read a CCI register.
 */
static uint16_t cci_read_register(uint16_t reg)
{
	uint8_t buf[2];

	// Write the register address
	buf[0] = reg >> 8;
	buf[1] = reg & 0xff;
  
	i2c_lock();
	if (i2c_master_write_slave(CCI_ADDRESS, buf, sizeof(buf)) != ESP_OK) {
		i2c_unlock();
		ESP_LOGE(TAG, "failed to write CCI register %02x", reg);
		return -1;
	}

	// Read
	if (i2c_master_read_slave(CCI_ADDRESS, buf, sizeof(buf)) != ESP_OK) {
		ESP_LOGE(TAG, "failed to read from CCI register %02x", reg);
	}
	i2c_unlock();

	return buf[0] << 8 | buf[1];
}


/**
 * Burst read a group of CCI data registers
 */
static int cci_read_burst(uint16_t start, uint16_t word_len, uint16_t* buf)
{
	int i;
	
	// Write the starting address
	burst_buf[0] = start >> 8;
	burst_buf[1] = start & 0xFF;
	i2c_lock();
	if (i2c_master_write_slave(CCI_ADDRESS, burst_buf, 2) != ESP_OK) {
		i2c_unlock();
		ESP_LOGE(TAG, "failed to write CCI register %02x", start);
		return -1;
	}
	
	// Read
	if (i2c_master_read_slave(CCI_ADDRESS, burst_buf, word_len*2) != ESP_OK) {
		ESP_LOGE(TAG, "failed to burst read from CCI register %02x with length %d", start, word_len);
		return -1;
	}
	i2c_unlock();
	
	// Copy data out
	for (i=0; i<word_len; i++) {
		*buf++ = burst_buf[i*2] << 8 | burst_buf[i*2 + 1];
	}
	
	return 1;
}


/**
 * Wait for busy to be clear in the status register
 *   Returns the 16-bit STATUS
 *   Returns 0x00010000 if there is a communication failure
 */
static uint32_t cci_wait_busy_clear()
{
	bool err = false;
	uint8_t buf[2] = {0x00, 0x07};

	// Wait for booted, not busy
	while (((buf[1] & 0x07) != 0x06) && !err) {
		// Write STATUS register address
		buf[0] = 0x00;
		buf[1] = 0x02;
		
		i2c_lock();
		if (i2c_master_write_slave(CCI_ADDRESS, buf, sizeof(buf)) != ESP_OK) {
			ESP_LOGE(TAG, "failed to set STATUS register");
			err = true;
		};

		// Read register - low bits in buf[1]
		if (i2c_master_read_slave(CCI_ADDRESS, buf, sizeof(buf)) != ESP_OK) {
			ESP_LOGE(TAG, "failed to read STATUS register");
			err = true;
		}
		i2c_unlock();
	}
	
	if (err) {
		return 0x00010000;
	} else {
		return (buf[0] << 8) | buf[1];
	}
}


/**
 * Wait for busy to be clear in the status register and check the result
 * printing an error if detected
 */
static void cci_wait_busy_clear_check(char* cmd)
{
	int8_t   response;
	uint32_t t32;
	
	cci_last_status_error = false;
	
	t32 = cci_wait_busy_clear();
	cci_last_status = t32 & 0xFFFF;
	if (t32 == 0x00010000) {
		ESP_LOGE(TAG, "cmd: %s failed wait_busy_clear", cmd);
		cci_last_status_error = true;
	} else {
		response = (int8_t) ((t32 & 0x0000FF00) >> 8);
		if (response < 0) {
			ESP_LOGE(TAG, "%s returned %d", cmd, response);
		}
	}
}
