#!/bin/bash
#
# Script to set the 
#
# Execute using sudo.  You should have first started the PRUs running using the
# start_pru.sh script.
#

# Configure the RPMsg device file for user access 
chmod 666 /dev/rpmsg_pru31

