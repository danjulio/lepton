/*
 * Lepton VoSPI Module
 *
 * Contains the functions to get frames from a Lepton 3.5 via its SPI port.
 * Optionally supports collecting telemetry when enabled as a footer (does not
 * support telemetry enabled as a header). 
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
#ifndef VOSPI_H
#define VOSPI_H

#include <stdbool.h>
#include <stdint.h>
#include "sys_utilities.h"


//
// VoSPI Constants
//

// LEP_FRAME_USEC is the per-frame period from the Lepton (interrupt rate) 
#define LEP_FRAME_USEC 9450
// LEP_MAX_FRAME_XFER_WAIT_USEC specifies the maximum time we should wait in
// vospi_transfer_segment() to read a valid frame.  It should be LEP_FRAME_USEC -
// (maximum ISR latency + transfer_packet() code path overhead)
// than LEP_FRAME_USEC -  maximum ISR latency)
#define LEP_MAX_FRAME_XFER_WAIT_USEC 9250

#define LEP_WIDTH      160
#define LEP_HEIGHT     120
#define LEP_NUM_PIXELS (LEP_WIDTH * LEP_HEIGHT)
#define LEP_PKT_LENGTH 164

// Telemetry related
#define LEP_TEL_PACKETS 3
#define LEP_TEL_PKT_LEN (LEP_PKT_LENGTH - 4)
#define LEP_TEL_WORDS   (LEP_TEL_PACKETS * LEP_TEL_PKT_LEN / 2)

// Dynamic values depending if telemetry is included or not
#define LEP_TEL_PKTS_PER_SEG     61
#define LEP_NOTEL_PKTS_PER_SEG   60
#define LEP_TEL_WORDS_PER_SEG    (LEP_TEL_PKTS_PER_SEG * LEP_WIDTH / 2)
#define LEP_NOTEL_WORDS_PER_SEG  (LEP_NOTEL_PKTS_PER_SEG * LEP_WIDTH / 2)

/* Lepton frame error return */
enum LeptonReadError {
  NONE, DISCARD, SEGMENT_ERROR, ROW_ERROR, SEGMENT_INVALID
};



//
// VoSPI API
//
int vospi_init(int csn_pin);
bool vospi_transfer_segment(uint64_t vsyncDetectedUsec);
void vospi_get_frame(lep_buffer_t* sys_bufP);
void vospi_include_telem(bool en);

#endif /* VOSPI_H */
