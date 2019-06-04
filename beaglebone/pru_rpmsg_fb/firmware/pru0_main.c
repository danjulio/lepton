/*
 * PRU Firmware for IR camera FLIR Lepton 3
 *
 * Implement the SPI portion of an interface for the Lepton camera.
 * This code assumes the Lepton AGC has been enabled (each 16-bit
 * data word contains 8-bits of data).  It reads packets from the camera
 * using a bit-banged MISO-only SPI interface running at 16.67 MHz.
 * It stores non-discard packets in a circular buffer located in shared
 * memory.  Only the low 8-bits of each Lepton data words are stored for
 * a total of 80 bytes per packet.  It assumes it is receiving a valid
 * frame when it sees packet 20 of segment 1 and notifies PRU1 that it can
 * start pushing packet data up to the host via RPMsg. 
 *
 * This PRU always starts loading at the beginning of the circular buffer.
 * Since the buffer is smaller than a complete frame it only starts pushing
 * data for a new frame after it sees that PRU1 has finished consuming
 * the current frame.  This means PRU1 must send data before a valid new
 * frame starts (every third frame).
 *
 * Also since the circular buffer is smaller than a complete frame it ends
 * up overwriting the start of the buffer with the end of the frame while
 * PRU1 is reading the buffer.  It's important to make sure the timing
 *
 * One frame requires 80 x 240 = 19200 bytes.  Almost the entire 12K shared
 * memory buffer is available for the circular buffer.
 *
 * This PRU reads one packet every 128 uSec (less than the maximum 158 uSec
 * period that is required to keep up with the Lepton).  The other PRU must
 * read data out fast enough yet slowly enough not to overrun this PRU. Since
 * the timing is fixed we can find this balance without requiring the two
 * PRUs to communicate other than valid (or cancelled buffer).
 *
 * This PRU also maintains a counter to validate each packet (the counter
 * value is the (segment-1) * packet number).  If it sees a non-discard 
 * packet with an unexpected count it resets.  If it has previously triggered
 * PRU1 to start reading out packets then it sends an abort notification to PRU1.
 *
 * PRU1 sets an enable locatation in shared memory buffer to 1 to indicate when
 * to run.  Otherwise this PRU spins waiting to be enabled.
 *
 * The LED attached to P9.31 is lit while receiving a valid frame (from segment 1,
 * packet 20 to segment 4, packet 59).
 *
 * Copyright (C) 2018 Dan Julio <dan@danjuliodesigns.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */
/* We are compiling for PRU0 */
#define PRU0

#include <stdint.h>
#include <pru_cfg.h>
#include <pru_ctrl.h>
#include "pru_common.h"
#include "resource_table_0.h"


/* ========= */
/* Constants */
/* ========= */

/* ----------- */
/* Sample Time */
/* ----------- */
#define PKT_SAMPLE_USEC  128
#define PRU_CLK_PER_USEC 200
#define PRU_SAMPLE_TO    (PKT_SAMPLE_USEC * PRU_CLK_PER_USEC)

/* Number of packet sample intervals to detect need to resync Lepton  */
/*   Time is greater than 1/9 second plus 10 mSec per spec 4.2.3.3.1  */
#define RESYNC_THRESHOLD_USEC 200000
#define RESYNC_THRESHOLD_COUNT (RESYNC_THRESHOLD_USEC / PKT_SAMPLE_USEC)

/* PRU Cycles to reset Lepton -> 185 mSec */
#define LEP_RESYNC_USEC 190000
#define LEP_RESYNC_CYCLES (LEP_RESYNC_USEC * PRU_CLK_PER_USEC)


/* -------------- */
/* PRU IO related */
/* -------------- */
#define CLK     14      //P8_12
#define MISO    5       //P9_27
#define CSN     2       //P9_30
#define LED     0       //P9_31

#define SET_PIN(bit,high)	if(high) __R30 |= (1 << (bit)); else __R30 &= ~(1 << (bit));
#define INVERT_PIN(bit)		__R30 ^= (1 << (bit));
#define CHECK_PIN(bit)		__R31 & (1 << (bit))


/* ------------------ */
/* Lepton 3.5 related */
/* ------------------ */
#define LEP_PACKET_SIZE      164 // Bytes
#define LEP_PACKET_DATA_SIZE 160 // Bytes


/* --------- */
/* Run state */
/* --------- */
#define RUN_STATE_STOPPED 0
#define RUN_STATE_DATA    1
#define RUN_STATE_DISCARD 2


/* ---------------------- */
/* Received Packet Status */
/*    from get_packet     */
/* ---------------------- */
#define PKT_DISCARD  0
#define PKT_GOOD     1
#define PKT_TRIGGER  2
#define PKT_ILLEGAL  3


/* ================ */
/* Global Variables */
/* ================ */
volatile register unsigned __R31;
volatile register unsigned __R30;

/* Shared memory buffer */
volatile uint8_t* buf_en_reg_ptr = SMEM_EN_REG;
volatile uint8_t* buf_pru1_cmd_ptr =  SMEM_CMD_REG;
volatile uint8_t* buf_cur_ptr = SMEM_BUF_START;

uint8_t run_state = RUN_STATE_STOPPED;
uint8_t cur_segment = 0; /* 0 while waiting, 1 - 4 while receiving */
uint8_t cur_packet = 59; /* 0 - 59 */
uint8_t cur_count = 0;   /* 0 - 239 */
uint16_t lep_resync_count = 0; /* Counts sample intervals, reset each trigger */
uint32_t lep_discard_pkt_count = 0;  /* Counts consecutive Lepton discard packets in a row */
                                     /* for diag purposes to read out with prudebug */


/* =========== */
/* Subroutines */
/* =========== */
void init_pru()
{
	/* Enable OCP Master Port */
	CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

	/* Make sure we always start up disabled - clear any enable left laying around */
	*buf_en_reg_ptr = P0_DISABLE;

	/* Set CSN de-asserted and SCK high */
	SET_PIN(CSN,1);
	SET_PIN(CLK,1);

	/* Slow blink the LED to let them know we've started */
	SET_PIN(LED,1);
	__delay_cycles(100000000);
	SET_PIN(LED,0);
}


void init_capture()
{
	buf_cur_ptr = SMEM_BUF_START;
	cur_segment = 0;   /* setup to receive segment 1 in packet 20 as first valid segment */
	cur_packet = 59;   /* setup to receive packet 0 as first valid packet */
	cur_count = 0;
}


void disable_timer()
{
	PRU0_CTRL.CTRL_bit.CTR_EN = 0;
}


void init_timer()
{
	PRU0_CTRL.CTRL_bit.CTR_EN = 0;  /* Disable timer */
	PRU0_CTRL.CYCLE = 0;            /* Reset timer */
	PRU0_CTRL.CTRL_bit.CTR_EN = 1;  /* Enable timer */
}


/* Returns 1 if the timer has expired - and resets timer */
int timer_expired()
{
	if (PRU0_CTRL.CYCLE >= PRU_SAMPLE_TO) {
		init_timer();
		return 1;
	}
	return 0;
}


/*
 * Read 8-bit word from SPI pins, SPI mode 3 - CPOL=1, CPHA=1, CSN low on entry
 *   Tuned to 12-13 clocks -> 60-65 nS -> 16.67-15.38 MHz (there is 1-clock variability
 *   writing to the miso variable depending on other activity in the PRU memory subystem)
 */
uint8_t spi_read8() 
{
	uint8_t i;
	uint8_t miso;

	miso = 0x0;
	
	for (i = 0; i < 8; i++) {
		miso <<= 1;
		SET_PIN(CLK,0);
		__delay_cycles(4);
		
		SET_PIN(CLK,1);
		if (CHECK_PIN(MISO))
			miso |= 0x01;
		else
			miso &= ~0x01;
	}
	return miso;
}


/*
 * Read one packet from the Lepton and store it in our buffer if it is valid
 */
uint8_t get_packet()
{
	uint8_t i;
	uint8_t pktNumHigh;
	uint8_t pktNumLow;

	/* Read one packet */
	pktNumHigh = spi_read8();  /* TTT bits and discard indication */
	pktNumLow = spi_read8();   /* Packet number */
	i = spi_read8();           /* Throw away CRC for now */
	i = spi_read8();           /* Throw away CRC for now */

	/* Look to see if we should discard this packet */
	if (((pktNumHigh & 0x0F) == 0x0F) || (run_state == RUN_STATE_DISCARD)) {
		/* Count discard packets */
		if ((pktNumHigh & 0x0F) == 0x0F) {
			++lep_discard_pkt_count;
		} else {
			lep_discard_pkt_count = 0;
		}

		/* Throw away this packet */
		for (i=0; i<LEP_PACKET_DATA_SIZE; i++) {
			(void) spi_read8();
		}
		return PKT_DISCARD;
	}

	/* Reset consecutive discard count */
	lep_discard_pkt_count = 0;

	/* Store packet - low 8-bits of each word */
	for (i=0; i<LEP_PACKET_DATA_SIZE/2; i++) {
		(void) spi_read8();                /* Skip high half */
		*buf_cur_ptr++ = spi_read8();      /* Store low half - output of AGC module */
		if (buf_cur_ptr > SMEM_BUF_END) {
			buf_cur_ptr = SMEM_BUF_START;
		}
	}

	/* Update state */
	if (pktNumLow == 20) {
		/* Check and update our segment */
		i = (pktNumHigh & 0x70) >> 4;    /* TTT bits */

		if ((cur_segment + 1) == i) {
			/* Expected segment */
			cur_segment = i;

			if (cur_packet == 19) {
				/* Expected packet */
				cur_packet = 20;

				if (cur_segment == 1) {
					/* Looks like a good frame, trigger other PRU */
					return PKT_TRIGGER;
				}
			} else {
				/* Unexpected packet */
				return PKT_ILLEGAL;
			}
		} else {
			/* Unexpected segment number */
			return PKT_ILLEGAL;
		}
	} else if (cur_packet == 59) {
		if (pktNumLow == 0) {
			/* Expected packet for start of next segment */
			cur_packet = 0;
		} else {
			/* Unexpected packet */
			return PKT_ILLEGAL;
		}
	} else {
		if ((cur_packet + 1) == pktNumLow) {
			/* Expected next packet in this segment */
			cur_packet = pktNumLow;
		} else {
			/* Unexpected packet */
			return PKT_ILLEGAL;
		}
	}

	return PKT_GOOD;
}


/* ==== */
/* Main */
/* ==== */

void main()
{
	uint8_t pkt_response;

	init_pru();

	/* Loop Forever */
	while (1) {
		/* Check our enable register */
		if (*buf_en_reg_ptr == P0_ENABLE) {
			if (run_state == RUN_STATE_STOPPED) {
				/* Start up */
				run_state = RUN_STATE_DATA;
				init_capture();
				init_timer();
				lep_resync_count = 0;

				/* Assert CS */
				SET_PIN(CSN,0);
			}
		} else {
			if (run_state != RUN_STATE_STOPPED) {
				/* Shut down */
				run_state = RUN_STATE_STOPPED;
				disable_timer();
				SET_PIN(LED,0);

				/* De-assert CS */
				SET_PIN(CSN,1);
			}
		}

		/* Get data if enabled */
		if (run_state != RUN_STATE_STOPPED) {
			/* Check if PRU1 is done processing and we can start pushing data again */
			if (run_state == RUN_STATE_DISCARD) {
				if (*buf_pru1_cmd_ptr == P1_CMD_IDLE) {
					run_state = RUN_STATE_DATA;

					/* Initialize capture system for next frame */
					init_capture();
					lep_resync_count = 0;
				}
			}

			/* Check if time to read and process a packet */
			if (timer_expired()) {
				pkt_response = get_packet();
				lep_resync_count++;
				if (pkt_response == PKT_ILLEGAL) {
					/* Notify PRU1 if necessary */
					if (*buf_pru1_cmd_ptr == P1_CMD_IN_FRAME) {
						*buf_pru1_cmd_ptr = P1_CMD_ABORT;
						/* PRU1 will clear command when it reads it */
					}

					/* Try to restart */
					init_capture();
					SET_PIN(LED,0);
				} else if (pkt_response == PKT_TRIGGER) {
					/* Got Seg1/Pkt20 - Tell PRU1 to start processing */
					*buf_pru1_cmd_ptr = P1_CMD_IN_FRAME;

					/* Reset lepton resync counter because we expect this is a good frame */
					lep_resync_count = 0;

					/* Set LED */
					SET_PIN(LED,1);
				} else if (pkt_response == PKT_GOOD) {
					/* Look for last packet just pushed */
					if ((cur_packet == 59) && (cur_segment == 4)) {
						/* Discard packets until PRU1 is done */
						run_state = RUN_STATE_DISCARD;

						/* Clear LED */
						SET_PIN(LED,0);
					}
				}

				/* Look for need to resync Lepton */
				if (lep_resync_count == RESYNC_THRESHOLD_COUNT) {
					/* De-assert CS and LED */
					SET_PIN(CSN,1);
					SET_PIN(LED,0);

					/* Delay to allow Lepton to resync */
					__delay_cycles(LEP_RESYNC_CYCLES);

					/* Reinitialize capture system */
					init_capture();
					init_timer();
					lep_resync_count = 0;

					/* Re-assert CS */
					SET_PIN(CSN,0);
				}
			}

		}
	}
}
