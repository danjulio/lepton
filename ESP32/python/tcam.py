"""
  tCam Python Package

  Copyright 2021 Dan Julio and Todd LaWall (bitreaper)

  This file is part of tCam.

  tCam is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  tCam is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with tCam.  If not, see <https://www.gnu.org/licenses/>.
"""

import array
import base64
import socket
from queue import Queue
from threading import Thread, Event
import json
from json import JSONDecodeError


class TCamManagerThread(Thread):
    """
    TCamManagerThread - The background thread that manages the socket communication and the three queues.

    Commands come in on the cmdQueue, responses to commands go to the responseQueue, and any frames that
    come from get_image or set_stream_on commands go into frameQueue.

    This thread has a run loop that does 3 things:
     - Reads the command queue and sends any commands it finds
     - Reads the socket for any data
     - Parses responses out of the data

     Instead of a time.sleep() it uses the socket timeout.  This ensures that the socket isn't sitting
     there with data waiting for the time.sleep() call to return before being read.  If there's no data,
     the socket times out and we repeat the cycle.  By checking the cmdQueue on every loop iteration, we
     maintain the ability to be responsive, especially if we are getting a high number of frames while
     streaming.

    """

    def __init__(self, cmdQueue, responseQueue, frameQueue, timeout):
        self.cmdQueue = cmdQueue
        self.responseQueue = responseQueue
        self.frameQueue = frameQueue
        self.timeout = timeout
        self.running = False
        self.event = Event()
        super().__init__()

    def start(self):
        self.running = True
        super().start()

    def stop(self):
        self.running = False
        self.event.set()

    def createSocket(self):
        tmpSock = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)
        tmpSock.settimeout(self.timeout)
        return tmpSock

    def run(self):
        """
        run( )
        Check cmdQueue for any new commands to send down, read the socket for any response coming back, split
        data into responses and deserialize them into python objects from JSON.
        """
        tcamSocket = None
        scratch = b""

        while self.running:
            # The send part of the cycle
            if not self.cmdQueue.empty():
                cmd = self.cmdQueue.get()
                cmdType = cmd.get("cmd")
                if cmdType == "connect":
                    tcamSocket = self.createSocket()
                    try:
                        tcamSocket.connect((cmd["ipaddress"], cmd["port"]))
                    except socket.timeout:
                        self.responseQueue.put({"status": "timeout"})
                    else:
                       self.responseQueue.put({"status": "connected"})
                elif cmdType == "disconnect":
                    tcamSocket.close()
                    tcamSocket = None
                    self.responseQueue.put({"status": "disconnected"})
                    continue
                elif cmdType == "raw":
                    tcamSocket.send(cmd["payload"])
                else:
                    # format the string with the start and stop chars, and encode as a byte string before sending
                    buf = f"\x02{json.dumps(cmd)}\x03".encode()
                    tcamSocket.send(buf)

            # The recv part of the cycle
            if tcamSocket:
                try:
                    rbuf = tcamSocket.recv(65536)
                    scratch += rbuf
                except socket.timeout as e:
                    pass
                scratch = self.findResponses(scratch)
            else:
                # If we're not connected we won't have a socket to timeout on.  Let's use an event to wait on instead.
                # Why event.wait instead of time.sleep?  Because event.wait can be interrupted unlike time.sleep.
                self.event.wait(self.timeout)
                self.event.clear()

    def findResponses(self, buf):
        """
        findResponses()

        This is how the manager thread stitches together packets across reads of the socket.  If you are streaming
        and you have a high enough frame rate, you may end up with more than one response in your buffer.  You may
        also have one stretched across reads.  This function attempts to extract complete ones and returns the
        remainder to be added to by the next read.
        """

        pkts = []
        idx = buf.find(3)
        while idx != -1:
            pkts.append(buf[: idx + 1])
            buf = buf[idx + 1 :]
            idx = buf.find(3)

        for response in pkts:
            try:
                respObj = json.loads(response.strip(b"\x02\x03").decode())
                if "radiometric" in respObj:
                    self.frameQueue.put(respObj)
                else:
                    self.responseQueue.put(respObj)
            except JSONDecodeError:
                respObj = {
                    "error": "malformed json payload, json parser threw exception processing it",
                    "payload": response.decode(),
                }
                self.responseQueue.put(respObj)
        return buf


class TCam:
    """
    TCam - Interface object for managing a tCam device.
    """

    def __init__(self, timeout=1, responseTimeout=10):
        self.frameQueue = Queue()
        self.cmdQueue = Queue()
        self.responseQueue = Queue()
        self.responseTimeout = responseTimeout
        self.timeout = timeout
        self.managerThread = TCamManagerThread(
            responseQueue=self.responseQueue,
            cmdQueue=self.cmdQueue,
            frameQueue=self.frameQueue,
            timeout=self.timeout,
        )
        self.managerThread.start()

    def connect(self, ipaddress="192.168.4.1", port=5001):
        """
        connect()
        """
        cmd = {"cmd": "connect", "ipaddress": ipaddress, "port": port}
        self.cmdQueue.put(cmd)
        return self.responseQueue.get(block=True, timeout=self.responseTimeout)

    def disconnect(self):
        """
        disconnect()
        """
        cmd = {"cmd": "disconnect"}
        self.cmdQueue.put(cmd)
        return self.responseQueue.get(block=True, timeout=self.responseTimeout)

    def shutdown(self):
        """
        shutdown()

        Use this to not only disconnect the socket, but also shut down the manager thread.  If you are using
        this object in an ipython session, you may find that the session won't exit until you ctrl+c.  This is
        because the manager thread is still alive in the background.  Calling stop and join on it will clean it up.
        """
        self.disconnect()
        self.managerThread.stop()
        self.managerThread.join()

    ##########################################################################################
    # Image/sensor array commands
    def start_stream(self, delay_msec=0, num_frames=0, timeout=None):
        if not timeout:
            timeout = self.responseTimeout
        cmd = {
            "cmd": "stream_on",
            "args": {"delay_msec": delay_msec, "num_frames": num_frames},
        }
        self.cmdQueue.put(cmd)
        return self.responseQueue.get(block=True, timeout=timeout)

    def stop_stream(self, timeout=None):
        if not timeout:
            timeout = self.responseTimeout
        cmd = {"cmd": "stream_off"}
        self.cmdQueue.put(cmd)
        return self.responseQueue.get(block=True, timeout=timeout)

    def get_image(self, timeout=None):
        cmd = {"cmd": "get_image"}
        self.cmdQueue.put(cmd)
        if not timeout:
            timeout = self.responseTimeout
        return self.frameQueue.get(block=True, timeout=timeout)

    def get_frame(self):
        if not self.frameQueue.empty():
            return self.frameQueue.get()
        else:
            return None

    def frame_count(self):
        return self.frameQueue.qsize()

    def run_ffc(self, timeout=None):
        if not timeout:
            timeout = self.responseTimeout
        cmd = {"cmd": "run_ffc"}
        self.cmdQueue.put(cmd)
        return self.responseQueue.get(block=True, timeout=timeout)

    ##########################################################################################
    # all of the set and get functions
    def get_status(self, timeout=None):
        if not timeout:
            timeout = self.responseTimeout
        cmd = {"cmd": "get_status"}
        self.cmdQueue.put(cmd)
        return self.responseQueue.get(block=True, timeout=timeout)

    def set_time(
        self,
        hour=None,
        minute=None,
        second=None,
        dow=None,
        day=None,
        month=None,
        year=None,
        timeout=None
    ):
        if not timeout:
            timeout = self.responseTimeout
        cmd = {
            "cmd": "set_time",
            "args": {
                "sec": second,
                "min": minute,
                "hour": hour,
                "dow": dow,
                "day": day,
                "mon": month,
                "year": year,
            },
        }
        self.cmdQueue.put(cmd)
        return self.responseQueue.get(block=True, timeout=timeout)

    def get_config(self, timeout=None):
        if not timeout:
            timeout = self.responseTimeout
        cmd = {"cmd": "get_config"}
        self.cmdQueue.put(cmd)
        return self.responseQueue.get(block=True, timeout=timeout)

    def set_config(self, agc_enabled=1, emissivity=98, gain_mode=2, timeout=None):
        if not timeout:
            timeout = self.responseTimeout
        cmd = {
            "cmd": "set_config",
            "args": {
                "agc_enabled": agc_enabled,
                "emissivity": emissivity,
                "gain_mode": gain_mode,
            },
        }
        self.cmdQueue.put(cmd)
        return self.responseQueue.get(block=True, timeout=timeout)

    def get_lep_cci(self, command=0x4ECC, length=4, timeout=None):
        """
        get_lep_cci()
        
        Default values are Command: RAD Spotmeter Region of Interest, Length: 4 DWORDS
        """
        if not timeout:
            timeout = self.responseTimeout
        cmd = {
            "cmd": "get_lep_cci",
            "args": {
                "command": command,
                "length": length
            }
        }
        self.cmdQueue.put(cmd)
        return self.responseQueue.get(block=True, timeout=timeout)


    def set_lep_cci(self, command, data, timeout=None):
        try:
            dataArray = array.array('H', data)
        except OverflowError as e:
            raise ValueError(f"A value in data list is not within the 0-65535 bounds of a 16 bit UInt. {e}")
        if not timeout:
            timeout = self.responseTimeout
        encodedData = base64.b64encode(dataArray.tobytes()).decode('ascii')
        cmd = {
            "cmd": "set_lep_cci",
            "args": {
                "command": command,
                "length": len(dataArray),
                "data": encodedData
             }
        }
        self.cmdQueue.put(cmd)
        return self.responseQueue.get(block=True, timeout=timeout)

    def set_spotmeter(self, c1=79, c2=80, r1=59, r2=60, timeout=None):
        """
        set_spotmeter()

        Default values are center 4 pixels of the sensor.

        c1 == Spotmeter column 1: Left X-axis spotmeter box coordinate (0-159)
        c2 == Spotmeter column 2: Right X-axis spotmeter box coordinate (0-159)
        r1 == Spotmeter row 1: Top Y-axis spotmeter box coordinate (0-119)
        r2 == Spotmeter row 2: Bottom Y-axis spotmeter box coordinate (0-119)
        """
        if not timeout:
            timeout = self.responseTimeout
        cmd = {"cmd": "set_spotmeter", "args": {"c1": c1, "c2": c2, "r1": r1, "r2": r2}}
        self.cmdQueue.put(cmd)
        return self.responseQueue.get(block=True, timeout=timeout)

    def get_wifi(self, timeout=None):
        """
        get_wifi()
        Returns wifi data
        """
        if not timeout:
            timeout = self.responseTimeout
        cmd = {"cmd": "get_wifi"}
        self.cmdQueue.put(cmd)
        return self.responseQueue.get(block=True, timeout=timeout)

    def set_wifi(
        self,
        ap_ssid="ApSSID",
        ap_pw="ApPassword",
        ap_ip_addr="192.168.4.1",
        flags=145,
        sta_ssid="AHomeNetwork",
        sta_pw="anotherpassword",
        sta_ip_addr="192.168.0.2",
        sta_netmask="255.255.255.0",
        timeout=None
    ):
        """
        set_wifi()
        """
        if not timeout:
            timeout = self.responseTimeout
        cmd = {
            "cmd": "set_wifi",
            "args": {
                "ap_ssid": ap_ssid,
                "ap_pw": ap_pw,
                "ap_ip_addr": ap_ip_addr,
                "flags": flags,
                "sta_ssid": sta_ssid,
                "sta_pw": sta_pw,
                "sta_ip_addr": sta_ip_addr,
                "sta_netmask": sta_netmask,
            },
        }
        self.cmdQueue.put(cmd)
        return self.responseQueue.get(block=True, timeout=timeout)

    def send_raw(self, payload: bytes, timeout=None):
        """
        send_raw() - Don't use.  I know you're curious, but this will likely only jam up comms and not return
        anything useful to you.  Unless you're working on the firmware.

        Sends a raw byte string through the socket.
        Meant as a developer tool to enable writing new command/response pairs in the tCam firmware as well
        as writing regression tests for the firmware.
        """
        if not timeout:
            timeout = self.responseTimeout
        cmd = {"cmd": "raw", "payload": payload}
        self.cmdQueue.put(cmd)
        return self.responseQueue.get(block=True, timeout=timeout)
