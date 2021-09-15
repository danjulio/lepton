#!/usr/bin/env python3
#
# Use the CCI interface to read the RAD Spotmeter Region of Interest (default), print out
# coordinates and then create an array containing a new region of interest and write those
# back to the camera.
#

import argparse
import array
import base64
from tcam import TCam
import sys

parser = argparse.ArgumentParser()

parser.prog = "cci_demo"
parser.description = f"{parser.prog} - an example program using the CCI interface\n"
parser.usage = "cci_demo.py [--ip=<ip address of camera>]"
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
    # Read the RAD Spotmeter Region of Interest (default command 0x4ECC) and then
    # parse the JSON reply which has the form:
    #
    #     {"cci_reg":{"command":20172,"length":4,"status":6,"data":"OwBPADwAUAA="}}
    #
    cci_data = cam.get_lep_cci()
    rsp_vals = cci_data["cci_reg"]
    b64_data = rsp_vals["data"]
    dec_data = base64.b64decode(b64_data)
    reg_array = array.array('H', dec_data)
    print(f"RAD Spotmeter ROI: ({reg_array[0]}, {reg_array[1]}) - ({reg_array[2]}, {reg_array[3]})")

    #
    # Create a new coordinate array (20, 20) - (21, 21), encode it and write the 4 16-bit data
    # words to the camera
    #
    reg_array2 = array.array('H', [20, 20, 21, 21])
    enc_data = reg_array2.tobytes()
    cam.set_lep_cci(command = 0x4ECD, data = enc_data)
    print('Wrote new RAD')
    
    cam.shutdown()
    