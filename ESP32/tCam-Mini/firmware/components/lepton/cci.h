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
#ifndef CCI_H
#define CCI_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_system.h"


//
// CCI constants
//

// Device characteristics
#define CCI_WORD_LENGTH 0x02
#define CCI_ADDRESS 0x2A

// CCI register locations
#define CCI_REG_STATUS 0x0002
#define CCI_REG_COMMAND 0x0004
#define CCI_REG_DATA_LENGTH 0x0006
#define CCI_REG_DATA_0 0x0008
#define CCI_REG_DATA_1 0x000A
#define CCI_REG_DATA_2 0x000C
#define CCI_REG_DATA_3 0x000E
#define CCI_REG_DATA_4 0x0010
#define CCI_REG_DATA_5 0x0012
#define CCI_REG_DATA_6 0x0014
#define CCI_REG_DATA_7 0x0016
#define CCI_REG_DATA_8 0x0018
#define CCI_REG_DATA_9 0x001A
#define CCI_REG_DATA_10 0x001C
#define CCI_REG_DATA_11 0x001E
#define CCI_REG_DATA_12 0x0020
#define CCI_REG_DATA_13 0x0022
#define CCI_REG_DATA_14 0x0024
#define CCI_REG_DATA_15 0x0026
#define CCI_BLOCK_BUF_0 0xF800
#define CCI_BLOCK_BUF_1 0xFC00

// Commands
#define CCI_CMD_SYS_RUN_PING 0x0202
#define CCI_CMD_SYS_GET_UPTIME 0x020C
#define CCI_CMD_SYS_GET_AUX_TEMP 0x0210
#define CCI_CMD_SYS_GET_FPA_TEMP 0x0214
#define CCI_CMD_SYS_GET_TELEMETRY_ENABLE_STATE 0x0218
#define CCI_CMD_SYS_SET_TELEMETRY_ENABLE_STATE 0x0219
#define CCI_CMD_SYS_GET_TELEMETRY_LOCATION 0x021C
#define CCI_CMD_SYS_SET_TELEMETRY_LOCATION 0x021D
#define CCI_CMD_SYS_RUN_FFC 0x0242
#define CCI_CMD_SYS_GET_GAIN_MODE 0x0248
#define CCI_CMD_SYS_SET_GAIN_MODE 0x0249

#define CCI_CMD_RAD_GET_RADIOMETRY_ENABLE_STATE 0x4E10
#define CCI_CMD_RAD_SET_RADIOMETRY_ENABLE_STATE 0x4E11
#define CCI_CMD_RAD_GET_RADIOMETRY_FLUX_LINEAR_PARAMS 0x4EBC
#define CCI_CMD_RAD_SET_RADIOMETRY_FLUX_LINEAR_PARAMS 0x4EBD
#define CCI_CMD_RAD_GET_RADIOMETRY_TLINEAR_ENABLE_STATE 0x4EC0
#define CCI_CMD_RAD_SET_RADIOMETRY_TLINEAR_ENABLE_STATE 0x4EC1
#define CCI_CMD_RAD_GET_RADIOMETRY_TLINEAR_AUTO_RES 0x4EC8
#define CCI_CMD_RAD_SET_RADIOMETRY_TLINEAR_AUTO_RES 0x4EC9
#define CCI_CMD_RAD_GET_RADIOMETRY_SPOT_ROI 0x4ECC
#define CCI_CMD_RAD_SET_RADIOMETRY_SPOT_ROI 0x4ECD

#define CCI_CMD_AGC_GET_AGC_ENABLE_STATE 0x0100
#define CCI_CMD_AGC_SET_AGC_ENABLE_STATE 0x0101
#define CCI_CMD_AGC_GET_CALC_ENABLE_STATE 0x0148
#define CCI_CMD_AGC_SET_CALC_ENABLE_STATE 0x0149

#define CCI_CMD_OEM_RUN_REBOOT 0x4842

#define CCI_CMD_OEM_GET_GPIO_MODE 0x4854
#define CCI_CMD_OEM_SET_GPIO_MODE 0x4855

#define CCI_CMD_OEM_GET_PART_NUM 0x481C


//
// Macros
//
#define WAIT_FOR_BUSY_DEASSERT() cci_wait_busy_clear()


//
// Enums
//

// Telemetry Modes for use with CCI_CMD_SYS_SET_TELEMETRY_*
typedef enum {
	CCI_TELEMETRY_DISABLED,
	CCI_TELEMETRY_ENABLED
} cci_telemetry_enable_state_t;

typedef enum {
	CCI_TELEMETRY_LOCATION_HEADER,
	CCI_TELEMETRY_LOCATION_FOOTER
} cci_telemetry_location_t;

// Gain Modes for use with CCI_CMD_SYS_SET_GAIN_MODE */
typedef enum {
	LEP_SYS_GAIN_MODE_HIGH,
	LEP_SYS_GAIN_MODE_LOW,
	LEP_SYS_GAIN_MODE_AUTO
	} cc_gain_mode_t;

// Radiometry Modes for use with CCI_CMD_RAD_SET_RADIOMETRY*
typedef enum {
	CCI_RADIOMETRY_DISABLED,
	CCI_RADIOMETRY_ENABLED
} cci_radiometry_enable_state_t;

typedef enum {
	CCI_RADIOMETRY_TLINEAR_DISABLED,
	CCI_RADIOMETRY_TLINEAR_ENABLED
} cci_radiometry_tlinear_enable_state_t;

typedef enum {
	CCI_RADIOMETRY_AUTO_RES_DISABLED,
	CCI_RADIOMETRY_AUTO_RES_ENABLED
} cci_radiometry_tlinear_auto_res_state_t;

// AGC Modes for use with CCI_CMD_AGC_SET_AGC*
typedef enum {
	CCI_AGC_DISABLED,
	CCI_AGC_ENABLED
} cci_agc_enable_state_t;

// GPIO Modes for use with CCI_CMD_OEM_SET_GPIO_MODE*
typedef enum {
	LEP_OEM_GPIO_MODE_GPIO = 0,
	LEP_OEM_GPIO_MODE_I2C_MASTER = 1,
	LEP_OEM_GPIO_MODE_SPI_MASTER_VLB_DATA = 2,
	LEP_OEM_GPIO_MODE_SPIO_MASTER_REG_DATA = 3,
	LEP_OEM_GPIO_MODE_SPI_SLAVE_VLB_DATA = 4,
	LEP_OEM_GPIO_MODE_VSYNC = 5
} cci_gpio_mode_t;

// Radiometry Flux Linear Params
typedef struct {
	uint16_t sceneEmissivity;
	uint16_t TBkgK;
	uint16_t tauWindow;
	uint16_t TWindowK;
	uint16_t tauAtm;
	uint16_t TAtmK;
	uint16_t reflWindow;
	uint16_t TReflK;
} cci_rad_flux_linear_params_t;



//
// CCI API
//

// Setup
bool cci_init();

// Generic access
void cci_set_reg(uint16_t cmd, int len, uint16_t* buf);
void cci_get_reg(uint16_t cmd, int len, uint16_t* buf);
bool cci_command_success(uint16_t* status);

// Module: SYS
uint32_t cci_run_ping();
void cci_run_ffc();
uint32_t cci_get_uptime();
uint32_t cci_get_aux_temp();
uint32_t cci_get_fpa_temp();
void cci_set_telemetry_enable_state(cci_telemetry_enable_state_t state);
uint32_t cci_get_telemetry_enable_state();
void cci_set_telemetry_location(cci_telemetry_location_t location);
uint32_t cci_get_telemetry_location();
void cci_set_gain_mode(cc_gain_mode_t mode);
uint32_t cci_get_gain_mode();

// Module: RAD
void cci_set_radiometry_enable_state(cci_radiometry_enable_state_t state);
uint32_t cci_get_radiometry_enable_state();
void cci_set_radiometry_flux_linear_params(cci_rad_flux_linear_params_t* params);
bool cci_get_radiometry_flux_linear_params(cci_rad_flux_linear_params_t* params);
void cci_set_radiometry_tlinear_enable_state(cci_radiometry_tlinear_enable_state_t state);
uint32_t cci_get_radiometry_tlinear_enable_state();
void cci_set_radiometry_tlinear_auto_res(cci_radiometry_tlinear_auto_res_state_t state);
uint32_t cci_get_radiometry_tlinear_auto_res();
void cci_set_radiometry_spotmeter(uint16_t r1, uint16_t c1, uint16_t r2, uint16_t c2);
bool cci_get_radiometry_spotmeter(uint16_t* r1, uint16_t* c1, uint16_t* r2, uint16_t* c2);

// Module: AGC
void cci_set_agc_enable_state(cci_agc_enable_state_t state);
uint32_t cci_get_agc_enable_state();
void cci_set_agc_calc_enable_state(cci_agc_enable_state_t state);
uint32_t cci_get_agc_calc_enable_state();

// Module: OEM
void cc_run_oem_reboot();
uint32_t cci_get_gpio_mode();
void cci_set_gpio_mode(cci_gpio_mode_t mode);
void cci_get_part_number(char* pn);

#endif /* CCI_H */
