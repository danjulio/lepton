/*
 * Program to send a command and receive any response from the beaglebone version
 * of the pi platter.  This code was ported from the talkpp.c source.
 *
 * First tries to communicate throught the pseudo-tty "/dev/bb-platter" that is
 * created when the daemon, bpd, is running.  If that fails (e.g. the daemon is
 * not running) then it tries to open the serial port associated with the board.
 *
 *   talkbp [-c <command string>]
 *          [-s] [-t] [-f]
 *          [-a <alarm timespec>] [-d <delta seconds>] [-w]
 *          [-u | -h]
 *
 *   -c <command string> : send the command string.  Command strings without an "="
 *      character cause the utility to echo back a response.
 *   -s : Set the Device RTC with the current system clock
 *   -t : Get the time from the Device RTC and display it in a form useful to
 *      pass to "date" to set the system clock ("+%m%d%H%M%Y.%S")
 *   -f : Get the time from the Device RTC and display it in a readable form.
 *   -a <alarm timespec> : Set the Device wakeup value (does not enable the alarm).
 *      <alarm timespec> is the alarm time in date time format ("+%m%d%H%M%Y.%S")
 *   -d <delta seconds> : Set the Device wakeup to <delta seconds> past the current
 *      Device RTC time value (does not enable the alarm)
 *   -w : Display the wakeup value in a readable form.
 *   -u, -h : Usage (and optional help)
 *
 * Build: gcc -o talkbp talkbp.c -ludev
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
#include <ctype.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <libudev.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#define __USE_XOPEN 1 /* needed for strptime */
#include <time.h>
#include <unistd.h>

#define MAX_SER_NAME_CHARS  64
#define MAX_RSP_CHARS       64
#define TRUE                1
#define FALSE               0

#define VERSION_MAJOR       0
#define VERSION_MINOR       5

// MAX_WAIT_TIME is uSec to wait for a response after sending a command
#define MAX_WAIT_TIME      2000000



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
        fprintf(stderr, "Can't create udev\n");
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
        fprintf(stderr, "Could not find Pi Platter\n");
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
        return(-1);
    }

    // Configure options
    options.c_cflag = CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNCR;
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
        perror("Error waiting for drain");
    }   

    close(fd);
}

    
int GetResponse(int fd, int enableWait, char* strP)
{
    char buf[MAX_RSP_CHARS+1];
    int Success = FALSE;
    int SawFirstLegal = FALSE;
    int Done = FALSE;
    int NumRcvd = 0;
    int i, j;
    int TotalSleepTime = 0; // uSec
    int MaxSleepTime;

    if (enableWait) {
        MaxSleepTime = MAX_WAIT_TIME;
    } else {
        MaxSleepTime = 20000;
    }

    // reset array
    for (i = 0; i<MAX_RSP_CHARS; i++) *(strP + i) = 0;
    
    // Try to get the data
    while (!Done) {
        if ((j = read(fd, buf, MAX_RSP_CHARS)) != -1) {
            for (i=0; i<j; i++) {
                if ((buf[i] == 0x0A) || (buf[i] == 0x0D)) {
                    // Ignore termination characters before valid data since they might
                    // be left over from some previous communication
                    if (SawFirstLegal) {
                        Done = TRUE;
                        Success = TRUE;
                    }
                } else {
                    SawFirstLegal = TRUE;
                    *(strP + NumRcvd + i) = buf[i];
                }
                if ((NumRcvd+i) >= MAX_RSP_CHARS) {
                    Done = TRUE;
                    break;
                }
            }
            NumRcvd += j;
        }
        if (!Done) {
            if (TotalSleepTime < MaxSleepTime) {
                TotalSleepTime += 20000;
                (void) usleep((unsigned int) 20000);
            } else {
                Done = TRUE; 
            }
        }
    }
    
    //printf("%d : ", TotalSleepTime);
    if (Success)
        return(NumRcvd);
    else
        return(0);
}


// Returns -1 if no "=" found, otherwise returns 1-based position
int StringHasEqualSign(char* sP) {
    int i;
    int l = strlen(sP);

    for (i=0; i<l; i++) {
        if (*(sP + i) == '=') {
           i++;
           return(i);
        }
    }

    return(-1);
}


// Returns true if command should generate a response, false otherwise
int SendCmd(int fd, char* cmdP) {
    write(fd, cmdP, strlen(cmdP));
    write(fd, "\r", 1);
    if (tcdrain(fd) == -1)
    {
        perror("Error waiting for drain after sending command");
    }   

    if (StringHasEqualSign(cmdP) == -1) {
        // No equal sign ==> expect a response
        return(TRUE);
    } else {
        return(FALSE);
    }
}


void PrintResponse(int fd, int enableWait) {
    char rspPacket[MAX_RSP_CHARS];
    int i;

    if (GetResponse(fd, enableWait, rspPacket) != 0) {
        if ((strncmp(rspPacket, "WARN", 4) == 0) ||
            (strncmp(rspPacket, "ERR", 3) == 0)) {
            // Print the entire string
            printf("%s\n", rspPacket);
        } else {
            // Find the "=" character position
            i = StringHasEqualSign(rspPacket);
 
            // Print the response to the right of the "="
            printf("%s\n", &rspPacket[0]+i);
        }
    }
}


void SetTime(int fd) {
    time_t now;
    char buf[80];

    // Get the current system time
    time(&now);

    // Send it to the device as a Time Set command
    sprintf(buf, "T=%d", (unsigned int) now);
    (void) SendCmd(fd, buf);
}


int GetDeviceTime(int fd, int isAlarmTime, unsigned int* t) {
    char rspPacket[MAX_RSP_CHARS];
    int i;

    if (isAlarmTime) {
        (void) SendCmd(fd, "W");  // Send a command to get the wakup time 
    } else {
        (void) SendCmd(fd, "T");  // Send a command to get the time 
    }

    if (GetResponse(fd, TRUE, rspPacket) != 0) {
        // Find the "=" character position
        for (i=0; i<strlen(rspPacket); i++) {
            if (rspPacket[i] == '=') {
               i++;
               break;
            }
        }

        // Convert the response to an integer
        *t = (unsigned int) atol(&rspPacket[0] + i);

        return(TRUE);
    }

    return(FALSE);
}


void PrintTime(int fd, int isAlarmTime, int beFriendly) {
    unsigned int it;
    time_t t;
    struct tm ts;
    char buf[80];

    if (GetDeviceTime(fd, isAlarmTime, &it)) {
        // Convert time epoch time to a readable representation
        t = (time_t) it;
        ts = *localtime(&t);
        if (beFriendly) {
            strftime(buf, sizeof(buf), "%c", &ts);
        } else {
            strftime(buf, sizeof(buf), "%m%d%H%M%Y.%S", &ts);
        }
        printf("%s\n", buf);
    }
}


void SetAlarmString(int fd, char* alarmP) {
    struct tm ts;
    time_t t;
    char buf[80];

    // Try to convert the alarm time into a timestamp
    if (strptime(alarmP, "%m%d%H%M%Y.%S", &ts)) {
        // Convert to epoch time
        if ((t = mktime(&ts)) != -1) {
            // Send it to the device
            sprintf(buf, "W=%d", (unsigned int) t);
            (void) SendCmd(fd, buf);
        }
    }
}


void SetAlarmDelta(int fd, unsigned int alarmT) {
    unsigned int t;
    char buf[80];

    if (GetDeviceTime(fd, FALSE, &t)) {
        // Send it to the device as a Wakeup Set command
        sprintf(buf, "W=%d", t + alarmT);
        (void) SendCmd(fd, buf);
    }
}



int main(int argc, char** argv) {
    char *optValue = NULL;
    char serName[MAX_SER_NAME_CHARS];
    char *cmdP;
    char *alarmP;
    char *deltaP;
    int c;
    int doCmd = FALSE;
    int cmdHasResponse = FALSE;
    int doSetTime = FALSE;
    int doGetTime = FALSE;
    int getTimeFriendly = FALSE;
    int doSetAlarmA = FALSE;
    int doSetAlarmD = FALSE;
    int doGetAlarm = FALSE;
    int serialFd;

    // process command line arguments
    while ((c = getopt(argc, argv, "a:c:d:fhstuw")) != -1) {
        switch(c) {
            case 'a':
                alarmP = optarg;
                doSetAlarmA = TRUE;
                doSetAlarmD = FALSE; // only one can be valid at time
                break;
            case 'c':
                cmdP = optarg;
                doCmd = TRUE;
                break;
            case 'd':
                deltaP = optarg;
                doSetAlarmD = TRUE;
                doSetAlarmA = FALSE; // only one can be valid at time
                break;
            case 's':
                doSetTime = TRUE;
                break;
            case 'f':
                doGetTime = TRUE;
                getTimeFriendly = TRUE;
                break;
            case 't':
                doGetTime = TRUE;
                getTimeFriendly = FALSE;
                break;
            case 'w':
                doGetAlarm = TRUE;
                break;
            case 'h':
            case 'u':
                printf("%s: [-c <command string>]\n", argv[0]);
                printf("       [-s] [-t] [-a <alarm timespec>] [-d <delta seconds>]\n");
                printf("       [-u | -h]\n");
                if (c == 'h') {
                    printf("\n");
                    printf("  -c <command string> : send the command string.  Command strings without\n");
                    printf("     a '=' character cause the utility to echo back a response.\n");
                    printf("  -s : Set the device RTC with the current system clock.\n");
                    printf("  -t : Get the time from the Device RTC and display it in a form useful to\n");
                    printf("     pass to \"date\" to set the system clock (mmddHHMMccyy.ss)\n");
                    printf("  -f : Get the time from the Device RTC and display it in a readable form.\n");
                    printf("  -a <alarm timespec> : Set the Device wakeup value (does not enable the alarm).\n");
                    printf("     <alarm timespec> is the alarm time in \"date\" format (mmddHHMMccyy.ss)\n");
                    printf("  -d <delta seconds> : Set the Device wakeup to <delta seconds> past the\n"); 
                    printf("     current Device RTC time value (does not enable the alarm)\n");
                    printf("  -w : Display the wakeup value in a readable form.\n");
                    printf("  -u, -h : Usage (and optional help)\n");
                }
                printf("Version %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
                exit(0);
            default:
              abort();
        }
    }

    // Try to find the pi platter
    memset(serName, '\0', sizeof(serName));
    if (FindPiPlatter(serName) == -1) {
        return(-1);
    }

    // Try to open the serial port indirectly or directly associated with the pi platter
    if ((serialFd = OpenSerialPort("/dev/bb-platter")) == -1) {
        if ((serialFd = OpenSerialPort(serName)) == -1) {
            perror("Could not open Pi Platter");
            return(-1);
	}
    }

    // Output any string that we get as soon as we opened the device
    PrintResponse(serialFd, FALSE);

    // Process any set commands
    if (doCmd == TRUE) {
        cmdHasResponse = SendCmd(serialFd, cmdP);
        PrintResponse(serialFd, cmdHasResponse);
    }
    if (doSetTime == TRUE) {
       SetTime(serialFd);
    }
    if (doGetTime == TRUE) {
       PrintTime(serialFd, FALSE, getTimeFriendly);
    }
    if (doSetAlarmA == TRUE) {
        SetAlarmString(serialFd, alarmP);
    }
    if (doSetAlarmD == TRUE) {
        SetAlarmDelta(serialFd, (unsigned int) atol(deltaP));
    }
    if (doGetAlarm == TRUE) {
        PrintTime(serialFd, TRUE, TRUE);
    }

    // Successfully done
    CloseSerialPort(serialFd);
    return(0);
}
