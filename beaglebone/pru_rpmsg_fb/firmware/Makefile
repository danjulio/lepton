# Setup environment variables for the PRU compiler and RPMSG support package
PRU_CGT_ROOT:= /usr/share/ti/cgt-pru/
PRU_RPMSG_ROOT:= /usr/lib/ti/pru-software-support-package/

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(notdir $(patsubst %/,%,$(dir $(MKFILE_PATH))))
PROJ_NAME=$(CURRENT_DIR)
LINKER_COMMAND_FILE=./AM335x_PRU.cmd
LIBS=--library=$(PRU_RPMSG_ROOT)lib/rpmsg_lib.lib --library=$(PRU_CGT_ROOT)lib/libc.a
INCLUDE=--include_path=$(PRU_CGT_ROOT)include --include_path=$(PRU_RPMSG_ROOT)include/ --include_path=$(PRU_RPMSG_ROOT)include/am335x/
STACK_SIZE=0x100
HEAP_SIZE=0x100
GEN_DIR=gen

#Common compiler and linker flags (Defined in 'PRU Optimizing C/C++ Compiler User's Guide)
#CFLAGS=-v3 -O2 --display_error_number --endian=little --hardware_mac=on --keep_asm --c_src_interlist --obj_directory=$(GEN_DIR) --pp_directory=$(GEN_DIR) -ppd -ppa
CFLAGS=-v3 -O2 --display_error_number --endian=little --hardware_mac=on --obj_directory=$(GEN_DIR) --pp_directory=$(GEN_DIR) -ppd -ppa
#Linker flags (Defined in 'PRU Optimizing C/C++ Compiler User's Guide)
LFLAGS=--reread_libs --warn_sections --stack_size=$(STACK_SIZE) --heap_size=$(HEAP_SIZE)

SOURCES=$(wildcard *.c)
#Using .object instead of .obj in order to not conflict with the CCS build process
OBJECTS=$(patsubst %,$(GEN_DIR)/%,$(SOURCES:.c=.object))

TARGET_PRU0=$(GEN_DIR)/pru0_main.out
TARGET_PRU1=$(GEN_DIR)/pru1_main.out

TARGETS=$(TARGET_PRU0) $(TARGET_PRU1)

MAP_PRU0=$(GEN_DIR)/pru0_main.map

MAP_PRU1=$(GEN_DIR)/pru1_main.map

SOURCES=main.c

OBJECTS_PRU0=$(GEN_DIR)/pru0_main.object

OBJECTS_PRU1=$(GEN_DIR)/pru1_main.object

all: printStart $(TARGETS) printEnd

printStart:
	@echo ''
	@echo '************************************************************'
	@echo 'Building project: $(PROJ_NAME)'

printEnd:
	@echo ''
	@echo 'Finished building project: $(PROJ_NAME)'
	@echo '************************************************************'
	@echo ''

# Invokes the linker (-z flag) to make the .out file
$(TARGET_PRU0): $(OBJECTS_PRU0) $(LINKER_COMMAND_FILE)
	@echo ''
	@echo 'Building target: $@'
	@echo 'Invoking: PRU Linker'
	/usr/bin/clpru $(CFLAGS) -z -i$(PRU_CGT_ROOT)/lib -i$(PRU_CGT_ROOT)/include $(LFLAGS) -o $(TARGET_PRU0) $(OBJECTS_PRU0) -m$(MAP_PRU0) $(LINKER_COMMAND_FILE) --library=libc.a $(LIBS)
	@echo 'Finished building target: $@'

$(TARGET_PRU1): $(OBJECTS_PRU1) $(LINKER_COMMAND_FILE)
	@echo ''
	@echo 'Building target: $@'
	@echo 'Invoking: PRU Linker'
	/usr/bin/clpru $(CFLAGS) -z -i$(PRU_CGT_ROOT)/lib -i$(PRU_CGT_ROOT)/include $(LFLAGS) -o $(TARGET_PRU1) $(OBJECTS_PRU1) -m$(MAP_PRU1) $(LINKER_COMMAND_FILE) --library=libc.a $(LIBS)
	@echo 'Finished building target: $@'

# Invokes the compiler on all c files in the directory to create the object files
$(GEN_DIR)/%.object: %.c
	@mkdir -p $(GEN_DIR)
	@echo ''
	@echo 'Building file: $<'
	@echo 'Invoking: PRU Compiler'
	/usr/bin/clpru --include_path=$(PRU_CGT_ROOT)/include $(INCLUDE) $(CFLAGS) -fe $@ $<

.PHONY: all clean

# Remove the $(GEN_DIR) directory
clean:
	@echo ''
	@echo '************************************************************'
	@echo 'Cleaning project: $(PROJ_NAME)'
	@echo ''
	@echo 'Removing files in the "$(GEN_DIR)" directory'
	@rm -rf $(GEN_DIR)
	@echo ''
	@echo 'Finished cleaning project: $(PROJ_NAME)'
	@echo '************************************************************'
	@echo ''

# Includes the dependencies that the compiler creates (-ppd and -ppa flags)
-include $(OBJECTS_PRU0:%.object=%.pp)
-include $(OBJECTS_PRU1:%.object=%.pp)


