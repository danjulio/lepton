.PHONY: clean

# Headers
INCLUDES = include/

# Sources
RPMSG_FB_SOURCES = src/cci.c src/fb.c src/log.c src/pru_rpmsg_fb.c src/vospi.c
PRU_LEPTONIC_SOURCES = src/cci.c src/log.c src/pru_leptonic.c src/vospi.c
ZMQ_FB_SOURCES = src/fb.c src/log.c src/zmq_fb.c
REBOOT_SOURCES = src/cci.c src/log.c src/reboot_lep.c
FFC_SOURCES = src/cci.c src/log.c src/ffc.c

CC = gcc
CFLAGS = -g -DLOG_USE_COLOR=1 -Wall

all: pru_rpmsg_fb pru_leptonic zmq_fb reboot_lep ffc

pru_rpmsg_fb: $(RPMSG_FB_SOURCES) $(INCLUDES)
	$(CC) $(CFLAGS) -pthread -I $(INCLUDES) $(RPMSG_FB_SOURCES) -o pru_rpmsg_fb

pru_leptonic: $(PRU_LEPTONIC_SOURCES) $(INCLUDES)
	$(CC) $(CFLAGS) -lzmq -pthread -I $(INCLUDES) $(PRU_LEPTONIC_SOURCES) -o pru_leptonic

zmq_fb: $(ZMQ_FB_SOURCES) $(INCLUDES)
	$(CC) $(CFLAGS) -lzmq -pthread -I $(INCLUDES) $(ZMQ_FB_SOURCES) -o zmq_fb

reboot_lep: $(REBOOT_SOURCES) $(INCLUDES)
	$(CC) $(CFLAGS) -I $(INCLUDES) $(REBOOT_SOURCES) -o reboot_lep

ffc: $(FFC_SOURCES) $(INCLUDES)
	$(CC) $(CFLAGS) -I $(INCLUDES) $(FFC_SOURCES) -o ffc

clean:
	@rm pru_rpmsg_fb
	@rm pru_leptonic
	@rm zmq_fb
	@rm reboot_lep
	@rm ffc
