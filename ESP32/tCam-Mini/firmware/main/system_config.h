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
//   Lepton uses HSPI (no MOSI)
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




//
// Hardware Configuration
//

// I2C
#define I2C_MASTER_NUM     1
#define I2C_MASTER_FREQ_HZ 100000

// SPI
//   Lepton uses HSPI (no MOSI)
//   LCD and TS use VSPI
#define LEP_SPI_HOST    HSPI_HOST
#define LEP_DMA_NUM     2
#define LEP_SPI_FREQ_HZ 16000000




// ======================================================================================
// System configuration
//

// Camera model number - tCam Mini is model 2 
#define CAMERA_MODEL_NUM 2


// Image (Lepton + Telemetry + Metadata) json object text size
// Based on the following items:
//   1. Base64 encoded Lepton image size: (160x120x2)*4 / 3
//   2. Base64 encoded Lepton telemetry size: (160x3x2)*4 / 3
//   3. Metadata text size: 2048
//   4. Json object overhead (child names, formatting characters, NLs): 256
// Manually calculate this and round to 4-byte boundary
#define JSON_MAX_IMAGE_TEXT_LEN (1024 * 54)

// Max command response json object text size
#define JSON_MAX_RSP_TEXT_LEN   2048

// Command Response Buffer Size (large enough for several responses)
#define CMD_RESPONSE_BUFFER_LEN (JSON_MAX_RSP_TEXT_LEN * 4)

// Maximum incoming command json string length (large enough for longest command)
#define JSON_MAX_CMD_TEXT_LEN   2048

// Maximum TCP/IP Socket receiver buffer
//  This should be large enough for the maximum number of command received at a time
//  (probably 2 is ok)
#define CMD_MAX_TCP_RX_BUFFER_LEN 4096

// TCP/IP listening port
#define CMD_PORT 5001

#endif // SYSTEM_CONFIG_H