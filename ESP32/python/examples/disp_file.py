#!/usr/bin/env python3
#
# Simple tCam file display program using PIL
#

import base64
from palettes import ironblack_palette
from PIL import Image as im
import json
import numpy as np
import argparse
import sys

parser = argparse.ArgumentParser()

parser.prog = "disp_file"
parser.description = f"{parser.prog} - an example program to display image data from a tCam file using PIL\n"
parser.usage = "disp_file.py --file=<tjsn file>"
parser.add_argument("-f", "--file", help="tCam radiometric image data file")


if __name__ == "__main__":

    args = parser.parse_args()

    if not args.file:
        print("A file name is necessary.")
        sys.exit(-1)

    print(f"Reading {args.file}")
    with open(args.file, 'r') as f:
        json_str = f.read()

    print("Getting radiometric data")
    img = json.loads(json_str)
    dimg = base64.b64decode(img["radiometric"])
    mv = memoryview(dimg).cast("H")

    print("Computing image min/max for mapping to palette")
    imgmin = 65535
    imgmax = 0

    for i in mv.tolist():
        if i < imgmin:
            imgmin = i
        if i > imgmax:
            imgmax = i

    delta = imgmax - imgmin
    print(f"Max val is {imgmax}, Min val is {imgmin}, Delta is {delta}")

    # now form the 8 bit range from the min and delta.  This allows us to bracket the 16 bit data into
    # an 8 bit range, and then use the 8 bits to look up the palette data for the pixel value.
    transformed = []
    for i in mv:
        val = int((i - imgmin) * 255 / delta)

        if val > 255:
            transformed.append(ironblack_palette[255])
        else:
            transformed.append(ironblack_palette[val])

    # Convert to an image
    print("Displaying image")
    a = np.zeros((120, 160, 3), np.uint8)
    for r in range(0, 120):
        for c in range(0, 160):
            a[r, c] = transformed[(r * 160) + c]

    data = im.fromarray(a, "RGB")
    data.show()

