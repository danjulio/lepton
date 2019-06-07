#include "log.h"
#include "vospi.h"

#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


/**
 *  Attempt to transfer a single VoSPI message with the expected sequence number.
 *  Returns:
 *    -1 : Bad Read - fatal
 *     0 : Successful message
 *     1 : Non expected sequence number
 *     2 : Abort detected
 */
int sync_and_transfer_exp_msg(int fd, vospi_rpmsg_t* msg, uint8_t exp_seq)
{
	int rsp;

	rsp = read(fd, (uint8_t*) msg, VOSPI_MSG_TOTAL_BYTES);

	if (rsp < 1) {
		log_fatal("RPMSG: failed to transfer packet");
		return -1;
	}

	if (msg->seq == VOSPI_ABORT_MSG) {
		return 2;
	} else if (msg->seq == exp_seq) {
		return 0;
	} else {
		return 1;
	}
}


/**
 *  Transfer a single VoSPI frame.
 *  Returns:
 *    -1 : Bad Read - fatal
 *     0 : Successful transfer
 *     1 : Non sequential sequence numbers
 *     2 : Abort detected
 *     3 : Could not sync
 */
int sync_and_transfer_frame(int fd, vospi_frame_t* frame)
{
	int rsp;
	int seq = 0;
	int cnt = 0;

	// Keep streaming packets until we receive a valid, first packets to sync
	log_debug("Synchronising with first message");
	while (seq == 0) {
		rsp = sync_and_transfer_exp_msg(fd, &frame->msg[seq], seq);
		log_debug("  Returned %d for seq %d", rsp, seq);
		switch (rsp) {
			case -1:
				return -1;
			case 0:
				/* success */
				seq = 1;
				break;
			case 1:
				/* keep waiting for a while */
				if (++cnt > VOSPI_FRAME_NUM_MSGS) {
					return 3;
				}
				break;
			case 2:
				return 2;
			default:
				return 3;
		}
	}

	// Attempt to read remaining frame messages
	while (seq < VOSPI_FRAME_NUM_MSGS) {
		log_debug("synchronising with message %d", seq);
		rsp = sync_and_transfer_exp_msg(fd, &frame->msg[seq], seq);
		if (rsp == 0) {
			// success
			seq++;
		} else {
			// failed
			return rsp;
		}
	}

	return 0;
}


/**
 * Copy data from a frame into a pixel buffer discarding the sequence numbers
 */
void frame_to_pixel(vospi_frame_t* frame, uint8_t* pixbuf)
{
	int i;
	int seq;

	for (seq=0; seq < VOSPI_FRAME_NUM_MSGS; seq++) {
		for (i=0; i<VOSPI_MSG_DATA_BYTES; i++) {
			*pixbuf++ = frame->msg[seq].data[i];
		}
	}
}

