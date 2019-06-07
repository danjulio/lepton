/*
 * bpd - Beaglebone version of the Pi Platter daemon
 *
 * Connects to the Pi Platter serial port and provides the following functions:
 *   1. Automatic controlled shutdown on critical battery warning with optional
 *      restart on charge.
 *   2. A new pseudo-tty called /dev/bb-platter for applications to connect too
 *   3. An optional port for direct TCP connection by applications
 *
 * Parts of this code are thanks to Paul Davis' remserial code.
 *
 * Build: gcc -o bpd bpd.c -ludev
 *  (install libudev-dev first)
 *
 * Copyright (c) 2016-2019 Dan Julio, dan@danjuliodesigns.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include <stdio.h>
#define __USE_XOPEN_EXTENDED  /* Needed for grantpt and unlockpt */
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <libudev.h>
#include <locale.h>
#include <termios.h>

#define MAX_SER_NAME_CHARS  64

#define VERSION_MAJOR       0
#define VERSION_MINOR       1


char serName[MAX_SER_NAME_CHARS];
int serialFd;
struct sockaddr_in addr,remoteaddr;
int sockfd = -1;
int port = 0;
int debug=0;
int linkFd;
int *remotefd;
char *linkname = "/dev/bb-platter";
int isdaemon = 0;
int restartEnable = 0;
int warnMsgIndex = 0;
int sawWarnCritical = 0;
int sawWarnLow = 0;
fd_set fdsread,fdsreaduse;
int curConnects = 0;
int sendMask = 0x01;

extern char* ptsname(int fd);


int FindPiPlatter(char* deviceFilePath)
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    int success = -1;

    // Create the udev object
    udev = udev_new();
    if (!udev) {
	syslog(LOG_ERR, "Can't create udev");
        exit(1);
    }

    // Create a list of the devices in the 'tty' subsystem and search for our identifiers
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "tty");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    // For each item enumerated, parse its information.  udev_list_entry_foreach is a 
    // macro which expands to a loop. The loop will be executed for each member in devices,
    // setting dev_list_entry to a list entry which contains the device's path in /sys.
    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;
        const char *devpath;

        // Get the filename of the /sys entry for the device and create a udev_device
        // object (dev) representing it
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        // Get the device node itself in /dev
        devpath = udev_device_get_devnode(dev);

        // The device pointed to by dev contains information about the tty device.
        // In order to get information about the USB device, get the parent device
        // with the subsystem/devtype pair of "usb"/"usb_device".  This will be
        // several levels up the tree, but the function will find it.
        dev = udev_device_get_parent_with_subsystem_devtype(
               dev,
               "usb",
               "usb_device");

        // Most tty devices aren't associated with the USB subsystem so we just skip
        // look at them
        if (dev) {
            if ((strcmp(udev_device_get_sysattr_value(dev, "idVendor"), "04d8") == 0) &&
                (strcmp(udev_device_get_sysattr_value(dev, "idProduct"), "f19c") == 0) &&
                (strcmp(udev_device_get_sysattr_value(dev, "product"), "Beagle Power") == 0)) {

                // Found our board
                strncpy(deviceFilePath, devpath, MAX_SER_NAME_CHARS-1);
                success = 0;
                break;
            }
        }
    }

    // Free the enumerator object
    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    if (success == -1) {
	syslog(LOG_ERR, "Could not find Pi Platter");
    }
    return(success);
}


int OpenSerialPort(const char *deviceFilePath)
{
    int fd = -1;
    struct termios options;

    // Attempt to open
    fd = open(deviceFilePath, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1)
    {
	syslog(LOG_ERR, "Error opening serial port %s - %m", deviceFilePath);
        close(fd);
        return(-1);
    }

    // Configure options
    options.c_cflag = CS8 | CLOCAL | CREAD;
    options.c_iflag = 0;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcsetattr(fd,TCSANOW,&options);

    // success
    return fd;
}


void CloseSerialPort(int fd)
{
    // Block until all written output has been sent from the device.
    if (tcdrain(fd) == -1)
    {
	syslog(LOG_ERR, "Error waiting for drain");
    }   

    close(fd);
}

    
// Looks for "WARN 0" (batt low) or "WARN 1" (batt critical)
int ProcessResponse(char* strP, int strLen)
{
	char c;

	sawWarnLow = 0;
	sawWarnCritical = 0;
	while (strLen--) {
		c = *strP++;
		switch (warnMsgIndex) {
			case 0: warnMsgIndex = (c == 'W') ? warnMsgIndex+1 : 0; break;
			case 1: warnMsgIndex = (c == 'A') ? warnMsgIndex+1 : 0; break;
			case 2: warnMsgIndex = (c == 'R') ? warnMsgIndex+1 : 0; break;
			case 3: warnMsgIndex = (c == 'N') ? warnMsgIndex+1 : 0; break;
			case 4: warnMsgIndex = (c == ' ') ? warnMsgIndex+1 : 0; break;
			case 5: 
				warnMsgIndex = 0;
				if (c == '0') {
					sawWarnLow = 1;
					return 1;
				} else if (c == '1') {
					sawWarnCritical = 1;
					return 1;
				}
				break;
			default: warnMsgIndex = 0;
		}
	}

	return 0;
}


void SigHandler(int sig)
{
	int i;

	if ( sockfd != -1 )
		close(sockfd);
	for (i=0 ; i<curConnects ; i++)
		close(remotefd[i]);
	if ( serialFd != -1)
		close(serialFd);
	if ( linkFd != -1 )
		close(linkFd);
	if (linkname)
		unlink(linkname);
	syslog(LOG_NOTICE, "Terminating on signal %d",sig);
	exit(0);
}


void LinkSlave(int fd)
{
	char *slavename;
	int status = grantpt(linkFd);
	if (status != -1)
		status = unlockpt(linkFd);
	if (status != -1) {
		slavename = ptsname(linkFd);
		if (slavename) {
			// Safety first
			unlink(linkname);
			status = symlink(slavename, linkname);
			status = chmod(linkname, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
			if (status == -1) {
				syslog(LOG_ERR, "Cannot chmod link for pseudo-tty: %m");
				exit(1);
			}
		}
		else
			status = -1;
	}
	if (status == -1) {
		syslog(LOG_ERR, "Cannot create link for pseudo-tty: %m");
		exit(1);
	}
}


void Usage(char *progname) {
	printf("bpd version %0d.%0d.  Usage:\n", VERSION_MAJOR, VERSION_MINOR);
	printf("bpd [-d] [-p netport] [-m maxconnect] [-x debuglevel] [-h]\n\n");

	printf("-d			Run as a daemon program\n");
	printf("-p netport		Enable socket connection on IP port#\n");
	printf("-m max-connections	Maximum number of simultaneous client connections to allow\n");
	printf("			 (only applies if -p is also specified)\n");
	printf("-r                      Enable auto-restart after critical battery shutdown\n");
	printf("-x debuglevel		Set debug level, 0 is default, 1-3 give more info\n");
	printf("-h                      Usage\n");
}


int main(int argc, char *argv[])
{
	int result;
	extern char *optarg;
	extern int optind;
	int maxfd = -1;
	char devbuf[512];
	int devbytes;
	int remoteaddrlen;
	int c;
	int maxConnects = 1;
	register int i;

	while ( (c=getopt(argc,argv,"dm:p:rx:h")) != EOF )
		switch (c) {
		case 'd':
			isdaemon = 1;
			break;
		case 'm':
			maxConnects = atoi(optarg);
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'r':
			restartEnable = 1;
			break;
		case 'x':
			debug = atoi(optarg);
			break;
		case 'h':
		case '?':
			Usage(argv[0]);
			exit(1);
		}

	remotefd = (int *) malloc (maxConnects * sizeof(int));

	openlog("bpd", LOG_PID, LOG_USER);

	// Try to find the pi platter
	memset(serName, '\0', sizeof(serName));
	if (FindPiPlatter(serName) == -1) {
		exit(1);
	}

	// Try to open the serial port
	if ((serialFd = OpenSerialPort(serName)) == -1) {
		exit(1);
	}

	// Initialize Pi Platter for Critical battery WARN message
	write(serialFd, "C2=1\r", 5);

	// Setup device file link
	linkFd = open("/dev/ptmx", O_RDWR);
	if (linkFd == -1) {
		syslog(LOG_ERR, "Open of /dev/ptmx failed: %m");
		exit(1);
	}
	LinkSlave(linkFd);

	signal(SIGINT,SigHandler);
	signal(SIGHUP,SigHandler);
	signal(SIGTERM,SigHandler);

	if (port) {
		/* Open the socket for communications */
		sockfd = socket(AF_INET, SOCK_STREAM, 6);
		if ( sockfd == -1 ) {
			syslog(LOG_ERR, "Can't open socket: %m");
			exit(1);
		}

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = 0;
		addr.sin_port = htons(port);
	 
		/* Set up to listen on the given port */
		if( bind( sockfd, (struct sockaddr*)(&addr),
			sizeof(struct sockaddr_in)) < 0 ) {
			syslog(LOG_ERR, "Couldn't bind port %d, aborting: %m",port );
			exit(1);
		}
		if ( debug>1 )
			syslog(LOG_NOTICE,"Bound port");

		/* Tell the system we want to listen on this socket */
		result = listen(sockfd, 4);
		if ( result == -1 ) {
			syslog(LOG_ERR, "Socket listen failed: %m");
			exit(1);
		}

		if ( debug>1 )
			syslog(LOG_NOTICE,"Done listen");
	}

	if ( isdaemon ) {
		setsid();
		close(0);
		close(1);
		close(2);
	}

	/* Set up the files/sockets for the select() call */
	FD_SET(serialFd,&fdsread);
	if ( serialFd >= maxfd )
		maxfd = serialFd + 1;

	FD_SET(linkFd,&fdsread);
	if ( linkFd >= maxfd )
		maxfd = linkFd + 1;

	if ( sockfd != -1 ) {
		FD_SET(sockfd,&fdsread);
		if ( sockfd >= maxfd )
			maxfd = sockfd + 1;
	}

	for (i=0 ; i<curConnects ; i++) {
		FD_SET(remotefd[i],&fdsread);
		if ( remotefd[i] >= maxfd )
			maxfd = remotefd[i] + 1;
	}

	/* Note we are running */
	syslog(LOG_NOTICE,"Connected to Pi Platter at %s", serName);

	while (1) {

		/* Wait for data from the listening socket, the device,
		   the linked device, or the remote connection */
		fdsreaduse = fdsread;
		if ( select(maxfd,&fdsreaduse,NULL,NULL,NULL) == -1 )
			break;

		/* Activity on the socket */
		if ( port && FD_ISSET(sockfd,&fdsreaduse) ) {
			int fd;

			/* Accept the remote systems attachment */
			remoteaddrlen = sizeof(struct sockaddr_in);
			fd = accept(sockfd,(struct sockaddr*)(&remoteaddr),
				&remoteaddrlen);

			if ( fd == -1 )
				syslog(LOG_ERR,"accept failed: %m");
			else if (curConnects < maxConnects) {
				unsigned long ip;

				remotefd[curConnects++] = fd;
				/* Tell select to watch this new socket */
				FD_SET(fd,&fdsread);
				if ( fd >= maxfd )
					maxfd = fd + 1;
				ip = ntohl(remoteaddr.sin_addr.s_addr);
				if (debug>0)
					syslog(LOG_NOTICE, "Connection from %d.%d.%d.%d",
						(int)(ip>>24)&0xff,
						(int)(ip>>16)&0xff,
						(int)(ip>>8)&0xff,
						(int)(ip>>0)&0xff);
			}
			else {
				// Too many connections, just close it to reject
				close(fd);
			}
		}

		/* Data to read from the pi platter */
		if ( FD_ISSET(serialFd,&fdsreaduse) ) {
			devbytes = read(serialFd,devbuf,512);
			if (debug>1)
				syslog(LOG_INFO,"Pi Platter: %d bytes",devbytes);
			if ( devbytes <= 0 ) {
				if ( debug>0 )
					syslog(LOG_INFO,"%s closed",serName);
				close(serialFd);
				FD_CLR(serialFd,&fdsread);
				while (1) {
					serialFd = OpenSerialPort(serName);
					if ( serialFd != -1 )
						break;
					syslog(LOG_ERR, "Open of %s failed: %m", serName);
					if ( errno != EIO )
						goto err_exit;
					sleep(1);
				}
				if ( debug>0 )
					syslog(LOG_INFO,"%s re-opened",serName);
				FD_SET(serialFd,&fdsread);
				if ( serialFd >= maxfd )
					maxfd = serialFd + 1;
			}
			else {
				if (sendMask & 0x01) {
					if ( linkFd != -1 ) 
						write(linkFd,devbuf,devbytes);
				}
				if (sendMask & 0x02) {
					for (i=0 ; i<curConnects ; i++)
						write(remotefd[i],devbuf,devbytes);
				}
				if (ProcessResponse(devbuf, devbytes)) {
					if (sawWarnCritical) {
						if (restartEnable) {
							write(serialFd, "C7=1\r", 5);
						}
						syslog(LOG_CRIT, "Battery Critical");
						system("sudo shutdown now");
					}
					if (sawWarnLow) {
						syslog(LOG_WARNING, "Battery Low");
					}
				}
				if ( debug>2 ) {
					devbuf[devbytes] = 0;
					syslog(LOG_INFO, "Pi Platter sent %s", devbuf);
				}
			}
		}

		/* Data to read from the linked device file */
		if ( FD_ISSET(linkFd,&fdsreaduse) ) {
			devbytes = read(linkFd,devbuf,512);
			if (debug>1)
				syslog(LOG_INFO,"%s: %d bytes", linkname, devbytes);
			if ( devbytes <= 0 ) {
				if ( debug>0 )
					syslog(LOG_INFO,"%s closed",linkname);
				close(linkFd);
				FD_CLR(linkFd,&fdsread);
				while (1) {
					linkFd = open("/dev/ptmx", O_RDWR);
					if ( linkFd != -1 )
						break;
					syslog(LOG_ERR, "Open of /dev/ptmx failed: %m");
					if ( errno != EIO )
						goto err_exit;
					sleep(1);
				}
				if ( debug>0 )
					syslog(LOG_INFO,"/dev/ptmx re-opened");
				LinkSlave(linkFd);
				FD_SET(linkFd,&fdsread);
				if ( linkFd >= maxfd )
					maxfd = linkFd + 1;
			}
			else if ( serialFd != -1 ) {
				/* Write the data to the pi platter */
				write(serialFd,devbuf,devbytes);
				if ( debug>2 ) {
					devbuf[devbytes] = 0;
					syslog(LOG_INFO, "%s sent %s", linkname, devbuf);
				}
				sendMask = 0x01;
			}
		}

		/* Data to read from the remote system */
		for (i=0 ; i<curConnects ; i++) {
			if (FD_ISSET(remotefd[i],&fdsreaduse) ) {

				devbytes = read(remotefd[i],devbuf,512);

				if (debug>1)
					syslog(LOG_INFO,"Remote: %d bytes",devbytes);

				if ( devbytes == 0 ) {
					register int j;

					if (debug>0)
						syslog(LOG_NOTICE,"Connection closed");
					close(remotefd[i]);
					FD_CLR(remotefd[i],&fdsread);
					curConnects--;
					for (j=i ; j<curConnects ; j++)
						remotefd[j] = remotefd[j+1];
				}
				else if ( serialFd != -1 ) {
					/* Write the data to the pi platter */
					write(serialFd,devbuf,devbytes);
					if ( debug>2 ) {
						devbuf[devbytes] = 0;
						syslog(LOG_INFO, "Remote sent %s", devbuf);
					}
					sendMask = 0x02;
				}
			}
		}
	}

err_exit:
	// We normally only exit from a signal (via the signal handler) so this
	// is used for an  error exit
	close(sockfd);
	for (i=0 ; i<curConnects ; i++)
		close(remotefd[i]);
	CloseSerialPort(serialFd);
	close(linkFd);
	exit(1);
}
