/*
 * FLIR Lepton 3/3.5 VoSPI interface using VSYNC output
 *
 */
#include "log.h"
#include "vospi.h"
#include "pigpio.h"

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <sys/ioctl.h>


vospi_frame_t* my_frame;   // Pointer to a scratch frame used to collect incoming data
int frame_captured;        // Boolean set when the ISR has a full frame
int line_list[60];         // Used to validate a segment - entry set to 1 when line seen
int bad_segments;          // Used to trigger resync, counts VSYNC interrupts that
                           //   do not contain good segments.

int spiFd;                 // File descriptor for SPI device file
vospi_packet_t lepPacket;  // Current incoming packet



/**
 * Initialise the VoSPI interface. frame points to a scratch buffer for
 * use by this code to store a received frame.  Calling code must initialize
 * the Lepton to output VSYNC pulses.
 */
int vospi_init(int fd, uint32_t speed, vospi_frame_t* frame)
{
  int i;

  // Set the various SPI parameters
  log_debug("setting SPI device mode...");
  uint16_t mode = SPI_MODE_3;
  if (ioctl(fd, SPI_IOC_WR_MODE, &mode) == -1) {
    log_fatal("SPI: failed to set mode");
    return -1;
  }

  log_debug("setting SPI bits/word...");
  uint8_t bits = 8;
  if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1) {
    log_fatal("SPI: failed to set the bits per word option");
    return -1;
  }

  log_debug("setting SPI max clock speed...");
  if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
    log_fatal("SPI: failed to set the max speed option");
    return -1;
  }

  // Setup the vsync interrupt handler
  i = gpioInitialise();
  if (i == PI_INIT_FAILED) {
    log_fatal("gpioInitialize failed: %d", i);
    return -1;
  }
  i = gpioSetISRFunc(4, RISING_EDGE, 20, transfer_segment);
  if (i != 0) {
    log_fatal("gpioSetISRFunc failed: %d", i);
    return -1;
  }

  // Save the SPI file descriptor for use in the ISR
  spiFd = fd;

  // Initialise variables
  my_frame = frame;
  frame_captured = 0;
  bad_segments = 0;

  return 1;
}


/**
 * Main thread access to stored frames.  Main code must copy data from the
 * scratch frame before the next VSYNC (It has about 74 mSec - 2/3 of a frame
 * period - to do so)
 */
int sync_and_transfer_frame()
{
  if (frame_captured) {
    frame_captured = 0;   // Note we've consumed the buffer
    return (1);
  } else {
    return (0);
  }
}


/**
 * VSYNC ISR Handler - reads packets from the lepton until it determines the
 * current set is gargage or discard or until it has a full segment.  Stores
 * in-process frame in local buffer, sets frame_captured to 1 when a valid
 * frame has been read. 
 * 
 */
void transfer_segment(int gpio, int level, uint32_t tick)
{
  uint8_t line, prevLine;
  uint8_t segment;
  int done = 0;
  int beforeValidData = 1;
  static int curSegment = 1;
  static int validSegmentRegion = 0;
  struct timeval startT, curT;

  (void) gettimeofday(&startT, NULL);
  prevLine = 255;

  // Clear our list of valid lines
  init_line_list();

  // Attempt to collect a segment's worth of lines
  while (!done) {
    if (get_packet(&line, &segment)) {
      // Saw a valid packet
      if ((line == prevLine) || (line > 59)) {
        // This is garbage data since line numbers should always increment
        done = 1;
      } else {
        // Check for termination or completion conditions
        if (line == 20) {
          // Check segment
          if (!validSegmentRegion) {
            // Look for start of valid segment data
            if (segment == 1) {
              beforeValidData = 0;
              validSegmentRegion = 1;
            }
          } else if ((segment < 2) || (segment > 4)) {
            // Hold/Reset in starting position (always collecting in segment 1 buffer
            // locations)
            validSegmentRegion = 0;  // In case it was set
            curSegment = 1;
          }
        } 
        
        // Copy the data to the current frame
        //  - beforeValidData is used to collect data before we know if the current segment
        //   (1) is valid
        //  - then we use validSegmentRegion for remaining data once we know we're seeing
        //    valid data
        if ((beforeValidData || validSegmentRegion) && (line <= 59)) {
          // Note line
          line_list[line] = 1;

          // Copy the already flipped id/crc
          my_frame-> segments[curSegment-1].packets[line].id = lepPacket.id;
          my_frame-> segments[curSegment-1].packets[line].crc = lepPacket.crc;
          uint8_t* sPtr = my_frame->segments[curSegment-1].packets[line].symbols;
          for (int i = 0; i < VOSPI_PACKET_SYMBOLS; i++) {
            *sPtr++ = lepPacket.symbols[i];
          }
        }
        
        if (line == 59) {
          // Saw a complete segment, move to next segment or complete frame aquisition
          // if possible
          if (validSegmentRegion && line_list_valid()) {
            if (curSegment < 4) {
              // Setup to get next segment
              curSegment++;
            } else {
              // Note that we got a frame
              frame_captured = 1;
 
              // Setup to get the next frame
              curSegment = 1;
              validSegmentRegion = 0;
            }
            bad_segments = 0;
          }
          done = 1;
        }
      }
      prevLine = line;
    } else {
      (void) gettimeofday(&curT, NULL);
      if (delta_time_usec(startT, curT) > LEP_MAX_FRAME_DELAY_USEC) {
        // Did not see a valid packet within this segment interval
        done = 1;
      }
    }
  }

  // If we haven't gotten a good frame within 12 interrupts then attempt to resync
  // (resyncing requires stopping access to the vospi for at least 185 mSec)
  if (++bad_segments == 12) {
    // Attemp to resync
    log_info("Resync");
    bad_segments = 0;
    isr_sleep_ms(185);
  }
}


/**
 * Transfer a packet
 *  Returns 1 for a non-discard packet with line number and potentially segment number
 */
int get_packet(uint8_t* line, uint8_t* seg)
{
  // Perform the spidev transfer
  if (read(spiFd, &lepPacket, VOSPI_PACKET_BYTES) < 1) {
    log_fatal("SPI: failed to transfer packet");
    return 0;
  }

  // Flip the byte order of the ID & CRC
  lepPacket.id = FLIP_WORD_BYTES(lepPacket.id);
  lepPacket.crc = FLIP_WORD_BYTES(lepPacket.crc);

  if ((lepPacket.id & 0x0f00) == 0x0f00) {
    // Skip discard 
    return 0;
  }

  *line = lepPacket.id & 0x00FF;

  // Get segment when possible
  if (*line == 20) {
    *seg = lepPacket.id >> 12;
  }

  return 1;
}


/**
 * Compute delta-time in uSec from two timeval values
 */
uint32_t delta_time_usec(struct timeval start, struct timeval stop)
{
  return ((stop.tv_sec * 1000000 + stop.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
}


/**
 * Sleep routine designed for use by the ISR
 */
void isr_sleep_ms(int milliseconds)
{
    struct timespec ts;

    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}


/**
 * Initialize (zero) valid seen line list array
 */
void init_line_list()
{
  int i;

  for (i=0; i<60; i++)
    line_list[i] = 0;
}


/**
 * Check the seen line list array
 *   Returns 1 if all 60 line numbers have been seen, 0 otherwise
 */
int line_list_valid()
{
  int i;

  for (i=0; i<60; i++) {
    if (line_list[i] == 0)
      return 0;
  }

  return 1;
}
