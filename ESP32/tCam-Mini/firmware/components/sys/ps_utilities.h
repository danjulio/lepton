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
#ifndef PS_UTILITIES_H
#define PS_UTILITIES_H

#include "net_utilities.h"
#include "sys_utilities.h"
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
#define PS_PW_MAX_LEN       63
#define PS_OLD_PW_MAX_LEN   32



//
// PS Utilities API
//
bool ps_init(int brd, int iface);
void ps_get_lep_state(json_config_t* state);
void ps_set_lep_state(const json_config_t* state);
void ps_get_net_info(net_info_t* info);
void ps_set_net_info(const net_info_t* info);
bool ps_reinit_net();
bool ps_has_new_cam_name(const net_info_t* info);
char ps_nibble_to_ascii(uint8_t n);

#endif /* PS_UTILITIES_H */