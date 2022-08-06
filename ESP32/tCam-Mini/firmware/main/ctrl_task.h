/*
 * Control Interface Task
 *
 * Manage user controls:
 *   Ethernet/WiFi Reset Button
 *   Red/Green Dual Status LED
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
#ifndef CTRL_TASK_H
#define CTRL_TASK_H

#include <stdbool.h>


//
// Control Task Constants
//

// Control Task evaluation interval
#define CTRL_EVAL_MSEC             50

// Timeouts (multiples of CTRL_EVAL_MSEC)
#define CTRL_BTN_PRESS_MSEC        5000
#define CTRL_RESET_ALERT_MSEC      3000
#define CTRL_FAST_BLINK_MSEC       100
#define CTRL_SLOW_BLINK_MSEC       1000
#define CTRL_FAULT_BLINK_ON_MSEC   200
#define CTRL_FAULT_BLINK_OFF_MSEC  300
#define CTRL_FAULT_IDLE_MSEC       2000
#define CTRL_FW_UPD_REQ_BLINK_MSEC 250

// Fault Types - sets blink count too
#define CTRL_FAULT_NONE           0
#define CTRL_FAULT_ESP32_INIT     1
#define CTRL_FAULT_PERIPH_INIT    2
#define CTRL_FAULT_MEM_INIT       3
#define CTRL_FAULT_LEP_CCI        4
#define CTRL_FAULT_LEP_VOSPI      5
#define CTRL_FAULT_NETWORK        6
#define CTRL_FAULT_LEP_SYNC       7
#define CTRL_FAULT_FW_UPDATE      8

// Board type
#define CTRL_BRD_ETH_TYPE         0
#define CTRL_BRD_WIFI_TYPE        1

// Interface mode
#define CTRL_IF_MODE_ETH          0
#define CTRL_IF_MODE_SIF          1
#define CTRL_IF_MODE_WIFI         2

// Control Task notifications
#define CTRL_NOTIFY_STARTUP_DONE      0x00000001
#define CTRL_NOTIFY_FAULT             0x00000002
#define CTRL_NOTIFY_FAULT_CLEAR       0x00000004
#define CTRL_NOTIFY_FW_UPD_REQ        0x00000010
#define CTRL_NOTIFY_FW_UPD_PROCESS    0x00000020
#define CTRL_NOTIFY_FW_UPD_DONE       0x00000040
#define CTRL_NOTIFY_FW_UPD_REBOOT     0x00000080


//
// Control Task API
//
void ctrl_task();
void ctrl_get_if_mode(int* brd, int* iface);
void ctrl_set_fault_type(int f);

#endif /* CTRL_TASK_H */