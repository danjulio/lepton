#ifndef VOSPI_H
#define VOSPI_H

#include <stdint.h>

// The size of a single VoSPI PRMsg 
#define VOSPI_PKT_NUM_BYTES   80
#define VPSPI_MSG_NUM_PKTS    6
#define VOSPI_MSG_DATA_BYTES  (VOSPI_PKT_NUM_BYTES * VPSPI_MSG_NUM_PKTS)
#define VOSPI_MSG_TOTAL_BYTES (VOSPI_MSG_DATA_BYTES + 1)

// A single frame
#define VOSPI_FRAME_LEN       (160 * 120)
#define VOSPI_FRAME_NUM_MSGS  (VOSPI_FRAME_LEN / VOSPI_MSG_DATA_BYTES)

// Abort message seq num
#define VOSPI_ABORT_MSG       0xFF



// A single VoSPI RPMsg 
typedef struct {
	uint8_t seq;
	uint8_t data[VOSPI_MSG_DATA_BYTES];
} vospi_rpmsg_t;

// A single VoSPI frame
typedef struct {
	vospi_rpmsg_t msg[VOSPI_FRAME_NUM_MSGS];
} vospi_frame_t;



int sync_and_transfer_exp_msg(int fd, vospi_rpmsg_t* msg, uint8_t exp_seq);
int sync_and_transfer_frame(int fd, vospi_frame_t* frame);
void frame_to_pixel(vospi_frame_t* frame, uint8_t* pixbuf);

#endif /* VOSPI_H */
