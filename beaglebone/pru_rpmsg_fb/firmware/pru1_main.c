/*
 * PRU Firmware for IR camera FLIR Lepton 3
 *
 * Implement the communication portion of an interface for the Lepton camera.
 * This code assumes the Lepton AGC has been enabled (each 16-bit
 * data word contains 8-bits of data).  After being triggered by PRU0
 * that is reading the raw data from the camera and storing it in a circular
 * queue in shared memory, this code writes the data to the host via the RPMsg
 * facility.
 *
 * In order to not overwhelm the host processor, 6 80-byte Lepton packets
 * are combined with a sequence number in each message.  Messages are sent
 * at a slightly lower transfer rate than data is being stored into the 
 * circular buffer but fast enough to a) keep PRU0 from overwriting good data
 * since the data is larger than the buffer size and b) keep up with the valid
 * frame rate of the lepton.
 *
 * RPMsg packets consist of a one-byte sequence number to allow the host to
 * validate incoming data followed by 480 bytes of 8-bit pixel data.  The
 * sequence number ranges from 0 to 39.  It is set to 0xFF to indicate that  the
 * transfer is invalid (PRU0 detected an invalid packet after it initiated PRU1
 * transfer).
 *
 * The host can control frame aquisition by sending a one-byte message, either '0'
 * to disable or '1' to enable, via RPMsg to PRU1.  Frame aquisition will be
 * automatically disabled if there is a failure to send a message to the host
 * (e.g. host buffer full because consuming application has died).
 *
 * The LED attached to P8.45 is lit while transfering data.  It also blinks at
 * varying rates during RPMsg initialization and at PRU boot.  Note that HDMI
 * must be disabled.
 *
 * Copyright (C) 2018 Dan Julio <dan@danjuliodesigns.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */
/* We are compiling for PRU1 */
#define PRU1

#include <stdint.h>
#include <pru_cfg.h>
#include <pru_ctrl.h>
#include <pru_intc.h>
#include <pru_rpmsg.h>
#include "pru_common.h"
#include "resource_table_1.h"


/* ========= */
/* Constants */
/* ========= */

/* ----------- */
/* Sample Time */
/* ----------- */
#define PKT_XMIT_USEC    1024
#define PRU_CLK_PER_USEC 200
#define PRU_XMIT_TO      (PKT_XMIT_USEC * PRU_CLK_PER_USEC)


/* -------------- */
/* PRU IO related */
/* -------------- */
#define LED     0       //P8_45

#define SET_PIN(bit,high)	if(high) __R30 |= (1 << (bit)); else __R30 &= ~(1 << (bit));
#define INVERT_PIN(bit)		__R30 ^= (1 << (bit));
#define CHECK_PIN(bit)		__R31 & (1 << (bit))


/* ------------------ */
/* Lepton 3.5 related */
/* ------------------ */
#define LEP_AGC_PACKET_SIZE 80
#define LEP_AGC_FRAME_SIZE  (160 * 120)
#define LEP_PKTS_PER_MSG    6
#define BYTES_PER_MSG       (LEP_AGC_PACKET_SIZE * LEP_PKTS_PER_MSG)
#define NUM_MSGS            (LEP_AGC_FRAME_SIZE / BYTES_PER_MSG)


/* --------- */
/* Run state */
/* --------- */
#define RUN_STATE_STOPPED 0
#define RUN_STATE_WAIT    1
#define RUN_STATE_SEND    2


/* ------------- */
/* RPMsg related */
/* ------------- */

/*
 * Used to make sure the Linux drivers are ready for RPMsg communication
 * Found at linux-x.y.z/include/uapi/linux/virtio_config.h
 */
#define VIRTIO_CONFIG_S_DRIVER_OK  4

/* RPMSG packet overhead is 16 bytes.  All the rest is available for data */
#define RPMSG_BUF_SIZE             512

/* RPMSG packet size */
#define RPMSG_MSG_LEN              (BYTES_PER_MSG + 1)

/* "Abort" sequence number */
#define PRMSG_ABORT_SEQ_NUM        0xFF


/* ================ */
/* Global Variables */
/* ================ */
volatile register unsigned __R31;
volatile register unsigned __R30;

/* Shared memory buffer */
volatile uint8_t* buf_en_reg_ptr = SMEM_EN_REG;
volatile uint8_t* buf_pru1_cmd_ptr =  SMEM_CMD_REG;
volatile uint8_t* buf_cur_ptr = SMEM_BUF_START;

/* Local variables */
uint8_t run_state = RUN_STATE_STOPPED;

uint8_t cur_seq_num = 0;
uint8_t msg_buffer[RPMSG_BUF_SIZE];

struct pru_rpmsg_transport transport;
uint16_t rpmsg_src, rpmsg_dst, rpmsg_len;



/* =========== */
/* Subroutines */
/* =========== */

void init_pru()
{
	/* Enable OCP Master Port */
	CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

	/* Always start off idle - delete any state laying around */
	*buf_pru1_cmd_ptr = P1_CMD_IDLE;

	/* Slow blink the LED to let them know we've started */
	SET_PIN(LED,1);
	__delay_cycles(100000000);
	SET_PIN(LED,0);
}


/*
 * Initialize RPMsg communication with the host - this routine may hang until communication is
 * established.  It "fast" blinks the LED while waiting at different rates depending where it
 * is waiting.
 */
void init_rpmsg()
{
	uint8_t *status;

	/* Clear the status of the PRU-ICSS system event that the ARM will use to 'kick' us */
	CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

	/* Wait for the Linux driver */
	status = &resourceTable.rpmsg_vdev.status;
	while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK)) {
		SET_PIN(LED,1);
		__delay_cycles(10000000);
		SET_PIN(LED,0);
		__delay_cycles(10000000);
	}

	/* Initialize the RPMsg transport structure */
	pru_rpmsg_init(&transport, &resourceTable.rpmsg_vring0, &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST);

	/* Create the PRMsg channel between the PRU and ARM user space using the transport structure */
	while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME, CHAN_DESC, CHAN_PORT) != PRU_RPMSG_SUCCESS) {
		SET_PIN(LED,1);
		__delay_cycles(20000000);
		SET_PIN(LED,0);
		__delay_cycles(20000000);
	}
}	


void init_send()
{
	cur_seq_num = 0;
	buf_cur_ptr = SMEM_BUF_START;
}


/*
 * Read data from the circular buffer and store it in our local buffer for transmission
 */
void get_lep_msg()
{
	uint16_t i;

	/* Load this message's sequence number */
	msg_buffer[0] = cur_seq_num;

	/* Copy bytes from the circular buffer into our message buffer */
	for (i=1; i<=BYTES_PER_MSG; i++) {
		msg_buffer[i] = *buf_cur_ptr++;
		if (buf_cur_ptr > SMEM_BUF_END) {
			buf_cur_ptr = SMEM_BUF_START;
		}
	}
}


/*
 * Prepare an "abort" message in our local buffer for transmission
 */
void set_abort_msg()
{
	msg_buffer[0] = PRMSG_ABORT_SEQ_NUM;
}


/*
 * Attemp to send a message to the host system.  Return 0 if the send was unsuccessful, 1
 * if it was successful
 */
int send_msg()
{
	if (pru_rpmsg_send(&transport, rpmsg_dst, rpmsg_src, msg_buffer, RPMSG_MSG_LEN) == PRU_RPMSG_SUCCESS) {
		return 1;
	}
	return 0;
}


/*
 * Timer control
 */
void disable_timer()
{
	PRU1_CTRL.CTRL_bit.CTR_EN = 0;
}

void init_timer()
{
	PRU1_CTRL.CTRL_bit.CTR_EN = 0;  /* Disable timer */
	PRU1_CTRL.CYCLE = 0;            /* Reset timer */
	PRU1_CTRL.CTRL_bit.CTR_EN = 1;  /* Enable timer */
}


/*
 * Returns 1 if the timer has expired - and resets timer 
 */
int timer_expired()
{
	if (PRU1_CTRL.CYCLE >= PRU_XMIT_TO) {
		init_timer();
		return 1;
	}
	return 0;
}


/*
 * Disable aquisition
 */
void disable_acq()
{
	/* Stop ourself */
	run_state = RUN_STATE_STOPPED;
	disable_timer();
	SET_PIN(LED,0);

	/* Tell PRU0 to stop */
	*buf_en_reg_ptr = P0_DISABLE;
}



/* ==== */
/* Main */
/* ==== */

void main()
{
	/* Initialize */
	init_pru();
	init_rpmsg();

	/* Loop Forever */
	while (1) {
		/* Look for incoming commands */
		if (__R31 & HOST_INT) {
			/* Clear the event status */
			CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

			/* Receive all available messages, multiple messages can be sent per kick */
			while (pru_rpmsg_receive(&transport, &rpmsg_src, &rpmsg_dst, msg_buffer, &rpmsg_len) == PRU_RPMSG_SUCCESS) {
				if (rpmsg_len > 0) {
					if (msg_buffer[0] == '0') {
						/* Stop */
						disable_acq();
					} else {
						/* Start */
						run_state = RUN_STATE_WAIT;

						/* Tell PRU0 to start */
						*buf_en_reg_ptr = P0_ENABLE;
					}
				}
			}
		}
		
		if (run_state != RUN_STATE_STOPPED) {
			if (run_state == RUN_STATE_WAIT) {
				/* Wait to be triggered */
				if (*buf_pru1_cmd_ptr == P1_CMD_IN_FRAME) {
					run_state = RUN_STATE_SEND;
					init_send();
					init_timer();
					SET_PIN(LED,1);
				}
			} else {
				int host_present = 1;  /* Assume host present */

				if (*buf_pru1_cmd_ptr == P1_CMD_ABORT) {
					/* Terminate this transfer */
					*buf_pru1_cmd_ptr = P1_CMD_IDLE; /* Tell PRU0 we got the abort */
					run_state = RUN_STATE_WAIT;
					set_abort_msg();
					host_present = send_msg();
					SET_PIN(LED,0);

				} else {
					/* Process data from the circular buffer */
					if (timer_expired()) {
						get_lep_msg();
						host_present = send_msg();
						if (++cur_seq_num == NUM_MSGS) {
						       	/* Tell PRU0 we finished the frame */
							*buf_pru1_cmd_ptr = P1_CMD_IDLE;

							/* Done with this frame */
							run_state = RUN_STATE_WAIT;
							SET_PIN(LED,0);
						}
					}
				}

				if (host_present == 0) {
					/* Stop acquisition */
					disable_acq();

					/* Attempt to destory the existing channel */
					(void) pru_rpmsg_channel(RPMSG_NS_DESTROY, &transport, CHAN_NAME,
					                         CHAN_DESC, CHAN_PORT);

					/* Attempt to reinitialize the RPmsg facility */
					init_rpmsg();
				}
			}
		}
	}
}
