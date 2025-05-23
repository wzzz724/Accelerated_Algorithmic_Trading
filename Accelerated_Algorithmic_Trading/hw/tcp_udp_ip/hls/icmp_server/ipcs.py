# Copyright (c) 2016, Xilinx, Inc.
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

import binascii

def split_string_into_len(str, size=8):
	return [ str[i:i+size] for i in range(0, len(str), size) ]

def hexstream_reverse_bytes(hexstream):
	bytes = split_string_into_len(hexstream, 2)
	reversed_bytes = bytes[::-1]
	return "".join(reversed_bytes)

def calculateChecksum(ip_header, size):
    while (len(ip_header) % 4) != 0:
        ip_header = ip_header + "0"
        size = size + 1

    cksum = 0
    pointer = 0

    #The main loop adds up each set of 2 bytes. They are first converted to strings and then concatenated
    #together, converted to integers, and then added to the sum.
    while size > 1:
        temp = ip_header[pointer:pointer+4]
        temp2 = int(temp, 16)
        cksum += temp2
        size -= 4
        pointer += 4
    if size: #This accounts for a situation where the header is odd
        cksum += int(ip_header[pointer], 16)

    cksum = (cksum >> 16) + (cksum & 0xffff)
    cksum += (cksum >>16)

    return (~cksum) & 0xFFFF

ip_header = "37363534333231302f2e2d2c2b2a292827262524232221201f1e1d1c1b1a1918171615141312111000000000000d188a0000000051cd55b80100b463b7b80008"
#ip_header = "0ADF0024FA"
print(ip_header)
ip_header = hexstream_reverse_bytes(ip_header)
ip_header = ip_header.upper()
binascii.hexlify(ip_header)
print(ip_header)
cs = hex(calculateChecksum(ip_header, len(ip_header)))[2:]
print(cs)