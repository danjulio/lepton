/*
 * Time related utilities
 *
 * Contains functions to interface the RTC to the system timekeeping
 * capabilities and provide application access to the system time.
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
 * along with tCam.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef TIME_UTILITIES_H
#define TIME_UTILITIES_H

#include <stdint.h>
#include "ds3232.h"


//
// Time Utilities API
//
void time_init();
void time_set(tmElements_t te);
void time_get(tmElements_t* te);
bool time_changed(tmElements_t* te, time_t* prev_time);
void time_get_disp_string(tmElements_t te, char* buf);
void time_get_short_string(tmElements_t te, char* buf);
void time_get_hhmm_string(tmElements_t te, char* buf);

#endif /* TIME_UTILITIES_H */