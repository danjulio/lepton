/*
 * A version of Damien Walsh's leptonic that uses the VSYNC signal from the FLIR
 * Lepton 3/3.5 and processes each frame using an interrupt handler.  This code can
 * be substituted for his leptonic server and works with his frontend for display.
 *
 * Damien's original code: https://github.com/themainframe/leptonic
 *
 * Requires PIGPIO C library to be installed: http://abyz.me.uk/rpi/pigpio/
 * Also requires I2C to be enabled.
 *
 * Run from top-level directory: sudo ./bin/leptonic /dev/i2c-1 /dev/spidev0.0
 *
 * Software provided "as-is" without warranty of any kind in hopes that it's useful
 * to someone.
 *
 * Version 1 : August 16, 2018 Dan Julio / dan@danjuliodesigns.com
 *
 */
#include "log.h"
#include "vospi.h"
#include "cci.h"
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <zmq.h>

// The default spec for the ZMQ socket that will be used for comms with the frontend
#define ZMQ_DEFAULT_SOCKET_SPEC "tcp://*:5555"

// The size of the circular frame buffer
#define FRAME_BUF_SIZE 8

// Positions of the reader in the frame buffer
int reader = 0, writer = 0;

// semaphore tracking the number of frames available
sem_t count_sem;

// a lock protecting accesses to the frame buffer
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// The frame buffer
vospi_frame_t* frame_buf[FRAME_BUF_SIZE];

/**
 * Utility to sleep the get_frames_from_device thread
 */
void sleep_ms(int milliseconds)
{
    struct timespec ts;

    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

/**
 * Read frames from the device into the circular buffer.
 */
void* get_frames_from_device(void* hw_dev_strings)
{
    char* i2cdev_path = (char*)hw_dev_strings;
    char* spidev_path = (char*)hw_dev_strings + 64;
    int spi_fd;
    int i2c_fd;
    int n;

    // Declare a static frame to use as a scratch space to avoid locking the framebuffer while
    // we're waiting for a new frame
    vospi_frame_t frame;

    // Open the I2C device
    log_info("opening I2C device... %s", i2cdev_path);
    if ((i2c_fd = open(i2cdev_path, O_RDWR)) < 0) {
      log_fatal("I2C: failed to open device - check permissions & i2cdev enabled");
      exit(-1);
    }

    // Open the spidev device
    log_info("opening SPI device... %s", spidev_path);
    if ((spi_fd = open(spidev_path, O_RDWR)) < 0) {
      log_fatal("SPI: failed to open device - check permissions & spidev enabled");
      exit(-1);
    }

    // Initialise the VoSPI interface.  Hand it the array of frame buffers to fill.
    //  Note: there is a bug in the spi driver that causes the frequency to be
    //        0.6 what is set...  So work around that here.
    if (vospi_init(spi_fd, 33333333, &frame) == -1) {
        log_fatal("SPI: failed to condition SPI device for VoSPI use.");
        exit(-1);
    }

    // Enable VSYNC
    cci_init(i2c_fd);
    cci_set_gpio_mode(i2c_fd, LEP_OEM_GPIO_MODE_VSYNC);
    // Uncomment to enable AGC
    //cci_set_agc_enable_state(i2c_fd, CCI_AGC_ENABLED);

    // Receive frames forever
    log_info("aquiring VoSPI synchronisation");
    do {

      while (0 == sync_and_transfer_frame()) {
        sleep_ms(1);
      }

      // Only push this frame if our circular frame buffer is not full
      sem_getvalue(&count_sem, &n);
      
      if (n < FRAME_BUF_SIZE) {
        pthread_mutex_lock(&lock);

        // Copy the newly-received frame into place
        memcpy(frame_buf[writer], &frame, sizeof(vospi_frame_t));

        // Move the writer ahead
        writer = (writer + 1) & (FRAME_BUF_SIZE - 1);

        // Unlock and post the space semaphore
        pthread_mutex_unlock(&lock);
        sem_post(&count_sem);
     }

    } while (1);  // Forever

}

/**
 * Wait for reqests for frames on the ZMQ socket and respond with a frame each time.
 */
void* send_frames_to_socket(void* socket_path_ptr)
{
    // Create the ZMQ context & socket
    char* socket_path = (char*)socket_path_ptr;
    void* context = zmq_ctx_new();
    void* responder = zmq_socket(context, ZMQ_REP);

    if (zmq_bind(responder, socket_path) != 0) {
      log_fatal("Failed to bind to socket: %s", zmq_strerror(errno));
      exit(1);
    }

    // Declare a static frame
    vospi_frame_t next_frame;

    // Declare a static buffer to copy frame data into for sending
    unsigned char message_buf[VOSPI_SEGMENTS_PER_FRAME * VOSPI_PACKETS_PER_SEGMENT_NORMAL * VOSPI_PACKET_SYMBOLS];

    while (1) {

      // Receive requests
      char req_buf[10];
      zmq_recv(responder, req_buf, 10, 0);

      // Wait if there are no new frames to transmit
      sem_wait(&count_sem);

      // Uncomment to display frame_buf fill level
      /*
      int n;
      sem_getvalue(&count_sem, &n);
      log_info("n=%d",n);
      */

      // Lock the data structure to prevent new frames being added while we're reading this one
      pthread_mutex_lock(&lock);

      // Copy the next frame out ready to transmit
      memcpy(&next_frame, frame_buf[reader], sizeof(vospi_frame_t));

      // Move the reader ahead
      reader = (reader + 1) & (FRAME_BUF_SIZE - 1);

      // Unlock data structure
      pthread_mutex_unlock(&lock);

      // Prepare the message buffer
      void* message_buf_pos = &message_buf;
      for (int seg = 0; seg < VOSPI_SEGMENTS_PER_FRAME; seg ++) {
        for (int pkt = 0; pkt < VOSPI_PACKETS_PER_SEGMENT_NORMAL; pkt ++) {
          // Copy each packet into the message buffer
          memcpy(
            message_buf_pos,
            next_frame.segments[seg].packets[pkt].symbols,
            VOSPI_PACKET_SYMBOLS
          );
          message_buf_pos += VOSPI_PACKET_SYMBOLS;
        }
      }

      // Send the message
      zmq_send(responder, message_buf, sizeof(message_buf), 0);
    }
}

/**
 * Main entry point for Leptonic's ZMQ server.
 */
int main(int argc, char *argv[])
{
  char hwDevs[2][64];
  pthread_t get_frames_thread, send_frames_to_socket_thread;

  // Set the log level
  log_set_level(LOG_INFO);

  // Setup semaphores
  sem_init(&count_sem, 0, 0);

  // Check we have enough arguments to work
  if (argc < 3) {
    log_error("Can't start - I2C then SPI device file path must be specified.");
    exit(-1);
  }

  // Allocate space to receive the segments in the circular buffer
  log_info("preallocating space for segments...");
  for (int frame = 0; frame < FRAME_BUF_SIZE; frame ++) {
    frame_buf[frame] = malloc(sizeof(vospi_frame_t));
    for (int seg = 0; seg < VOSPI_SEGMENTS_PER_FRAME; seg ++) {
      frame_buf[frame]->segments[seg].packet_count = VOSPI_PACKETS_PER_SEGMENT_NORMAL;
    }
  }
  (void) strcpy(hwDevs[0], argv[1]); // I2C
  (void) strcpy(hwDevs[1], argv[2]); // SPI
  log_info("Creating get_frames_from_device thread");
  if (pthread_create(&get_frames_thread, NULL, get_frames_from_device, hwDevs)) {
    log_fatal("Error creating get_frames_from_device thread");
    return 1;
  }

  log_info("Creating send_frames_to_socket thread");
  char* socket_path = argc > 3 ? argv[3] : ZMQ_DEFAULT_SOCKET_SPEC;
  if (pthread_create(&send_frames_to_socket_thread, NULL, send_frames_to_socket, socket_path)) {
    log_fatal("Error creating send_frames_to_socket thread");
    return 1;
  }

  pthread_join(get_frames_thread, NULL);
  pthread_join(send_frames_to_socket_thread, NULL);
}
