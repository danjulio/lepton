/*
 * System Configuration File
 *
 * Contains system definition and configurable items.
 *
 * Copyright 2020-2021 Dan Julio
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
#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include "esp_system.h"


// ======================================================================================
// System debug
//

// Undefine to include the system monitoring task (included only for debugging/tuning)
//#define INCLUDE_SYS_MON



// ======================================================================================
// System hardware definitions
//

//
// IO Pins
//
#define BTN_IO            0
#define RED_LED_IO        14
#define GREEN_LED_IO      15

#define LEP_SCK_IO        5
#define LEP_CSN_IO        12
#define LEP_VSYNC_IO      13
#define LEP_MISO_IO       19
#define LEP_RESET_IO      21

#define I2C_MASTER_SDA_IO 23
#define I2C_MASTER_SCL_IO 22

#define MODE_SENSE_IO     32

#define SIF_RX_IO         26
#define SIF_TX_IO         27

#define HOST_SCK_IO       34
#define HOST_MISO_IO      33
#define HOST_CSN_IO       35




//
// Hardware Configuration
//

// I2C
#define I2C_MASTER_NUM     1
#define I2C_MASTER_FREQ_HZ 100000

// SPI
//   Lepton uses HSPI (no MOSI)
//   Host SPI Slave uses VSPI (no MOSI)
#define LEP_SPI_HOST    HSPI_HOST
#define LEP_DMA_NUM     2
#define LEP_SPI_FREQ_HZ 16000000

#define HOST_SPI_HOST   VSPI_HOST
#define HOST_DMA_NUM    1
#define HOST_SPI_MODE   0




// ======================================================================================
// System configuration
//

// Camera model number
//  1. tCam Mini using the WiFi interface is model 2
//  2. tCam Mini using the Serial/SPI interface is model 3
#define CAMERA_MODEL_NUM_WIFI 2
#define CAMERA_MODEL_NUM_SIF  3

// tCam capabilities mask 
//   Bit  8: Non-radiometric (set during run-time)
//   Bit 16: Has Battery (not set)
//   Bit 17: Has Filesystem (not set)
//   Bit 18: Has OTA Firmware Updates (set)
#define CAMERA_CAP_MASK_RAD     0x00040000
#define CAMERA_CAP_MASK_NONRAD  0x00040100


// Image (Lepton + Telemetry + Metadata) json object text size
// Based on the following items:
//   1. Base64 encoded Lepton image size: (160x120x2)*4 / 3
//   2. Base64 encoded Lepton telemetry size: (80x3x2)*4 / 3
//   3. Metadata text size: 1024
//   4. Json object overhead (child names, formatting characters, NLs): 128
// Manually calculate this and round to 4-byte boundary
#define JSON_MAX_IMAGE_TEXT_LEN (1024 * 53)

// Maximum firmware update chunk request size
#define FM_UPD_CHUNK_MAX_LEN    (1024 * 8)

// Maximum command response json object text size
#define JSON_MAX_RSP_TEXT_LEN   2048

// Command Response Buffer Size (large enough for several responses)
#define CMD_RESPONSE_BUFFER_LEN (JSON_MAX_RSP_TEXT_LEN * 4)

// Maximum incoming command json string length
// Large enough for longest command: fw_segment
//    1. Base64 encoded firmware chunk: FM_UPD_CHUNK_MAX_LEN * 4 / 3
//    2. Json object overhead: 128
// Manually calculate this and round to a 4-byte boundary
#define JSON_MAX_CMD_TEXT_LEN   (12 * 1024)

// TCP/IP listening port
#define CMD_PORT 5001

// Serial port baud rate
#define CMD_BAUD_RATE 230400

// Maximum serial port receive buffer (sized to hold any command but fw_segment)
#define SIF_RX_BUFFER_SIZE 2048

// Maximum serial port transmit buffer (sized to hold a non-image response)
#define SIF_TX_BUFFER_SIZE JSON_MAX_RSP_TEXT_LEN

#endif // SYSTEM_CONFIG_H