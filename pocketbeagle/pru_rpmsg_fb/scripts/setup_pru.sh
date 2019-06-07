#!/bin/bash
#
# Script to start the PRUs running and make /dev/rpmsg_pru31 accessible
# by non-root processes.  This script must be run as root and is designed
# to assist in debugging.
#

# Setup remoteproc devices for user access
chmod 666 /sys/class/remoteproc/remoteproc1/state
chmod 666 /sys/class/remoteproc/remoteproc1/firmware
chmod 666 /sys/class/remoteproc/remoteproc2/state
chmod 666 /sys/class/remoteproc/remoteproc2/firmware

# Load the firmware
echo "am335x-pru0-fw" > /sys/class/remoteproc/remoteproc1/firmware
echo "am335x-pru1-fw" > /sys/class/remoteproc/remoteproc2/firmware

# Start the firmware
echo "start" > /sys/class/remoteproc/remoteproc1/state
echo "start" > /sys/class/remoteproc/remoteproc2/state

# Configure the RPMsg device file for user access 
sleep 1
chmod 666 /dev/rpmsg_pru31

