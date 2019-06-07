# Linux Utilities for the Beaglebone version of the Pi Platter
The ```talkbp``` (Talk BeagleBone Platter) and ```bpd``` (BeagleBone Platter Daemon) are versions of the ```talkbp``` and ```bpd``` programs originally written for the [Pi Platter](https://github.com/danjulio/rocketblue-automation/tree/master/pi_platter) board but ported for use with the BeagleBone SBCs (primarily designed for the Pocketbeagle).  They are designed to work with the special Beaglebone firmware for the Pi Platter contained in this repository.

Pre-compiled versions built on the Debian 9.5 release.

```Linux beaglebone 4.14.71-ti-r80 #1 SMP PREEMPT Fri Oct 5 23:50:11 UTC 2018 armv7l GNU/Linux```

are included here or they may be compiled on your Beaglebone.

### talkbp
talkbp is a utility program to simplify communicating with the Beaglebone version of the Pi Platter.  It provides a simple command-line interface to allow the user to directly (or via scripts) send commands to the board and to easily manage the Real Time Clock.  

It can communicate with the Pi Platter via either the pseudo-tty ```/dev/bb-platter``` if the bpd daemon is running or the actual hardware serial device associated with the board if bpd is not running.  It uses udev to automatically find the correct serial device for the board, independent of other USB serial devices.

Both the source and a pre-compiled binary are included.  The binary can simply be downloaded and installed in /usr/local/bin.  The source is easily compiled in the directory containing the source file.

    sudo apt-get update
    sudo apt-get install libudev-dev
    gcc -o talkbp talkbp.c -ludev
    sudo mv talkbp /usr/local/bin

talkbp takes the following arguments:

    talkbp [-c <command string>]

          [-s] [-t] [-f]

          [-a <alarm timespec>] [-d <delta seconds>] [-w]

          [-u | -h]


    -c <command string> : send the command string.  Command strings without an "=" character cause the utility to echo back a response.

    -s : Set the Device RTC with the current system clock
 
    -t : Get the time from the Device RTC and display it in a form useful to pass to "date" to set the system clock ("+%m%d%H%M%Y.%S")

    -f : Get the time from the Device RTC and display it in a readable form.

    -a <alarm timespec> : Set the Device wakeup value (does not enable the alarm).  <alarm timespec> is the alarm time in date time format ("+%m%d%H%M%Y.%S")

    -d <delta seconds> : Set the Device wakeup to <delta seconds> past the current Device RTC time value (does not enable the alarm)

    -w : Display the wakeup value in a readable form.

    -u, -h : Usage (and optional help)


Example command to Pi Platter: `talkbp -c B`

Setting the Pi Platter RTC from the Beaglebone's RTC: `talkbp -s`

Setting the Beaglebone's RTC from the Pi Platter (using BASH): `sudo date $(talkbp -t)`

talkbp will echo responses from the Pi Platter to stdout.  It will also echo the last warnings or error messages that has been sent.

### bpd
bpd is a daemon for the Pi Platter.  It provides two main functions.  It will execute a controlled shutdown if the Pi Platter detects a critical battery voltage (and will power-down the entire system after [default] 30 seconds).  Since it opens the serial port associated with the Pi Platter it also provides one or two mechanisms for other applications to communicate with the Pi Platter.  It creates a pseudo-tty device named ```/dev/bb-platter``` which can be used just like the hardware serial port.  It also, optionally, can create a TCP port for applications like telnet to connect to.  

It is important that software not open the hardware serial port, ```/dev/ttyACM<n>```, when bpd is running since it is using the port.

####Command line options
bpd takes the following command line arguments:

    -d : Run as a daemon program (disconnecting from normal IO, etc).  bpd can be run as a traditional process without this argument.

    -p netport : Enable a TCP socket connection on the specified port.  This is required to enable socket communication with bpd.  Exclude this line to only enable /dev/bb-platter as a mechanism to communicate with the Pi Platter.

    -m max-connections : Specify the maximum number of socket connections that can be made to the port specified with -p.  The default is 1.

    -r : Enable auto-restart on charge (set the Pi Platter "C7=1") after critical battery shutdown.

    -x debuglevel : Set the debug level (bpd uses the system logging facility.  0 is default (only log start-up).  Values of 1 - 3 include progressively more information.

    -h : Display usage and command line options.

####Building
Both the source and a pre-compiled binary are included.  The binary can simply be downloaded and installed in /usr/local/bin.  The source is easily compiled in the directory containing the source file.

    sudo apt-get update
    sudo apt-get install libudev-dev
    gcc -o bpd bpd.c -ludev
    sudo mv bpd /usr/local/bin

####Usage
There are many ways to start a daemon, for example a configuration file in / or a script in /etc/init.d.  A very easy way to start it is to include it in /etc/rc.local.  For example, add the following before the "exit 0" at the end of /etc/rc.local (assuming you have placed the bpd executable in /usr/local/bin).

    # Start the Pi Platter Daemon
    sleep 20; /usr/local/bin/bpd -p 23000 -r -d &

This starts bpd with socket communication available on port 23000 and auto-restart in the event of a critical battery shutdown.  I found the sleep to be necessary so that the USB subsystem came up before starting the daemon.

???root crontab 
@reboot /usr/local/bin/ppd -p 23000 -r -d

####telnet example
To connect to the TCP socket using telnet from some other computer:

    telnet <Beaglebone IP Address> 23000

Which should result in something like (user types "B" and then "S"):

    Trying <Beaglebone IP Address>...
    Connected to <Beaglebone IP Address>.
    Escape character is '^]'.
    B
    B=4.03
    S
    S=16

Note that opening the socket without additional precautions may be a security risk (anybody could telnet to your Pi and shut it down).  That's why bpd makes this an option.

### Questions?

Contact the author - dan@danjuliodesigns.com