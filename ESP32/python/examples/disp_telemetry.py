#!/usr/bin/env python3
#
# Get an image and decode and print out example data from the Lepton telemetry packet
#

import argparse
import array
import base64
from tcam import TCam
import sys

parser = argparse.ArgumentParser()

parser.prog = "disp_telemetry"
parser.description = f"{parser.prog} - an example program to dump telemetry data from tCam\n"
parser.usage = "disp_telemetry.py [--ip=<ip address of camera>]"
parser.add_argument("-i", "--ip", help="IP address of the camera")


if __name__ == "__main__":

    #
    # Parse command line
    #
    args = parser.parse_args()

    if not args.ip:
        ip = "192.168.4.1"
    else:
        ip = args.ip

    #
    # Open camera
    #
    cam = TCam()
    stat = cam.connect(ip)
    if stat["status"] != "connected":
        print(f"Could not connect to {ip}")
        cam.shutdown()
        sys.exit()

    #
    # Get image and decode telemetry into an array of uint16 values
    #
    img_json = cam.get_image()
    dec_tel = base64.b64decode(img_json["telemetry"])
    ra = array.array('H', dec_tel)

    #
    # Shutdown the camera driver
    #
    cam.shutdown()

    #
    # Lepton telemetry is comprised of 3 telemetry rows: A, B, and C.  Each
    # row is 80 16-bit words long.  Some data from each row is decoded and
    # displayed to demonstrate how to access the combined decoded array.
    # See the Lepton Engineering Datasheet for a complete list of telemetry
    # values.
    #
    print(f"Telemetry Array length = {len(ra)} words")

    #
    # Row A - offset 0 in the register array
    #   Word 24 : FPA Temperature (Kelvin x 100)
    #   Word 26 : Housing Temperature (Kelvin x 100)
    #
    t = (ra[24] / 100) - 273.15
    print(f"  FPA Temp      = {t} C")
    t = (ra[26] / 100) - 273.15
    print(f"  Housing Temp  = {t} C")

    #
    # Row B - offset 80
    #   Word 19 : Emissivity (scaled by 8192)
    #
    e = ra[99] / 8192
    print(f"  Emissivity    = {e}")

    #
    # Row C - offset 160
    #   Word  5 : Gain Mode (0 = High, 1 = Low, 2 = Auto)
    #   Word  6 : Effective Gain Mode (0 = High, 1 = Low)
    #   Word 48 : TLinear Enable State (0 = disabled, 1 = enabled)
    #   Word 49 : T-Linear resolution (0 = 0.1, 1 = 0.01)
    #   Word 50 : Spotmeter mean value (Kelvin scaled by T-Linear resolution)
    #
    print(f"  Gain Mode     = {ra[165]}")
    print(f"  Eff Gain Mode = {ra[166]}")
    print(f"  TLinear Mode  = {ra[208]}")
    if ra[209] == 0:
        res = 0.1
    else:
        res = 0.01
    print(f"  TLinear Res   = {res}")
    t = (ra[210] / (1 / res))  - 273.15
    print(f"  Spotmeter     = {t} C")
  
