/*
 * Firmware Update Utilities
 *
 * Utility functions to implement a firmware update mechanism using two OTA slots
 * in flash memory.
 *
 * Copyright 2021 Dan Julio
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
#ifndef UPD_UTILITIES_H
#define UPD_UTILITIES_H

#include <stdbool.h>
#include <stdint.h>



//
// Update Utilities constants
//
#define UPD_MAX_VER_LEN 32


//
// Update Utilities API
//
bool upd_init(uint32_t len, char* version);
bool upd_complete();
void upd_early_terminate();
bool upd_process_bytes(uint32_t start, uint32_t len, uint8_t* buf);

#endif /* UPD_UTILITIES_H */
