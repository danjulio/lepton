#
## Makefile
#
#CC = gcc
CFLAGS = -Wall -Wshadow -Wundef -Wmaybe-uninitialized
CFLAGS += -O3 -g3 -I./
#LDFLAGS += -lSDL2 -lm
BIN = demo
VPATH = 

LVGL_DIR = ${shell pwd}

MAINSRC = main.c

#LIBRARIES
include ./lvgl/src/lv_core/lv_core.mk
include ./lvgl/src/lv_hal/lv_hal.mk
include ./lvgl/src/lv_objx/lv_objx.mk
include ./lvgl/src/lv_font/lv_font.mk
include ./lvgl/src/lv_misc/lv_misc.mk
include ./lvgl/src/lv_themes/lv_themes.mk
include ./lvgl/src/lv_draw/lv_draw.mk

#DRIVERS
include ./lv_drivers/display/display.mk
include ./lv_drivers/indev/indev.mk

#DEMO
include ./lv_examples/lv_apps/demo/demo.mk

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
	COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

## MAINOBJ -> OBJFILES

all: clean default

%.o: %.c
		@$(CC)  $(CFLAGS) -c $< -o $@
			@echo "CC $<"
			    
default: $(AOBJS) $(COBJS) $(MAINOBJ)
		$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS)

clean: 
		rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ)
