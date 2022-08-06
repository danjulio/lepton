/*
 * Serial interface related utilities.
 *
 * Provides access to the serial interface for serial mode communications.  Designed
 * to be polled.
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
#ifndef SIF_UTILITIES_H
#define SIF_UTILITIES_H



//
// SIF API
//
void sif_init();
void sif_send(const char* s, int len);
int sif_get(char* s, int max);


#endif /* SIF_UTILITIES_H */