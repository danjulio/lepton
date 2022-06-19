"""
Source: http://code.activestate.com/recipes/578225-linux-ioctl-numbers-in-python/
Linux ioctl numbers made easy
size can be an integer or format string compatible with struct module
for example include/linux/watchdog.h:
#define WATCHDOG_IOCTL_BASE     'W'
struct watchdog_info {
        __u32 options;          /* Options the card/driver supports */
        __u32 firmware_version; /* Firmware version of the card */
        __u8  identity[32];     /* Identity of the board */
};
#define WDIOC_GETSUPPORT  _IOR(WATCHDOG_IOCTL_BASE, 0, struct watchdog_info)
becomes:
WDIOC_GETSUPPORT = _IOR(ord('W'), 0, "=II32s")
"""

import struct
# constant for linux portability
_IOC_NRBITS = 8
_IOC_TYPEBITS = 8

# architecture specific
_IOC_SIZEBITS = 14
_IOC_DIRBITS = 2

_IOC_NRMASK = (1 << _IOC_NRBITS) - 1
_IOC_TYPEMASK = (1 << _IOC_TYPEBITS) - 1
_IOC_SIZEMASK = (1 << _IOC_SIZEBITS) - 1
_IOC_DIRMASK = (1 << _IOC_DIRBITS) - 1

_IOC_NRSHIFT = 0
_IOC_TYPESHIFT = _IOC_NRSHIFT + _IOC_NRBITS
_IOC_SIZESHIFT = _IOC_TYPESHIFT + _IOC_TYPEBITS
_IOC_DIRSHIFT = _IOC_SIZESHIFT + _IOC_SIZEBITS

_IOC_NONE = 0
_IOC_WRITE = 1
_IOC_READ = 2


def _IOC(dir, type, nr, size):
    # in Python3, unicode -> str; str -> bytes
    if isinstance(size, bytes) or isinstance(size, str):
        size = struct.calcsize(size)
    return dir  << _IOC_DIRSHIFT  | \
           type << _IOC_TYPESHIFT | \
           nr   << _IOC_NRSHIFT   | \
           size << _IOC_SIZESHIFT


def _IO(type, nr): return _IOC(_IOC_NONE, type, nr, 0)
def _IOR(type, nr, size): return _IOC(_IOC_READ, type, nr, size)
def _IOW(type, nr, size): return _IOC(_IOC_WRITE, type, nr, size)
def _IOWR(type, nr, size): return _IOC(_IOC_READ | _IOC_WRITE, type, nr, size)


################################################################################
# This section was influenced from the Lepton.py code from GroupGets.
# Here: https://github.com/groupgets/pylepton.git


SPI_IOC_MAGIC   = ord("k")

SPI_IOC_RD_MODE          = _IOR(SPI_IOC_MAGIC, 1, "=B")
SPI_IOC_WR_MODE          = _IOW(SPI_IOC_MAGIC, 1, "=B")

SPI_IOC_RD_LSB_FIRST     = _IOR(SPI_IOC_MAGIC, 2, "=B")
SPI_IOC_WR_LSB_FIRST     = _IOW(SPI_IOC_MAGIC, 2, "=B")

SPI_IOC_RD_BITS_PER_WORD = _IOR(SPI_IOC_MAGIC, 3, "=B")
SPI_IOC_WR_BITS_PER_WORD = _IOW(SPI_IOC_MAGIC, 3, "=B")

SPI_IOC_RD_MAX_SPEED_HZ  = _IOR(SPI_IOC_MAGIC, 4, "=I")
SPI_IOC_WR_MAX_SPEED_HZ  = _IOW(SPI_IOC_MAGIC, 4, "=I")

SPI_CPHA   = 0x01                 # /* clock phase */
SPI_CPOL   = 0x02                 # /* clock polarity */
SPI_MODE_0 = (0|0)                # /* (original MicroWire) */
SPI_MODE_1 = (0|SPI_CPHA)
SPI_MODE_2 = (SPI_CPOL|0)
SPI_MODE_3 = (SPI_CPOL|SPI_CPHA)

