/*
 * Network related utilities
 *
 * Contains definitions and a set of function pointers to the selected
 * networking hardware functions (ethernet or WiFi) so that other can 
 * operate without having to know which hardware is in use.
 *
 * Copyright 2022 Dan Julio
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
#include "eth_utilities.h"
#include "net_utilities.h"
#include "wifi_utilities.h"
#include "ctrl_task.h"



//
// Network Utilities Function pointers
//
bool (*net_init)();
bool (*net_reinit)();
bool (*net_is_connected)();
net_info_t* (*net_get_info)();



//
// Network Utilities API
//
void net_init_if(int if_type)
{
	if (if_type == CTRL_IF_MODE_ETH) {
		net_init = &eth_init;
		net_reinit = &eth_reinit;
		net_is_connected = &eth_is_connected;
		net_get_info = &eth_get_info;
	} else {
		net_init = &wifi_init;
		net_reinit = &wifi_reinit;
		net_is_connected = &wifi_is_connected;
		net_get_info = &wifi_get_info;
	}
}
