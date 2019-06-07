## FLIR Lepton Code

This repository contains code, libraries and hardware I developed over the course of making a thermal imaging camera based on the FLIR Lepton 3.5 [module](https://store.groupgets.com/collections/flir-lepton-accessories/products/flir-lepton-breakout-board).

The project process is documented at [hackaday.io](https://hackaday.io/project/159615-lepton-35-thermal-imaging-camera).

### beaglebone
Code for the Beaglebone Black including my initial PRU-based VoSPI video pipleline and LCD display.

![Beaglebone Black Prototype](beaglebone/pictures/pru_rpmsg_fb.png)

### pocketbeagle
The pocketbeagle was used for the final design of a thermal imaging camera.  This directory contains the code supporting the camera and the re-targeting of my Solar Pi Platter as a power-management and expansion board for the Pocketbeagle.

![Pocketbeagle Thermal Imaging Camera](pocketbeagle/pictures/boxy_pb_camera.png)

### raspberrypi
Contains a modified version of Damien Walsh's great [leptonic](https://github.com/themainframe/leptonic) program running on the Raspberry Pi.  My version uses the VSYNC output from the Lepton to synchronize the VoSPI transfer as an experiment to increase the reliability of syncing a user-space process to the Lepton video stream.

![Raspberry Pi Prototype](raspberrypi/pictures/pi_lepton.png)

### teensy3
Contains the code I wrote initially for a test platform based on the PJRC Teensy 3.2 board to learn about the Lepton.  Also includes a port of FLIR's LeptonSDKEmb32OEM CCI to the Arduino platform.

![Teensy 3 Cam](teensy3/pictures/display_pi_rainbow.png)