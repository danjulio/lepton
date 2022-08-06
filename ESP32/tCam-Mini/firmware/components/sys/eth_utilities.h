/*
 * Ethernet related utilities
 *
 * Contains functions to initialize and query the ethernet interface.
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
#ifndef ETH_UTILITIES_H
#define ETH_UTILITIES_H

#include <stdbool.h>
#include <stdint.h>
#include "net_utilities.h"

//
// Ethernet Utilities Constants
//



//
// Ethernet Utilities API
//
bool eth_init();
bool eth_reinit();
bool eth_is_connected();
net_info_t* eth_get_info();

#endif /* ETH_UTILITIES_H */
