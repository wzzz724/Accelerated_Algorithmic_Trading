# Copyright (c) 2019-2020 Xilinx, Inc.
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

# Common Makefile definitions for the HLS IP subcores in the TCP/IP stack.

.PHONY: export_ip hls clean

# Top level Makefile will push down an XPART value
# Default below only currently effective for verification flow
XPART?=xcu50-fsvh2104-2L-e
KERNEL_PERIOD?=3.33

# Relative directories are defined from the point of view of the subdirectory below.
IPREPO=../../iprepo

VENDOR_LIBRARY=com_xilinx_dcg_fintech_hls
# Fully qualified IP name definition
FQIPNAME=$(VENDOR_LIBRARY)_$(IPNAME)_$(IPVERSION)

ifdef UDP_ONLY
HLSBUILD=UDP_ONLY
else ifdef TCP_ONLY
HLSBUILD=TCP_ONLY
else
HLSBUILD=TCP_UDP
endif

.PHONY: hls
hls:
	XPART=$(XPART) KERNEL_PERIOD=$(KERNEL_PERIOD) HLSBUILD=$(HLSBUILD) vitis_hls -f run_hls.tcl

.PHONY: export_ip
export_ip: hls
	cp $(IPNAME)_prj/solution1/impl/ip/$(FQIPNAME).zip $(IPREPO)
	rm -rf   $(IPREPO)/$(FQIPNAME)
	mkdir -p $(IPREPO)/$(FQIPNAME)
	unzip $(IPREPO)/$(FQIPNAME).zip -d $(IPREPO)/$(FQIPNAME)
	rm $(IPREPO)/$(FQIPNAME).zip

.PHONY: clean
clean:
	-rm -rf $(IPNAME)_prj
	-rm -f *.log

.PHONY: realclean
realclean: clean
