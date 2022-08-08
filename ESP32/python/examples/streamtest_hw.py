#!/usr/bin/env python3

'''
streamtest_hw.py is a simple utility to demonstrate streaming the frames from a tCam-Mini
using the hardware interface to a canvas real time.  The only change from the original 
streamtest.py demo is to specify the hardware interface when the driver is instantiated. 

original author: bitreaper (hacked to use the hw interface by Dan Julio)
'''

import base64
import argparse
import numpy as np
from tcam import TCam
from palettes import rainbow_palette
from tkinter import *
from array import array
from PIL import Image, ImageTk
from threading import Event

def convert(img):

    dimg = base64.b64decode(img["radiometric"])
    ra = array('H', dimg)

    imgmin = 65535
    imgmax = 0
    for i in ra:
        if i < imgmin:
            imgmin = i
        if i > imgmax:
            imgmax = i
    delta = imgmax - imgmin
    a = np.zeros((120,160,3), np.uint8)
    for r in range(0, 120):
        for c in range(0, 160):
            val = int((ra[(r * 160) + c] - imgmin) * 255 / delta)
            if val > 255:
                a[r, c] = rainbow_palette[255]
            else:
                a[r, c] = rainbow_palette[val]
    return a

def update():
    if tcam.frameQueue.empty():
        evt.wait(.05)
    else:
        image = convert(tcam.get_frame())
        frame_image = Image.fromarray(image)
        frame_image = ImageTk.PhotoImage(frame_image)
        l1.config(image=frame_image)
        l1.image = frame_image
    l1.after(10, update)


########### Main Program ############

if __name__ == '__main__':
    #
    # Instantiate the driver and configure it to use the hardware interface
    #
    tcam = TCam(is_hw=True)

    #
    # Connect to the camera using the default serial and SPI ports (/dev/serial0 and
    # /dev/spidev0.0 respectively).  To use alternate ports, specify them as shown
    # below:
    #         tcam.connect(spiFile='/dev/spidev0.1', serialFile='/dev/serial1')
    #
    tcam.connect()

    root = Tk()
    root.title('tCam-Mini Video in a Frame')
    f1 = Frame()
    l1 = Label(f1)
    l1.pack()
    f1.pack()

    evt = Event()

    try:
        # prime the frame with the first image, avoids a strange shaped window showing up due to drawing the
        # first frame before an image is in the queue
        image = convert(tcam.get_image())
        frame_image = Image.fromarray(image)
        frame_image = ImageTk.PhotoImage(frame_image)
        l1.config(image=frame_image)
        l1.image = frame_image

        ret = tcam.start_stream()

        update()

        root.mainloop()
        tcam.shutdown()

    except KeyboardInterrupt:
        evt.set()
        root.destroy()
        tcam.shutdown()
