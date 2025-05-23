#
# Copyright 2021 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Makefile for xclbin creation
#

#+-------------------------------------------------------------------------------
# The following parameters are assigned with default values. These parameters can
# be overridden through the make command line
#+-------------------------------------------------------------------------------

REPORT := no
PROFILE := no
DEBUG := no
ETHERNET_DATA_WIDTH := 64

#'estimate' for estimate report generation
#'system' for system report generation
ifneq ($(REPORT), no)
CLFLAGS += --report estimate
LDCLFLAGS += --report system
endif

# generates profile summary report
ifeq ($(PROFILE), yes)
LDCLFLAGS += --profile_kernel data:all:all:all
endif

# generates debug summary report
ifeq ($(DEBUG), yes)
CLFLAGS += --dk protocol:all:all:all
endif

# configuration settings
VPP := $(XILINX_VITIS)/bin/v++

# extract platform name only from full path to xpfm
XILINX_PLATFORM=$(basename $(notdir $(DEVICE)))

# set default hardware target, allows override for hardware emulation (hw_emu)
TARGET ?= hw

# set default Slave Bridge mode, allows overide for DMA variant (DMA)
DM_MODE ?= SB

ifeq ($(TARGET),hw)
PROJECT=aat
else
PROJECT=aat_hw_emu
endif

ETHERNET_XO := ethernet_krnl_axis_x4.hw.$(XILINX_PLATFORM).xo
LOOPBACK_XO := loopback_krnl_axis_x1.xo
UDP_IP_XO := udp_ip_krnl.xo
TCP_IP_XO := tcp_ip_krnl.xo
LINEHANDLER_XO := lineHandlerTop.xo
FEEDHANDLER_XO := feedHandlerTop.xo
ORDERBOOK_XO := orderBookTop.xo
ORDERBOOK_DM_XO := orderBookDataMoverTop.xo
PRICINGENGINE_XO := pricingEngineTop.xo
ORDERENTRY_TCP_XO := orderEntryTcpTop.xo
CLK_TICK_GEN_XO = clockTickGeneratorTop.xo
IPC_SLAVE_XO := sim_ipc_axis_slave_$(ETHERNET_DATA_WIDTH).xo
IPC_MASTER_XO := sim_ipc_axis_master_$(ETHERNET_DATA_WIDTH).xo

ETHERNET_XO_FULLPATH=../hw/ethernet_krnl/ethernet_krnl_axis_x4/xclbin/$(ETHERNET_XO)
LOOPBACK_XO_FULLPATH=../hw/ethernet_krnl/loopback_krnl_axis_x1/$(LOOPBACK_XO)
UDP_IP_XO_FULLPATH=../hw/tcp_udp_ip/$(UDP_IP_XO)
TCP_IP_XO_FULLPATH=../hw/tcp_udp_ip/$(TCP_IP_XO)
LINEHANDLER_XO_FULLPATH=../hw/lineHandler/$(LINEHANDLER_XO)
FEEDHANDLER_XO_FULLPATH=../hw/feedHandler/$(FEEDHANDLER_XO)
ORDERBOOK_XO_FULLPATH=../hw/orderBook/$(ORDERBOOK_XO)
ORDERBOOK_DM_XO_FULLPATH=../hw/orderBook/$(ORDERBOOK_DM_XO)
PRICINGENGINE_XO_FULLPATH=../hw/pricingEngine/$(PRICINGENGINE_XO)
ORDERENTRY_TCP_XO_FULLPATH=../hw/orderEntry/$(ORDERENTRY_TCP_XO)
CLK_TICK_GEN_XO_FULLPATH=../hw/clockTickGenerator/$(CLK_TICK_GEN_XO)

# emulation flow uses IPC libs from Vitis release
IPC_SLAVE_XO_FULLPATH=${XILINX_VITIS}/data/emulation/XO/$(IPC_SLAVE_XO)
IPC_MASTER_XO_FULLPATH=${XILINX_VITIS}/data/emulation/XO/$(IPC_MASTER_XO)

# shell specific connectivity
ifeq ($(TARGET),$(filter $(TARGET),hw_emu))
ifeq ($(DM_MODE), SB)
VPPLINKFLAGS := --config aat.hw_emu.sb.ini
else
VPPLINKFLAGS := --config aat.hw_emu.ini
endif
else
ifneq (,$(shell echo $(XILINX_PLATFORM) | awk '/_u50_gen.*_xdma_/'))
ifeq ($(DM_MODE), DMA)
VPPLINKFLAGS := --config aat.u50.ini
else
$(error Unsupported DM_MODE=$(DM_MODE) for XILINX_PLATFORM=$(XILINX_PLATFORM))
endif
else ifneq (,$(shell echo $(XILINX_PLATFORM) | awk '/_u50_gen.*_nodma_/'))
ifeq ($(DM_MODE), SB)
VPPLINKFLAGS := --config aat.u50.sb.ini
else
$(error Unsupported DM_MODE=$(DM_MODE) for XILINX_PLATFORM=$(XILINX_PLATFORM))
endif
else ifneq (,$(shell echo $(XILINX_PLATFORM) | awk '/_u250_gen.*_xdma_/'))
ifeq ($(DM_MODE), DMA)
VPPLINKFLAGS := --config aat.u250.ini
else ifeq ($(DM_MODE), SB)
VPPLINKFLAGS := --config aat.u250.sb.ini
else
$(error Unsupported DM_MODE=$(DM_MODE) for XILINX_PLATFORM=$(XILINX_PLATFORM))
endif
else
$(error Unsupported XILINX_PLATFORM=$(XILINX_PLATFORM))
endif
endif

XODIR = xo
ifeq ($(TARGET),$(filter $(TARGET),hw_emu))
# adds debug symbols, IPC hooks for hardware emulation builds
VPPLINKFLAGS += -g
XOS = $(IPC_MASTER_XO) $(IPC_SLAVE_XO)
else
XOS = $(ETHERNET_XO) \
      $(LOOPBACK_XO)
endif
XOS += $(UDP_IP_XO) \
       $(TCP_IP_XO) \
       $(LINEHANDLER_XO) \
       $(FEEDHANDLER_XO) \
       $(ORDERBOOK_XO) \
       $(ORDERBOOK_DM_XO) \
       $(PRICINGENGINE_XO) \
       $(ORDERENTRY_TCP_XO) \
       $(CLK_TICK_GEN_XO)

CLFLAGS += -t $(TARGET) --platform $(DEVICE) --save-temps --kernel_frequency 320

$(XODIR):
	mkdir $@

# gather up external kernels
xo/$(ETHERNET_XO): $(ETHERNET_XO_FULLPATH) | $(XODIR)
	cp $< $@

xo/$(LOOPBACK_XO): $(LOOPBACK_XO_FULLPATH) | $(XODIR)
	cp $< $@

xo/$(UDP_IP_XO): $(UDP_IP_XO_FULLPATH) | $(XODIR)
	cp $< $@

xo/$(TCP_IP_XO): $(TCP_IP_XO_FULLPATH) | $(XODIR)
	cp $< $@

xo/$(LINEHANDLER_XO): $(LINEHANDLER_XO_FULLPATH) | $(XODIR)
	cp $< $@

xo/$(FEEDHANDLER_XO): $(FEEDHANDLER_XO_FULLPATH) | $(XODIR)
	cp $< $@

xo/$(ORDERBOOK_XO): $(ORDERBOOK_XO_FULLPATH) | $(XODIR)
	cp $< $@

xo/$(ORDERBOOK_DM_XO): $(ORDERBOOK_DM_XO_FULLPATH) | $(XODIR)
	cp $< $@

xo/$(PRICINGENGINE_XO): $(PRICINGENGINE_XO_FULLPATH) | $(XODIR)
	cp $< $@

xo/$(ORDERENTRY_TCP_XO): $(ORDERENTRY_TCP_XO_FULLPATH) | $(XODIR)
	cp $< $@

xo/$(CLK_TICK_GEN_XO): $(CLK_TICK_GEN_XO_FULLPATH) | $(XODIR)
	cp $< $@

xo/$(IPC_SLAVE_XO): $(IPC_SLAVE_XO_FULLPATH) | $(XODIR)
	cp $< $@

xo/$(IPC_MASTER_XO): $(IPC_MASTER_XO_FULLPATH) | $(XODIR)
	cp $< $@

# emulation configuration
emconfig: $(EMCONFIG_DIR)/emconfig.json
$(EMCONFIG_DIR)/emconfig.json:
	emconfigutil --platform $(DEVICE) --od $(EMCONFIG_DIR)

BINARY_CONTAINER = $(PROJECT).xclbin
BINARY_CONTAINER_XOS = $(addprefix $(XODIR)/,$(XOS))

.PHONY: gather-xos
gather-xos: $(BINARY_CONTAINER_XOS)

.PHONY: binary-container
binary-container: $(BINARY_CONTAINER)

$(BINARY_CONTAINER): $(BINARY_CONTAINER_XOS)
	$(VPP) $(CLFLAGS) $(LDCLFLAGS) -l -o $@ $^ $(VPPLINKFLAGS)

.PHONY: clean
clean:
	-rm -f $(PROJECT).xclbin* $(PROJECT).ltx
	-rm -rf _x .Xil
	-rm -f *.log

realclean: clean
	-rm -rf xo/

.PHONY: help
help:
	@echo "Makefile usage:"
	@echo "  make binary-container  Generate hardware build (xclbin)"
	@echo "  make clean             Clean intermediate build folders"
	@echo "  make realclean         Clean + remove imported kernel objects (xo)"
