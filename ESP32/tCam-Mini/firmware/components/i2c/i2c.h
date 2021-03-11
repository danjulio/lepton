/*
 * I2C Module
 *
 * Provides I2C Access routines for other modules/tasks.  Provides a locking mechanism
 * since the underlying ESP IDF routines are not thread safe.
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
#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include "esp_system.h"

//
// I2C constants
//

// Slave buffer size (not used)
#define I2C_MASTER_TX_BUF_LEN 0
#define I2C_MASTER_RX_BUF_LEN 0

#define ACK_CHECK_EN 0x1
#define ACK_CHECK_DIS 0x0
#define ACK_VAL 0x0
#define NACK_VAL 0x1 


//
// I2C API
//
esp_err_t i2c_master_init();
void i2c_lock();
void i2c_unlock();
esp_err_t i2c_master_read_slave(uint8_t addr7, uint8_t *data_rd, size_t size);
esp_err_t i2c_master_write_slave(uint8_t addr7, uint8_t *data_wr, size_t size);


#endif /* I2C_H */