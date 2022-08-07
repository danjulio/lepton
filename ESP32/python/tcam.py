"""
  tCam Python Package

  Copyright 2021-2022 Dan Julio and Todd LaWall (bitreaper)

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
import os
import sys
import abc
import json
import array
import base64
import socket
from queue import Queue
from json import JSONDecodeError
from threading import Thread, Event

from fcntl import ioctl
from ioctl_numbers import *


class TCamManagerThreadBase(Thread, metaclass=abc.ABCMeta):
    """
    TCamManagerThreadBase - The background thread that manages the socket communication and the three queues.

    Commands come in on the cmdQueue, responses to commands go to the responseQueue, and any frames that
    come from get_image or set_stream_on commands go into frameQueue.

    For any time.sleep() calls, we should use the Event object's wait() method, because if we have to we can
    wake up the code that is sleeping by setting the event with self.event.set().
    """

    def __init__(self, cmdQueue, responseQueue, frameQueue, timeout):
        self.cmdQueue = cmdQueue
        self.responseQueue = responseQueue
        self.frameQueue = frameQueue
        self.internalQueue = Queue()
        self.timeout = timeout
        self.connected = False
        self.running = False
        self.event = Event()
        super().__init__()

    def start(self):
        self.running = True
        super().start()

    def stop(self):
        self.running = False
        self.event.set()

    def run(self):
        """
        run( )
        Check cmdQueue for any new commands to send down, read the interface for any response coming back, split
        data into responses and deserialize them into python objects from JSON.
        """
        self.interface = None
        scratch = b""

        while self.running:
            # The send part of the cycle
            if not self.cmdQueue.empty():
                cmd = self.cmdQueue.get()
                cmdType = cmd.get("cmd", None)
                if cmdType == "connect":
                    self.open_interface(cmd)
                elif cmdType == "disconnect":
                    self.close_interface()
                    continue
                else:
                    # format the string with the start and stop chars, and encode as a byte string before sending
                    buf = f"\x02{json.dumps(cmd)}\x03".encode()
                    self.write(buf)

            # The recv part of the cycle
            if self.connected:
                scratch += self.read()
                scratch = self.find_responses(scratch)

                # process any items in the internal queue
                while not self.internalQueue.empty():
                    msg = self.internalQueue.get()
                    self.post_process(msg)
            else:
                # If we're not connected we won't have a socket to timeout on.  Let's use an event to wait on instead.
                # Why event.wait instead of time.sleep?  Because event.wait can be interrupted unlike time.sleep.
                self.event.wait(self.timeout)
                self.event.clear()

    def find_responses(self, buf):
        """
        find_responses()

        This is how the manager thread stitches together packets across reads of the interface.  If you are streaming
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
                self.internalQueue.put(respObj)
            except JSONDecodeError:
                respObj = {
                    "error": "malformed json payload, json parser threw exception processing it",
                    "payload": response.decode(),
                }
                self.responseQueue.put(respObj)
        return buf

    @abc.abstractmethod
    def open_interface(self, cmd):
        '''
        open_interface()
        
        How the particular type of manager will open it's interface(s).
        '''
        pass

    @abc.abstractmethod
    def close_interface(self):
        '''
        close_interface()
        
        How the particular type of manager will close it's interface(s)
        '''
        pass
    
    @abc.abstractmethod
    def read(self):
        '''
        read()

        How the interface(s) will be read.  Allows abstraction from the base code
        as to how the interfaces need to be read.  Sockets use recv(), serial uses read().
        '''
        pass
    
    @abc.abstractmethod
    def write(self):
        '''
        write()

        How the interface(s) will be written to.  Allows abstraction from the base code
        as to how the interfaces need to be written.  Sockets use send(), serial uses write().
        '''
        pass
    
    @abc.abstractmethod
    def post_process(self):
        '''
        post_process()

        How the messages in the internalQueue will be handled.  This is how the 
        image frames are put on the frameQueue.
        '''
        pass
    
    
class TCamManagerThread(TCamManagerThreadBase):
    """
    TCamManagerThread - The background thread that manages the socket communication and the three queues.

    """

    def open_interface(self, cmd):
        tmpSock = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)
        tmpSock.settimeout(self.timeout)
        try:
            tmpSock.connect((cmd["ipaddress"], cmd["port"]))
        except OSError as e:
            self.responseQueue.put({"status": "disconnected", "message": f"{e}"})
            self.tcamSocket = None
        except socket.timeout:
            self.responseQueue.put({"status": "disconnected", "message": "timeout"})
            self.tcamSocket = None
        except ConnectionRefusedError as e:
            self.responseQueue.put({"status": "disconnected", "message": f"{e}"})
            self.tcamSocket = None
        else:
            self.responseQueue.put({"status": "connected"})
        self.tcamSocket = tmpSock
        self.connected = True
        return

    def close_interface(self):
        self.responseQueue.put({"status": "disconnected"})
        if hasattr(self, 'tcamSocket'):
            # handle the case of a shutdown before it gets used, otherwise this becomes an execption in a background thread.
            self.tcamSocket.close()
        self.tcamSocket = None
        self.connected = False

    def read(self):
        rbuf = b''
        try:
            rbuf = self.tcamSocket.recv(65536)
        except socket.timeout as e:
            pass
        return rbuf

    def write(self, buf):
        if not hasattr(self, 'tcamSocket'):
            self.responseQueue.put({"status": "disconnected", "msg":"Please call connect() first, refusing to write to empty interface."})
            self.frameQueue.put({"status": "disconnected", "msg":"Please call connect() first, refusing to write to empty interface."})
        else:
            self.tcamSocket.send(buf)

    def post_process(self, msg):
        if "radiometric" in msg:
            self.frameQueue.put(msg)
        else:
            self.responseQueue.put(msg)


################################################################################
class TCamHwManagerThread(TCamManagerThreadBase):
    """
    TCamHwManagerThread - The background thread for the hardware interfaced version of tCam-mini
    """
    MODE = SPI_MODE_3  # this comes from ioctl_numbers.py
    BITS = 8

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        from serial import Serial
        self.SerialClass = Serial

    def open_interface(self, data):
        try: 
            self.serial = self.SerialClass(data['serialFile'], baudrate=data['baudrate'], timeout=.1)
            self.spi = open(data['spiFile'], "rb", buffering=0)
            ioctl(self.spi, SPI_IOC_WR_MODE, struct.pack("=B", self.MODE))
            ioctl(self.spi, SPI_IOC_WR_BITS_PER_WORD, struct.pack("=B", self.BITS))
            ioctl(self.spi, SPI_IOC_WR_MAX_SPEED_HZ, struct.pack("=I", data['spiSpeed']))
        except Exception as e:
            self.responseQueue.put({"status": "disconnected", "message": f"{e}"})
            self.connected = False
            return
        self.connected = True
        self.responseQueue.put({"status": "connected"})

        
    def close_interface(self):
        if hasattr(self, 'serial'):
            # handle the case of a shutdown before it gets used, otherwise this becomes an execption in a background thread.
            self.serial.close()
            self.spi.close()
        self.serial = None
        self.spi = None
        self.connected = False
        self.responseQueue.put({"status": "disconnected"})

        
    def read(self):
        return self.serial.read(256)
    

    def write(self, buf):
        if not hasattr(self, 'serial'):
            self.responseQueue.put({"status": "disconnected", "msg":"Please call connect() first, refusing to write to empty interface."})
            self.frameQueue.put({"status": "disconnected", "msg":"Please call connect() first, refusing to write to empty interface."})
        else:
            self.serial.write(buf)
            self.event.wait(.1)

        
    def post_process(self, msg):
        if "image_ready" in msg:
            self.frameQueue.put(self.get_spi_frame(msg['image_ready']))
        else:
            self.responseQueue.put(msg)

            
    def get_spi_frame(self, frameLength):
        frame = self.spi.read(frameLength)
        cs = int.from_bytes(frame[-4:], 'big')
        sum = 0
        for i in frame[:-4]:
            sum += i
        if sum != cs:
            # if a bogus frame comes in, since this is a thread and not the main thread, we need
            # to signal that it was bad, but we also want to put the bogus data on the frameQueue
            # so that we can debug what happened.
            self.responseQueue.put({"status": f"Bad frame! Sums don't match: Frame:{cs} Calc:{sum}"})
            return frame
        frameObj = json.loads(frame[1:-5].decode())
        return frameObj



################################################################################
class TCam:
    """
    TCam - Interface object for managing a tCam device.
    """

    def __init__(self, timeout=1, responseTimeout=10, is_hw=False):
        self.frameQueue = Queue()
        self.cmdQueue = Queue()
        self.responseQueue = Queue()
        self.responseTimeout = responseTimeout
        self.timeout = timeout
        self.is_hw = is_hw

        if is_hw:
            self.hwChecks()
            self.managerThread = TCamHwManagerThread(
                responseQueue=self.responseQueue,
                cmdQueue=self.cmdQueue,
                frameQueue=self.frameQueue,
                timeout=self.timeout,
            )
        else:
            self.managerThread = TCamManagerThread(
                responseQueue=self.responseQueue,
                cmdQueue=self.cmdQueue,
                frameQueue=self.frameQueue,
                timeout=self.timeout,
            )

        self.managerThread.start()
        

    def hwChecks(self):
        try:
            from serial import Serial
        except ImportError as e:
            print("Attempting to use hardware interface without the pyserial module installed!")
            sys.exit(-42)
        if not os.path.exists('/dev/spidev0.0') or not os.path.exists('/dev/spidev0.1'):
            print("Do you have SPI turned on?  Didn't find the SPI device files in /dev")
            sys.exit(-43)
        if not os.path.exists('/dev/serial0'):
            print("Do you have the UART turned on?  Didn't find the serial device file in /dev")
            sys.exit(-44)
        with open('/proc/cmdline', 'r') as f:
            cmdline = f.read()
            if 'spidev.bufsiz=65536' not in cmdline:
                print(f"You will need to add 'spidev.bufsiz=65536' to the kernel cmdline in /boot/cmdline.txt")
                sys.exit(-45)
                
            
    def connect(self, ipaddress="192.168.4.1", port=5001,
                spiFile='/dev/spidev0.0',
                serialFile='/dev/serial0',
                baudrate=230400,
                serialTimeout=.01,
                spiSpeed=7000000
                ):
        """
        connect()
        """
        if self.is_hw:
            cmd = {"cmd": "connect",
                   "spiFile": spiFile,
                   "serialFile": serialFile,
                   "baudrate": baudrate,
                   "timeout": serialTimeout,
                   "spiSpeed": spiSpeed,
                   }
        else:
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
        """get_image()
        Used for when you are needing only one frame.
        """
        cmd = {"cmd": "get_image"}
        self.cmdQueue.put(cmd)
        if not timeout:
            timeout = self.responseTimeout
        return self.frameQueue.get(block=True, timeout=timeout)

    def get_frame(self):
        """
        get_frame()
        Used for when you need to pull frames from the frameQueue, usually when you are streaming.
        """
        if not self.frameQueue.empty():
            return self.frameQueue.get()
        else:
            return None

    def frame_count(self):
        """
        frame_count()
        Returns number of pending frames waiting in the queue
        """
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

    def set_config_agc(self, agc_enabled=1, timeout=None):
        if not timeout:
            timeout = self.responseTimeout
        cmd = {
            "cmd": "set_config",
            "args": {
                "agc_enabled": agc_enabled,
            },
        }
        self.cmdQueue.put(cmd)
        return self.responseQueue.get(block=True, timeout=timeout)

    def set_config_emissivity(self, emissivity=98, timeout=None):
        if not timeout:
            timeout = self.responseTimeout
        cmd = {
            "cmd": "set_config",
            "args": {
                "emissivity": emissivity,
            },
        }
        self.cmdQueue.put(cmd)
        return self.responseQueue.get(block=True, timeout=timeout)

    def set_config_gain_mode(self, gain_mode=2, timeout=None):
        if not timeout:
            timeout = self.responseTimeout
        cmd = {
            "cmd": "set_config",
            "args": {
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
        set_wifi() - Deprecated.  Use set_wifi_ap, set_wifi_sta or set_network instead.
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

    def set_wifi_ap(self, ap_ssid, ap_pw, timeout=None):
        """
        set_wifi_ap()
        
        Configure the camera as a WiFi access point.  Note that the camera will be
        disconnected.  You should call disconnect() after issuing this call.
        """
        if not timeout:
            timeout = self.responseTimeout
        cmd = {
            "cmd": "set_wifi",
            "args": {
                "ap_ssid": ap_ssid,
                "ap_pw": ap_pw,
                "flags": 1,
            },
        }
        self.cmdQueue.put(cmd)

    def set_wifi_sta(
        self,
        sta_ssid,
        sta_pw,
        is_static=False,
        sta_ip_addr="192.168.0.2",
        sta_ip_netmask="255.255.255.0",
        timeout=None
    ):
        """
        set_wifi_sta()
        
        Configure the camera as a WiFi client.  Note that the camera will be
        disconnected.  You should call disconnect() after issuing this call.
        Set is_static to True and include sta_ip_addr and sta_ip_netmask to configure
        a static IP address.  Setting is_static to False configured a DHCP served
        address (and doesn't require the stat_ip_addr or sta_ip_netmask arguments).
        """
        if not timeout:
            timeout = self.responseTimeout
        if is_static:
            cmd = {
                "cmd": "set_wifi",
                "args": {
                    "sta_ssid": sta_ssid,
                    "sta_pw": sta_pw,
                    "flags": 145,
                    "sta_ip_addr": sta_ip_addr,
                    "sta_netmask": sta_ip_netmask,
                },
            }
        else:
            cmd = {
                "cmd": "set_wifi",
                "args": {
                    "sta_ssid": sta_ssid,
                    "sta_pw": sta_pw,
                    "flags": 129,
                },
            }
        self.cmdQueue.put(cmd)

    def set_static_ip(
        self,
        is_static=False,
        sta_ip_addr="192.168.0.2",
        sta_ip_netmask="255.255.255.0",
        timeout=None
    ):
        """
        set_static_ip()
        
        Configure the camera's network interface with a static IP address or to receive
        a DHCP served address.  Should only be sent to a WiFi connected camera when it
        is configured as in STA mode (client) or an Ethernet connected camera.  Note that
        the camera will be disconnected.  You should call disconnect() after issuing this call.
        Set is_static to True and include sta_ip_addr and sta_ip_netmask to configure
        a static IP address.  Setting is_static to False configured a DHCP served
        address (and doesn't require the stat_ip_addr or sta_ip_netmask arguments).
        """
        if not timeout:
            timeout = self.responseTimeout
        if is_static:
            cmd = {
                "cmd": "set_wifi",
                "args": {
                    "flags": 145,
                    "sta_ip_addr": sta_ip_addr,
                    "sta_netmask": sta_ip_netmask,
                },
            }
        else:
            cmd = {
                "cmd": "set_wifi",
                "args": {
                    "flags": 129,
                },
            }
        self.cmdQueue.put(cmd)

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
