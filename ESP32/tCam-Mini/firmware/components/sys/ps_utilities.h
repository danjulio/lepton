/*
 * Persistent Storage Module
 *
 * Manage the persistent storage kept in the ESP32 NVS and provide access
 * routines to it.
 *
 * NOTE: It is assumed that only task will access persistent storage at a time.
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
#ifndef PS_UTILITIES_H
#define PS_UTILITIES_H

#include "sys_utilities.h"
#include "wifi_utilities.h"
#include <stdbool.h>
#include <stdint.h>



//
// PS Utilities Constants
//

// Base part of the default SSID/Camera name - the last 4 nibbles of the ESP32's
// mac address are appended as ASCII characters
#define PS_DEFAULT_AP_SSID "tCam-Mini-"

// Field lengths
#define PS_SSID_MAX_LEN     32
#define PS_PW_MAX_LEN       32



//
// PS Utilities API
//
bool ps_init();
void ps_get_lep_state(json_config_t* state);
void ps_set_lep_state(const json_config_t* state);
void ps_get_wifi_info(wifi_info_t* info);
void ps_set_wifi_info(const wifi_info_t* info);
bool ps_reinit_wifi();

#endif /* PS_UTILITIES_H */