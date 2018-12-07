#!/bin/bash
#
# Script to start the PRUs running the existing /lib/firmware/am335x-pru0-fw and
# /lib/firmware/am335x-pru1-fw firmware files.
#
# This may be run as a normal user.  However you should have run the set_rp_permissions.sh
# script first using sudo.
#

# Load the firmware
echo "stop" > /sys/class/remoteproc/remoteproc1/state
echo "stop" > /sys/class/remoteproc/remoteproc2/state
echo "am335x-pru0-fw" > /sys/class/remoteproc/remoteproc1/firmware
echo "am335x-pru1-fw" > /sys/class/remoteproc/remoteproc2/firmware

# Start the firmware
echo "start" > /sys/class/remoteproc/remoteproc1/state
echo "start" > /sys/class/remoteproc/remoteproc2/state

