#!/usr/bin/env python3
#
# Print spot meter value
#

import argparse
import array
import base64
from tcam import TCam
import sys

parser = argparse.ArgumentParser()

parser.prog = "disp_spot"
parser.description = f"{parser.prog} - an example program to print the spotmeter value from the CCI interface\n"
parser.usage = "disp_spot.py [--ip=<ip address of camera>]"
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
    # Connect to tCam
    #
    cam = TCam()
    stat = cam.connect(ip)
    if stat["status"] != "connected":
        print(f"Could not connect to {ip}")
        cam.shutdown()
        sys.exit()

    #
    # OEM Mask 
    #
    COMMAND_OEM_MASK = 0x4000

    #
    # Request the RAD T-Linear resolution (RAD 0x0EC4)
    #    Response is 0 for 0.1 C (Low Gain), 1 for 0.01 C (High Gain)
    # 
    rsp = cam.get_lep_cci(COMMAND_OEM_MASK | 0x0EC4, 2)

    #
    # Convert the json response into an array of 2 16-bit words
    #  Index  : Value
    #    0    : Response[15:0]
    #    1    : Response[31:16]
    #
    rsp_vals = rsp["cci_reg"]
    dec_data = base64.b64decode(rsp_vals["data"])
    reg_array = array.array('H', dec_data)
    if reg_array[0] == 0:
        res = 0.1
    else:
        res = 0.01

    print(f"T-Linear resolution = {res}")
    
    #
    # Request the RAD Spotmeter Value (RAD 0xED0)
    #
    rsp = cam.get_lep_cci(COMMAND_OEM_MASK | 0x0ED0, 4)

    #
    # Convert the json response into an array of 4 16-bit words
    #  Index  : Value
    #    0    : Spotmeter Value
    #    1    : Spotmeter Max Value
    #    2    : Spotmeter Min Value
    #    3    : Spotmeter Population
    #
    rsp_vals = rsp["cci_reg"]
    dec_data = base64.b64decode(rsp_vals["data"])
    reg_array = array.array('H', dec_data)
    
    #
    # Convert the Spotmeter Value into degrees C
    #   Temp = (Spotmeter Value / (1 / T-Linear Resolution)) - 273.15
    temp = (reg_array[0] / (1 / res)) - 273.15
    print(f"spot average = {temp} C")

    cam.shutdown()

