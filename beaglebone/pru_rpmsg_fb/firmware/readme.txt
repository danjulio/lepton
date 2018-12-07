Some notes about making and installing the firmware.  This must be done
before starting any userspace program in order to create the /dev/rpmsg_pru31
device file used to communicate with the PRUs through. 

Type "make" to build firmware.  Firmware binary files are in 'gen': pru0_main.out and
pru1_main.out.  

Setup remoteproc devices for access
  sudo chmod 666 /sys/class/remoteproc/remoteproc1/state
  sudo chmod 666 /sys/class/remoteproc/remoteproc1/firmware
  sudo chmod 666 /sys/class/remoteproc/remoteproc2/state
  sudo chmod 666 /sys/class/remoteproc/remoteproc2/firmware

Copy firwmare to /lib/firmware directory where the remoteproc system can find it to load.
  sudo cp gen/pru0_main.out /lib/firmware/am335x-pru0-fw
  sudo cp gen/pru1_main.out /lib/firmware/am335x-pru1-fw

Load firmware.  The PRUs must be stopped before loading new code.
  echo "stop" > /sys/class/remoteproc/remoteproc1/state
  echo "stop" > /sys/class/remoteproc/remoteproc2/state
  echo "am335x-pru0-fw" > /sys/class/remoteproc/remoteproc1/firmware
  echo "am335x-pru1-fw" > /sys/class/remoteproc/remoteproc2/firmware

Start firmware
  echo "start" > /sys/class/remoteproc/remoteproc1/state
  echo "start" > /sys/class/remoteproc/remoteproc2/state

