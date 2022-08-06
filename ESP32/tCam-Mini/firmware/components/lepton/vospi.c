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
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "system_config.h"
#include "vospi.h"



//
// VoSPI Variables
//

// Logging support
static const char* TAG = "vospi";

// SPI Interface
static spi_device_handle_t spi;
static spi_transaction_t lep_spi_trans;

// Pointer to allocated array to store one Lepton packet (DMA capable)
static uint8_t* lepPacketP;

// Lepton Frame buffer (16-bit values)
static uint16_t lepBuffer[LEP_NUM_PIXELS];

// Lepton Telemetry buffer (16-bit values)
static uint16_t lepTelem[LEP_TEL_WORDS];

// Processing State
static int curSegment = 1;
static int curLinesPerSeg = LEP_NOTEL_PKTS_PER_SEG;
static int curWordsPerSeg = LEP_NOTEL_WORDS_PER_SEG;
static bool validSegmentRegion = false;
static bool includeTelemetry = false;




//
// VoSPI Forward Declarations for internal functions
//
static bool transfer_packet(uint8_t* line, uint8_t* seg);
static void copy_packet_to_lepton_buffer(uint8_t line);
static void copy_packet_to_telem_buffer(uint8_t line);



//
// VoSPI API
//

/**
 * Initialise the VoSPI interface.
 */
int vospi_init(int csn_pin)
{
	esp_err_t ret;
  
	spi_device_interface_config_t devcfg = {
		.command_bits = 0,
		.address_bits = 0,
		.clock_speed_hz = LEP_SPI_FREQ_HZ,
		.mode = 3,
		.spics_io_num = csn_pin,
		.queue_size = 1,
		.flags = SPI_DEVICE_HALFDUPLEX,
		.cs_ena_pretrans = 10
	};

	if ((ret=spi_bus_add_device(LEP_SPI_HOST, &devcfg, &spi)) != ESP_OK) {
		ESP_LOGE(TAG, "failed to add lepton spi device");
	} else {
		// Allocate DMA capable memory for the lepton packet
		lepPacketP = (uint8_t*) heap_caps_malloc(LEP_PKT_LENGTH, MALLOC_CAP_DMA);
		if (lepPacketP != NULL) {
			ret = ESP_OK;
		} else {
			ESP_LOGE(TAG, "failed to allocate lepton DMA packet buffer");
			ret = ESP_FAIL;
		}
	}
	
	// Setup our SPI transaction
	memset(&lep_spi_trans, 0, sizeof(spi_transaction_t));
	lep_spi_trans.tx_buffer = NULL;
	lep_spi_trans.rx_buffer = lepPacketP;
	lep_spi_trans.rxlength = LEP_PKT_LENGTH*8;

	return ret;
}


/**
 * Attempt to read a complete segment from the Lepton
 *  - Data loaded into lepBuffer
 *  - Returns true when last successful segment read, false otherwise
 */
bool vospi_transfer_segment(uint64_t vsyncDetectedUsec)
{
	uint8_t line, prevLine;
	uint8_t segment;
	bool done = false;
	bool beforeValidData = true;
	bool success = false;

	prevLine = 255;

	while (!done) {
		if (transfer_packet(&line, &segment)) {
			// Saw a valid packet
			if (line == prevLine) {
			// This is garbage data since line numbers should always increment
			done = true;
			} else {
				// Check for termination or completion conditions
				if (line == 20) {
					// Check segment
					if (!validSegmentRegion) {
						// Look for start of valid segment data
						if (segment == 1) {
							beforeValidData = false;
							validSegmentRegion = true;
						}
					} else if ((segment < 2) || (segment > 4)) {
						// Hold/Reset in starting position (always collecting in segment 1 buffer locations)
						validSegmentRegion = false;  // In case it was set
						curSegment = 1;
					}
				}
        
				// Copy the data to the lepton frame buffer or telemetry buffer
				//  - beforeValidData is used to collect data before we know if the current segment (1) is valid
				//  - then we use validSegmentRegion for remaining data once we know we're seeing valid data
				if (includeTelemetry && validSegmentRegion && (curSegment == 4) && (line >= 57)) {
					copy_packet_to_telem_buffer(line - 57);
				}
				else if ((beforeValidData || validSegmentRegion) && (line < curLinesPerSeg)) {
					copy_packet_to_lepton_buffer(line);
				}
	
				if (line == (curLinesPerSeg-1)) {
					// Saw a complete segment, move to next segment or complete frame aquisition if possible
					if (validSegmentRegion) {
						if (curSegment < 4) {
							// Setup to get next segment
							curSegment++;
						} else {
							// Got frame
							success = true;

							// Setup to get the next frame
							curSegment = 1;
							validSegmentRegion = false;
						}
					}
					done = true;
				}
			}
			prevLine = line;
		} else if ((esp_timer_get_time() - vsyncDetectedUsec) > LEP_MAX_FRAME_XFER_WAIT_USEC) {
			// Did not see a valid packet within this segment interval
      		done = true;
    	}
	}
	
  	return success;
}


/**
 * Load the a system buffer from our buffers for another task
 */
void vospi_get_frame(lep_buffer_t* sys_bufP)
{
	uint16_t* sptr = sys_bufP->lep_bufferP;
	uint16_t* lptr = &lepBuffer[0];
	uint16_t min = 0xFFFF;
	uint16_t max = 0x0000;
	uint16_t t16;

	// Load lepton image data
	while (lptr < &lepBuffer[LEP_NUM_PIXELS]) {
		t16 = *lptr++;
		if (t16 < min) min = t16;
		if (t16 > max) max = t16;
		*sptr++ = t16;
	}
	sys_bufP->lep_min_val = min;
	sys_bufP->lep_max_val = max;
	
	// Optionally load telemetry
	sys_bufP->telem_valid = includeTelemetry;
	if (includeTelemetry) {
		sptr = sys_bufP->lep_telemP;
		lptr = &lepTelem[0];
		while (lptr < &lepTelem[LEP_TEL_WORDS]) {
			*sptr++ = *lptr++;
		}
	}
}


/**
 * Configure the pipeline to include telemetry or not.
 * This should be done during initialization
 */
void vospi_include_telem(bool en)
{
	includeTelemetry = en;
	curLinesPerSeg = (en) ? LEP_TEL_PKTS_PER_SEG : LEP_NOTEL_PKTS_PER_SEG;
	curWordsPerSeg = (en) ? LEP_TEL_WORDS_PER_SEG : LEP_NOTEL_WORDS_PER_SEG;
}



//
// VoSPI Forward Declarations for internal functions
//

/**
 * Attempt to read one packet from the lepton
 *  - Return false for discard packets
 *  - Return true otherwise
 *    - line contains the packet line number for all valid packets
 *    - seg contains the packet segment number if the line number is 20
 */
static bool transfer_packet(uint8_t* line, uint8_t* seg)
{
	bool valid = false;
	esp_err_t ret;

	// *seg will be set if possible
	*seg = 0;

	// Get a packet
	ret = spi_device_polling_transmit(spi, &lep_spi_trans);
	//ret = spi_device_transmit(spi, &lep_spi_trans);
	ESP_ERROR_CHECK(ret);
  
	// Repeat as long as the frame is not valid, equals sync
	if ((*lepPacketP & 0x0F) == 0x0F) {
		valid = false;
	} else {
		*line = *(lepPacketP + 1);

		// Get segment when possible
		if (*line == 20) {
			*seg = (*lepPacketP >> 4);
		}

		valid = true;
	}

	return(valid);
}


/**
 * Copy the lepton packet to the raw lepton frame
 *   - line specifies packet line number
 */
static void copy_packet_to_lepton_buffer(uint8_t line)
{
	uint8_t* lepPopPtr = lepPacketP + 4;
	uint16_t* acqPushPtr = &lepBuffer[((curSegment-1) * curWordsPerSeg) + (line * (LEP_WIDTH/2))];
	uint16_t t;

	while (lepPopPtr <= (lepPacketP + (LEP_PKT_LENGTH-1))) {
		t = *lepPopPtr++ << 8;
		t |= *lepPopPtr++;
		*acqPushPtr++ = t;
	}
}


/**
 * Copy the lepton packet to the telemetry buffer
 *   - line specifies packet line number (only 0-2 are valid, do not call with line 3)
 */
static void copy_packet_to_telem_buffer(uint8_t line)
{
	uint8_t* lepPopPtr = lepPacketP + 4;
	uint16_t* telPushPtr = &lepTelem[line * (LEP_WIDTH/2)];
	uint16_t t;
	
	if (line > 2) return;
	
	while (lepPopPtr <= (lepPacketP + (LEP_PKT_LENGTH-1))) {
		t = *lepPopPtr++ << 8;
		t |= *lepPopPtr++;
		*telPushPtr++ = t;
	}
}


