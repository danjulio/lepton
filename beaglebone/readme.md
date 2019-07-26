# Beaglebone Black Code

This directory contains code and libraries developed for the Beaglebone Black board.

## pru\_rpmsg\_fb
![Beaglebone Black Thermal Imaging Camera](pictures/pru_rpmsg_fb.png)

The first code I got running on the Beaglebone Black using the two PRU co-processors to handle the Lepton's VoSPI stream and display the image on a linuxfb device driving a ubiquitous ILI9341-based 320x240 pixel LCD display.

![PRU Pipeline to LCD Display](pictures/pru_rpmsg_fb_dataflow.png)

## littlevgl\_devicetree
The device tree files and connection diagram to run the littlevgl GUI on the LCD display attached to a Beaglebone Black.