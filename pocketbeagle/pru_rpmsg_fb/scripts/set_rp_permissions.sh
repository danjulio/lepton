#!/bin/bash
#
# Script to set the permissions for user-level access to the PRU remoteproc
# entries.
#
# Execute using sudo
#

# Setup remoteproc devices for user access
chmod 666 /sys/class/remoteproc/remoteproc1/state
chmod 666 /sys/class/remoteproc/remoteproc1/firmware
chmod 666 /sys/class/remoteproc/remoteproc2/state
chmod 666 /sys/class/remoteproc/remoteproc2/firmware
