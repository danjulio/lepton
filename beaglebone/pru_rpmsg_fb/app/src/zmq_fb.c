#include "fb.h"
#include "log.h"
#include "vospi.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h> 
#include <zmq.h>


// The default spec for the ZMQ socket that will be used for comms with the frontend
#define ZMQ_DEFAULT_SOCKET_SPEC "tcp://127.0.0.1:5555"

/* ------------ */
/* Device files */
/* ------------ */
char fb_dev[] = "/dev/fb0";


/* --------------- */
/* Local Variables */
/* --------------- */
uint8_t pixbuf[VOSPI_FRAME_LEN];


/*
 * Sleep function
 */
void sleep_ms(int ms)
{
  struct timespec ts;

  ts.tv_sec = ms/1000;
  ts.tv_nsec = (ms % 1000) * 1000000;
  nanosleep(&ts, NULL);
}


/**
 * Main entry point for PRU-based Lepton FB display
 */
int main(int argc, char *argv[])
{
  char req_buf[10];

  // Set the log level
  log_set_level(LOG_INFO);

  // Create the ZMQ context & socket
  char* socket_path = argc > 2 ? argv[2] : ZMQ_DEFAULT_SOCKET_SPEC;
  void* context = zmq_ctx_new();
  void* requester = zmq_socket(context, ZMQ_REQ);
  zmq_connect(requester, socket_path);

  // Set the request frame string (could be anything)
  strcpy(req_buf, "get");

  // Initialize frame buffer
  (void) init_fb(fb_dev);

  // Setup colormap if user has selected a non-default
  if (argc > 1) {
	  set_colormap(atoi(argv[1]));
  }

  while (1) {
    // Request a frame
    zmq_send(requester, req_buf, sizeof(req_buf), 0);

    // Wait for a response
    zmq_recv(requester, pixbuf, VOSPI_FRAME_LEN, 0);

    // Render it into the frame buffer
    update_fb(pixbuf);

    // Sleep a bit between frames
    sleep_ms(111);
  }
}
