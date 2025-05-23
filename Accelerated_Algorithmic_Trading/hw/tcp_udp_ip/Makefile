# Copyright (c) 2019 Xilinx, Inc.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from this
#    software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE#
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.

# Makefile to create kernel for UDP/IP, TCP/IP, or combined TCP/UDP/IP kernel

SUBCORES = arp_server_subnet_1_0 icmp_server_1_0 igmp_1_0

ifndef UDP_ONLY
SUBCORES += toe_1_0 uram_datamover_1_0 hash_table_1_0
endif

ifdef UDP_ONLY
SUBCORES += ip_handler_udp_1_0
TARGET=udp_ip_krnl.xo
else ifdef TCP_ONLY
SUBCORES += ip_handler_tcp_1_0
TARGET=tcp_ip_krnl.xo
else
SUBCORES += ip_handler_tcp_udp_1_0
TARGET=tcp_udp_ip_krnl.xo
endif

# use platform info utility to query correct part for board target
ifndef DEVICE
$(error DEVICE should be set to a valid Xilinx platform file (xpfm))
else
XPART=$(shell platforminfo $(DEVICE) --json="hardwarePlatform.board.part")
endif

# set default clock period while allowing environment override
KERNEL_PERIOD?=3.125

all: $(TARGET)

FQSUBCORES = $(addprefix com_xilinx_dcg_fintech_hls_,$(SUBCORES))

IPREPO_DIR = iprepo
IPREPO_SUBCORES = $(addprefix $(IPREPO_DIR)/,$(FQSUBCORES))

PACKAGED_KERNELS_DIR = packaged_kernels
PACKAGED_KERNELS = $(addprefix $(PACKAGED_KERNELS_DIR)/,$(FQSUBCORES))
gather-packaged-kernels: $(PACKAGED_KERNELS)

hls: $(IPREPO_SUBCORES)

$(IPREPO_SUBCORES):
	XPART=$(XPART) KERNEL_PERIOD=$(KERNEL_PERIOD) $(MAKE) -C hls

$(PACKAGED_KERNELS_DIR):
	mkdir $@

# Define a pattern rule to copy HLS IPs out of
$(PACKAGED_KERNELS_DIR)/% : $(IPREPO_DIR)/% | $(PACKAGED_KERNELS_DIR)
	cp -r $< $(PACKAGED_KERNELS_DIR)

$(TARGET): $(PACKAGED_KERNELS)
	XPART=$(XPART) vivado -mode batch -source scripts/create_xo.tcl -tclargs $(TARGET)

.PHONY: clean
clean:
	-rm -f *.xo
	-rm -rf tmp_tcp_ip_pack packaged_kernels packaged_top_level

.PHONY: realclean
realclean: clean
	$(MAKE) -C hls realclean
	-rm -rf iprepo/com_xilinx_dcg_fintech*
	-rm -f *.jou *.log *.tmp
	-rm -rf .Xil
