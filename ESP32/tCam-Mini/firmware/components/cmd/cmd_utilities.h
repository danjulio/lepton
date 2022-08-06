/*
 * Command related utilities for use by either wifi_cmd_task or ser_cmd_task.
 *
 * Includes functions to decode and execute commands.
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
#ifndef CMD_UTILITIES_H
#define CMD_UTILITIES_H

#include <stdbool.h>
#include <stdint.h>



//
// CMD Utilities Constants
//

// Commands
#define CMD_GET_STATUS  0
#define CMD_GET_IMAGE   1
#define CMD_SET_TIME    2
#define CMD_GET_WIFI    3
#define CMD_SET_WIFI    4
#define CMD_GET_CONFIG  5
#define CMD_SET_CONFIG  6
#define CMD_SET_SPOT    7
#define CMD_STREAM_ON   8
#define CMD_STREAM_OFF  9
#define CMD_TAKE_PIC    10
#define CMD_RECORD_ON   11
#define CMD_RECORD_OFF  12
#define CMD_POWEROFF    13
#define CMD_RUN_FFC     14
#define CMD_GET_FS_LIST 15
#define CMD_GET_FS_FILE 16
#define CMD_DEL_FS_OBJ  17
#define CMD_GET_LEP_CCI 18
#define CMD_SET_LEP_CCI 19
#define CMD_FW_UPD_REQ  20
#define CMD_FW_UPD_SEG  21
#define CMD_DUMP_SCREEN 22
#define CMD_NUM         23

#define CMD_UNKNOWN     999

// Command strings
#define CMD_GET_STATUS_S "get_status"
#define CMD_GET_IMAGE_S  "get_image"
#define CMD_SET_TIME_S   "set_time"
#define CMD_GET_WIFI_S   "get_wifi"
#define CMD_SET_WIFI_S   "set_wifi"
#define CMD_GET_CONFIG_S "get_config"
#define CMD_SET_CONFIG_S "set_config"
#define CMD_SET_SPOT_S   "set_spotmeter"
#define CMD_STREAM_ON_S  "stream_on"
#define CMD_STREAM_OFF_S "stream_off"
#define CMD_TAKE_PIC_S   "take_picture"
#define CMD_RECORD_ON_S  "record_on"
#define CMD_RECORD_OFF_S "record_off"
#define CMD_POWEROFF_S   "poweroff"
#define CMD_RUN_FFC_S    "run_ffc"
#define CMD_GET_FS_LIST_S "get_filesystem_list"
#define CMD_GET_FS_FILE_S "get_file"
#define CMD_DEL_FS_OBJ_S  "delete_filesystem_obj"
#define CMD_GET_LEP_CCI_S "get_lep_cci"
#define CMD_SET_LEP_CCI_S "set_lep_cci"
#define CMD_FW_UPD_REQ_S  "fw_update_request"
#define CMD_FW_UPD_SEG_S  "fw_segment"
#define CMD_DUMP_SCREEN_S "dump_screen"


// Delimiters used to wrap json strings sent over the network
#define CMD_JSON_STRING_START 0x02
#define CMD_JSON_STRING_STOP  0x03


//
// CMD Utilities API
//
void init_command_processor();
void push_rx_data(char* data, int len);
bool process_rx_data();

#endif /* CMD_UTILITIES_H */