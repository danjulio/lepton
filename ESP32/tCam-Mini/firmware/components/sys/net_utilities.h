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
#ifndef NET_UTILITIES_H
#define NET_UTILITIES_H

#include <stdbool.h>
#include <stdint.h>

//
// Network Utilities Constants
//

// net_info_t flags
//   Used for both interfaces although only a subset of flags are used
//   when the ethernet interface is active.  Ethernet can only be a client.
#define NET_INFO_FLAG_STARTUP_ENABLE 0x01
#define NET_INFO_FLAG_INITIALIZED    0x02
#define NET_INFO_FLAG_ENABLED        0x04
#define NET_INFO_FLAG_CONNECTED      0x08
#define NET_INFO_FLAG_CL_STATIC_IP   0x10
#define NET_INFO_FLAG_CLIENT_MODE    0x80



//
// Network Utilities Data structures
//   Used for both interfaces although only ap_ssid (camera name), flags,
//   sta_ip_addr/netmask and cur_ip_addr are used when the ethernet interface is active
//
typedef struct {
	char* ap_ssid;             // AP SSID is also the Camera Name
	char* sta_ssid;
	char* ap_pw;
	char* sta_pw;
	uint8_t flags;
	uint8_t ap_ip_addr[4];
	uint8_t sta_ip_addr[4];
	uint8_t sta_netmask[4];
	uint8_t cur_ip_addr[4];
} net_info_t;



//
// Network Utilities Function pointers
//
extern bool (*net_init)();
extern bool (*net_reinit)();
extern bool (*net_is_connected)();
extern net_info_t* (*net_get_info)();



//
// Network Utilities API
//
void net_init_if(int if_type);

#endif /* NET_UTILITIES_H */