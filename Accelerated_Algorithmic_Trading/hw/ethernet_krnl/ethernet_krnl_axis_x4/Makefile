.PHONY: help

help::
	$(ECHO) "Makefile Usage:"
	$(ECHO) "  make all DEVICE=<FPGA platform>"
	$(ECHO) "      Command to generate the kernel xo for specified Target and Device."
	$(ECHO) ""
	$(ECHO) "  make clean "
	$(ECHO) "      Command to remove the generated non-hardware files."
	$(ECHO) ""
	$(ECHO) "  make cleanall"
	$(ECHO) "      Command to remove all the generated files."

# Points to Utility Directory
COMMON_REPO = ../../
ABS_COMMON_REPO = $(shell readlink -f $(COMMON_REPO))

KERNEL := ethernet_krnl_axis_x4

TARGETS := hw
TARGET := $(TARGETS)
DEVICE := $(DEVICES)
XCLBIN := ./xclbin

include ./utils.mk    
$(info $$DSA is [${DSA}])

include config.mk

BINARY_CONTAINERS += $(XCLBIN)/$(KERNEL).$(TARGET).$(DSA).xclbin
BINARY_CONTAINER_OBJS += $(XCLBIN)/$(KERNEL).$(TARGET).$(DSA).xo

CP = cp -rf

.PHONY: all clean cleanall
all: check-devices $(BINARY_CONTAINER_OBJS)

.PHONY: xo
xo: $(XCLBIN)/$(KERNEL).$(TARGET).$(DSA).xo

ifneq ($(TARGET),$(findstring $(TARGET), hw))
$(warning WARNING:Application supports only hw TARGET. Please use the target for running the application)
endif

# Cleaning stuff
clean:
	-$(RMDIR) sdaccel_* TempConfig system_estimate.xtxt *.rpt
	-$(RMDIR) src/*.ll _xocc_* .Xil dltmp* xmltmp* *.log *.jou *.wcfg *.wdb

cleanall: clean
	-$(RMDIR) $(XCLBIN)
	-$(RMDIR) _x
	-$(RMDIR) ./tmp_kernel_pack* ./packaged_kernel* 
