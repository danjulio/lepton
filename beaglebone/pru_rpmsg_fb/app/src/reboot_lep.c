#include "cci.h"
#include "log.h"
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>


char i2c_dev[] = "/dev/i2c-2";


int main(int argc, char *argv[])
{
  int fd;

  // Open the I2C device
  log_info("opening I2C device ... %s", i2c_dev);
  if ((fd = open(i2c_dev, O_RDWR)) < 0) {
    log_fatal("I2C: failed to open device - check permissions & i2c enabled");
    return -1;
  }

  // Initialize I2C interface
  cci_init(fd);
  
  // Reboot the Lepton in case it's in a funny state
  log_info("Starting reboot...");
  cc_run_oem_reboot(fd);
  log_info("  Done");

  // Close up
  close(fd);
  return 0;
}
