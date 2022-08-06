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
#include "sif_utilities.h"
#include "driver/uart.h"
#include "system_config.h"



//
// SIF API
//

void sif_init()
{
	/* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = CMD_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, BRD_W_SIF_TX_IO, BRD_W_SIF_RX_IO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_1, SIF_RX_BUFFER_SIZE, SIF_TX_BUFFER_SIZE, 0, NULL, 0);
}


void sif_send(const char* s, int len)
{
	int i = 0;
	int written;
	
	// Write as many bytes as possible to the TX circular buffer - this may block
	while (len != 0) {
		written = uart_write_bytes(UART_NUM_1, s+i, len);
		i += written;
		len -= written;
	}
}


int sif_get(char* s, int max)
{
	size_t n;
	
	uart_get_buffered_data_len(UART_NUM_1, &n);
	
	if (n > max) n = max;
	
	if (n != 0) {
		n = uart_read_bytes(UART_NUM_1, (uint8_t*) s, n, 0);
	}
	
	return n;
}
