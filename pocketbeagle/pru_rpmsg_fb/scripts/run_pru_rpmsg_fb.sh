#!/bin/bash
#
# Script to start pru_rpmsg_fb from /etc/rc.local on boot
#

# Sleep to let system finish coming up
sleep 40

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
sleep 2
chmod 666 /dev/rpmsg_pru31

# Finally start the application
#/home/debian/pru_rpmsg_fb/app/reboot_lep
/home/debian/pru_rpmsg_fb/app/pru_rpmsg_fb 3

