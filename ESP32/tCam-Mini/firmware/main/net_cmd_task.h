/*
 * Network Command Task
 *
 * Implement the command processing module for use when either the
 * Ethernet or WiFi interfaces are active.  Enable mDNS for device
 * discovery.
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
#ifndef NET_CMD_TASK_H
#define NET_CMD_TASK_H

#include <stdbool.h>
#include <stdint.h>



//
// Network CMD Task API
//
void net_cmd_task();
bool net_cmd_connected();
int net_cmd_get_socket();

#endif /* NET_CMD_TASK_H */