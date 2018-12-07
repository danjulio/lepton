#include "cci.h"
#include "fb.h"
#include "log.h"
#include "vospi.h"
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>


// The size of the circular frame buffer
#define FRAME_BUF_SIZE 8

/* ------------ */
/* Device files */
/* ------------ */
char i2c_dev[] = "/dev/i2c-2";
char pru_dev[] = "/dev/rpmsg_pru31";
char fb_dev[] = "/dev/fb0";


/* --------------- */
/* Local Variables */
/* --------------- */

// Positions of the reader and writer in the frame buffer
int reader = 0, writer = 0;

// semaphore tracking the number of frames available
sem_t count_sem;

// a lock protecting accesses to the frame buffer
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// PRU file device for RPMsg
//
int pru_fd;

// The frame buffer
vospi_frame_t* frame_buf[FRAME_BUF_SIZE];



/**
 * Attempt to configure the Lepton into AGC mode via the I2C interface
 */
int init_lepton()
{
  int fd;
  uint32_t rsp;

  // Open the I2C device
  log_info("opening I2C device ... %s", i2c_dev);
  if ((fd = open(i2c_dev, O_RDWR)) < 0) {
    log_fatal("I2C: failed to open device - check permissions & i2c enabled");
    return -1;
  }

  // Initialize I2C interface
  cci_init(fd);
  
  // Perform a FFC to (re)initialize the sensor
  /*
  log_info("  Perform FFC");
  cci_run_ffc(fd);
  sleep(2);
  */

  // Configure Radiometry for TLinear disabled (to support AGC)
  cci_set_radiometry_enable_state(fd, CCI_RADIOMETRY_ENABLED);
  rsp = cci_get_radiometry_enable_state(fd);
  log_info("  Radiometry = %d", rsp);
  cci_set_radiometry_tlinear_enable_state(fd, CCI_RADIOMETRY_TLINEAR_DISABLED);
  rsp = cci_get_radiometry_tlinear_enable_state(fd);
  log_info("  Radiometry TLinear = %d", rsp);

  // Enable AGC calculations
  cci_set_agc_calc_enable_state(fd, CCI_AGC_ENABLED);
  rsp = cci_get_agc_calc_enable_state(fd);
  log_info("  AGC Calc En = %d", rsp);

  // Enable AGC
  cci_set_agc_enable_state(fd, CCI_AGC_ENABLED);
  rsp = cci_get_agc_enable_state(fd);
  log_info("  AGC = %d", rsp);

  // Close up
  close(fd);
  return 0;
}


/**
 * Read frames from the device into the circular buffer.
 */
void* get_frames_from_device(void* prudev_path_ptr)
{
    char* prudev_path = (char*)prudev_path_ptr;
    int rsp;

    // Declare a static frame to use as a scratch space to avoid locking the framebuffer while
    // we're waiting for a new frame
    vospi_frame_t frame;

    // Open the PRU SPI interface device
    log_info("opening PRU ... %s", prudev_path);
    if ((pru_fd = open(prudev_path, O_RDWR)) < 0) {
      log_fatal("PRU: failed to open device - check permissions & firmware loaded");
      exit(-1);
    }

    // Enable the PRU
    if (write(pru_fd, "1", 2) == 0) {
        log_fatal("PRU: Failed to enable");
        exit(-1);
    }

    // Receive frames forever
    log_info("Starting VoSPI transfers");
    do {

      rsp = sync_and_transfer_frame(pru_fd, &frame);
      if ((rsp == -1) || (rsp == 3)) {
	      log_error("Failed to get frame with error %d", rsp);
	      exit(-1);
      } else if ((rsp == 1) || (rsp == 2)) {
	      log_info("Transfer failed with reason %d", rsp);
      }
      else {
          /* got frame */
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
 * Send frames to the fb as soon as they are ready
 */
void* send_frames_to_fb(void* fbdev_path_ptr)
{
    char* fbdev_path = (char*)fbdev_path_ptr;
    uint8_t pixbuf[VOSPI_FRAME_LEN];

    // Initialize frame buffer
    (void) init_fb(fbdev_path);

    while (1) {
      // Wait if there are no new frames to transmit
      sem_wait(&count_sem);

      // Lock the data structure to prevent new frames being added while we're reading this one
      pthread_mutex_lock(&lock);

      // Copy the next frame out to our local buffer
      frame_to_pixel(frame_buf[reader], pixbuf);

      // Move the reader ahead
      reader = (reader + 1) & (FRAME_BUF_SIZE - 1);

      // Unlock data structure
      pthread_mutex_unlock(&lock);

      // Render it into the frame buffer
      update_fb(pixbuf);
    }
}

/*
 * SIGINT signal handler
 */
void sig_handler(int sig)
{
	// Try to shut down the PRUs before exiting
	(void) write(pru_fd, "0", 2);
	log_info("Shutting down");
	sleep(1); /* make sure command makes it to PRU */
	exit(0);
}


/**
 * Main entry point for PRU-based Lepton FB display
 */
int main(int argc, char *argv[])
{
  pthread_t get_frames_thread, send_frames_to_fb_thread;

  // Set the log level
  log_set_level(LOG_INFO);

  // Setup semaphores
  sem_init(&count_sem, 0, 0);

  // Allocate space to receive the frames in the circular buffer
  log_info("Preallocating space for frames...");
  for (int frame = 0; frame < FRAME_BUF_SIZE; frame ++) {
    frame_buf[frame] = malloc(sizeof(vospi_frame_t));
  }

  // Setup colormap if user has selected a non-default
  if (argc > 1) {
	  set_colormap(atoi(argv[1]));
  }

  // Attempt to initialize the lepton
  if (init_lepton()) {
	  exit(-1);
  }

  // Setup the signal handler
  signal(SIGINT, sig_handler);

  log_info("Creating get_frames_from_device thread");
  if (pthread_create(&get_frames_thread, NULL, get_frames_from_device, pru_dev)) {
    log_fatal("Error creating get_frames_from_device thread");
    return 1;
  }

  log_info("Creating send_frames_to_fb thread");
  if (pthread_create(&send_frames_to_fb_thread, NULL, send_frames_to_fb, fb_dev)) {
    log_fatal("Error creating send_frames_to_fb thread");
    return 1;
  }

  pthread_join(get_frames_thread, NULL);
  pthread_join(send_frames_to_fb_thread, NULL);
}
