#!/usr/bin/env python

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

from xilinx_xtlm import ipc_axis_master_util
from xilinx_xtlm import ipc_axis_slave_util
from xilinx_xtlm import xtlm_ipc
from xtlm_ethernet import *
import argparse
import datetime
import logging

ETH_NAME = "eth0"
INGRESS_PORT_A = 0
EGRESS_PORT = 1
INGRESS_PORT_B = 2

logging.basicConfig(level=logging.INFO)

arg_p = argparse.ArgumentParser(description="AAT hardware emulation script")
arg_p.add_argument("-i", "--inputFile", required=True, help="Input pcap file")
arg_p.add_argument("-o", "--outputFile", required=True, help="Output captured pcap file")
arg_p.add_argument("-s", "--size", required=False, help="Number of output packets that will be captured", default=None, type=int)

args = arg_p.parse_args()

# Declare ethernet hw_emu ifaces
aat_in_a = ipc_udp_ip_pcap(ETH_NAME, INGRESS_PORT_A, ipc_eth_type.SEND,
            sendPcapName = args.inputFile)

aat_in_b = ipc_udp_ip_pcap(ETH_NAME, INGRESS_PORT_B, ipc_eth_type.SEND,
            sendPcapName = args.inputFile)

aat_out = ipc_tcp_ip_pcap(ETH_NAME, EGRESS_PORT, ipc_eth_type.RECEIVE,
            recvPcapName = args.outputFile, recvSize = args.size)

# Perform TCP connection
if aat_out.tcp_passive_connect() != 0:
    sys.exit(-1)

print("Starting threads...")
aat_in_a.start_thread()
aat_in_b.start_thread()
aat_out.start_thread()

aat_in_a.join_thread()
aat_in_b.join_thread()
aat_out.join_thread()

with open('timestamp.log', 'w') as logfile:
    logfile.write(str(datetime.datetime.now()) + '\n')

print("Emulation finished!")
