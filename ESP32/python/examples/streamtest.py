


import time
import base64
import numpy as np
from tcam import TCam
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
                a[r, c] = [255, 255, 255]
            else:
                a[r, c] = [val, val, val]
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
    l1.after(50, update)


########### Main Program ############

if __name__ == '__main__':

    tcam = TCam()
    tcam.connect('192.168.0.200')

    root = Tk()
    root.title('Video in a Frame')
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
        print(ret)

        update()

        root.mainloop()
        print("After mainloop")
        tcam.shutdown()

    except KeyboardInterrupt:
        evt.set()
        root.destroy()
        tcam.shutdown()