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
from scapy.all import *
from abc import ABC, abstractmethod

import socket
import logging
import threading
import traceback
import random
import enum

class ipc_eth_type(enum.Enum):
    '''
    SEND: This will connect the simulation module to a data path from tb to dut
    '''
    SEND = 1
    '''
    RECEIVE: This will connect the simulation module to a data path from dut to tb
    '''
    RECEIVE = 2

class _ipc_eth_base(ABC):
    def __init__(self, ethName, port, ethType, recvSize, busWidthBytes = 8):
        self.ethName = str(ethName)
        self.port = str(port)
        self.rx_name = self.ethName + "_rx" + self.port + "_axis"
        self.tx_name = self.ethName + "_tx" + self.port + "_axis"
        self.ethType = ethType
        if ethType == ipc_eth_type.SEND:
            self.tx_axis = ipc_axis_slave_util(self.tx_name)
            self.rx_axis = ipc_axis_master_util(self.rx_name)
        else:
            self.tx_axis = ipc_axis_master_util(self.tx_name)
            self.rx_axis = ipc_axis_slave_util(self.rx_name)
        self.isSend = ethType == ipc_eth_type.SEND
        self.recvSize = int(recvSize)
        self.busWidthBytes = busWidthBytes
        self.send_thread = threading.Thread(target=self._send_thread)
        self.recv_thread = threading.Thread(target=self._recv_thread)

    def __str__(self):
        return str(self.ethName) + ":" + str(self.port)

    def __del__(self):
        self.tx_axis.disconnect()
        self.rx_axis.disconnect()

    def _transmit_packet(self, strm, data):
        '''
        Utility function to slice and transfer payload data through an ipc xtlm stream
        '''
        length = len(data)
        i = 0
        busW = self.busWidthBytes
        while length > 0:
            word = xtlm_ipc.axi_stream_packet()
            word.data_length = min(length, busW)
            length -= word.data_length
            word.tlast = (length == 0)
            word.data = data[(busW * i) : (busW * i) + word.data_length]
            # tkeep setting will contain a string of 0xFF bytes 'data_length' wide.
            # e.g. data_length(3) -> tkeep = b'\xFF\xFF\xFF'
            word.tkeep = b'\xFF' * word.data_length
            strm.b_transport(word)
            i += 1

    @abstractmethod
    def _send_thread(self):
        # To be overriden by child classes
        pass

    @abstractmethod
    def _recv_thread(self):
        # To be overriden by child classes
        pass

    @abstractmethod
    def _get_next_packet(self):
        # To be overriden by child classes
        pass

    @abstractmethod
    def _get_num_send_packets(self):
        # To be overriden by child classes
        pass

    @abstractmethod
    def _process_out_packet(self, data):
        # To be overriden by child classes
        pass

    def start_thread(self):
        self.send_thread.start()
        self.recv_thread.start()

    def join_thread(self):
        self.send_thread.join()
        self.recv_thread.join()


class _ipc_tcp_ip_base(_ipc_eth_base):
    def __init__(self, ethName, port, ethType, recvSize, busWidthBytes = 8):
        _ipc_eth_base.__init__(self, ethName, port, ethType, recvSize, busWidthBytes)
        logging.info(" +++ Creating TCP ipc %s. +++", self)
        # Set sequence number to a random starting seq.
        self.seq = random.randrange(0, (2 ** 32) - 1)

    def _tcp_send(self, strm, ether_packet):
        # Recalculate checksum, scapy packets won't recalculate the checksum on the fly, so before we send
        # the packet, delete the fields for the TCP and IP checksum and recreate the packet, scapy will
        # calculate the checksum then.
        del ether_packet[IP].chksum
        del ether_packet[TCP].chksum
        pkt = ether_packet.__class__(bytes(ether_packet))

        pkt_raw = bytes(pkt)
        self._transmit_packet(strm, pkt_raw)

    def _send_thread(self):
        # TODO - Not implemented
        logging.info("[%s] TCP send thread finishing.", self)

    def _recv_thread(self):
        logging.info("[%s] TCP recv thread starting.", self)
        data_strm = self.tx_axis if self.isSend else self.rx_axis
        ack_strm = self.rx_axis if self.isSend else self.tx_axis
        p_id = 0
        while p_id < self.recvSize:
            # Receive TCP packet
            packet = data_strm.sample_transaction().data
            p = Ether(packet)
            self._process_out_packet(p)
            p_id += 1
            logging.info("[%s] TCP recv thread received packet #%d/%d (%.2f %%)", self, p_id, self.recvSize,
                (float(p_id) / self.recvSize) * 100)
            # Prepare ACK message
            ack = p[TCP].seq + len(p[Raw])
            if p.haslayer(Padding):
                ack -= len(p[Padding])
            ack_pkt = Ether(src=p[Ether].dst, dst=p[Ether].src) / IP(dst=p[IP].src, src=p[IP].dst) / TCP(dport=p[TCP].sport,
                    sport=p[TCP].dport, ack=ack, seq=self.seq, flags='A')
            self._tcp_send(ack_strm, ack_pkt)
        logging.info("[%s] TCP recv thread finishing.", self)

    def tcp_passive_connect(self):
        if self.ethType != ipc_eth_type.RECEIVE:
            raise ValueError("Trying to stablish a passive connection on an active TCP module."
                + " Please use tcp_active_connect instead.")
        logging.info("Setting up connection...")

        # Wait until SYN message from TCP IP
        packet = self.rx_axis.sample_transaction()
        syn = Ether(packet.data)
        logging.info("Received SYN message")
        if TCP not in syn or (syn[TCP].flags & 0x2 != 0x2):
            logging.error("Received message is not SYN message!")
            syn.show()
            return -1

        # Prepare SYN-ACK packet
        syn_ack = Ether(src=syn[Ether].dst, dst=syn[Ether].src) / IP(dst=syn[IP].src, src=syn[IP].dst) / TCP(dport=syn[TCP].sport,
            sport=syn[TCP].dport, ack=syn[TCP].seq + 1, seq=self.seq, flags='SA')
        self.seq += 1
        logging.info("Sending SYN_ACK message")
        self._tcp_send(self.tx_axis, syn_ack)

        # Receive ACK
        self.rx_axis.sample_transaction()
        logging.info("Connection established!")
        return 0

    def tcp_active_connect(self,srcMacAddr,dstMacAddr,srcIpAddr,dstIpAddr,source_prt,dest_prt):
        if self.ethType != ipc_eth_type.SEND:
            raise ValueError("Trying to stablish an active connection on a passive TCP module."
                + " Please use tcp_passive_connect instead.")

        ether = Ether(src=srcMacAddr, dst=dstMacAddr)
        ip = IP(dst=dstIpAddr, src=srcIpAddr)
        tcp_syn = TCP(sport=source_prt,dport=dest_prt, ack=0, seq=self.seq_tx, flags='S')

        # Send SYN message to TCP IP
        logging.info("Sending SYN message")
        syn = ether / ip / tcp_syn
        # Un-comment to show syn message
        # syn.show()
        self._tcp_send(self.rx_axis, syn)

        # Wait for SYN-ACK packet
        packet = self.tx_axis.sample_transaction()
        syn_ack = Ether(packet.data)
        logging.info("Received SYN-ACK message")
        if TCP not in syn_ack or (syn_ack[TCP].flags & 0x12 != 0x12) or (syn_ack[TCP].ack !=self.seq_tx+1):
            logging.error("Received message is not SYN-ACK message!")
            syn_ack.show()
            return -1
        self.seq_rx=syn_ack[TCP].seq

        # Send ACK
        tcp_ack = TCP(sport=source_prt,dport=dest_prt, seq=self.seq_tx, ack=self.seq_rx+1, flags='A')
        ack = ether / ip / tcp_ack
        logging.info("Sending ACK message")
        # Un-comment to show ack message
        # ack.show()
        self._tcp_send(self.rx_axis, ack)
        return 0



class _ipc_udp_ip_base(_ipc_eth_base):
    def __init__(self, ethName, port, ethType, recvSize, busWidthBytes = 8):
        _ipc_eth_base.__init__(self, ethName, port, ethType, recvSize, busWidthBytes)
        logging.info(" +++ Creating UDP ipc %s. +++", self)

    def _send_thread(self):
        logging.info("[%s] UDP send thread starting.", self)
        data_strm = self.rx_axis if self.isSend else self.tx_axis

        totalPackets = self._get_num_send_packets()
        p_id = 0
        for p_id in range(totalPackets):
            buf = bytes(self._get_next_packet())
            logging.info("[%s] Sending UDP packet %d / %d (len = %d)", self, p_id, totalPackets, len(buf))
            self._transmit_packet(data_strm, buf)
        logging.info("[%s] UDP send thread finishing.", self)

    def _recv_thread(self):
        logging.info("[%s] UDP recv thread starting.", self)
        data_strm = self.tx_axis if self.isSend else self.rx_axis
        p_id = 0
        while p_id < self.recvSize:
            packet = data_strm.sample_transaction().data
            p_id += 1
            logging.info("[%s] UDP recv received packet %d/%d (%.2f %%).", self, p_id, self.recvSize,
                    (float(p_id) / self.recvSize) * 100)
            udp_packet = Ether(packet)
            self._process_out_packet(udp_packet)
        logging.info("[%s] UDP recv thread finishing.", self)


'''
Class for emulating UDP IP module with pcap stimulus
'''
class ipc_udp_ip_pcap(_ipc_udp_ip_base):
    def __init__(self, ethName, port, ethType, sendPcapName = None, recvPcapName = None, recvSize = 0, busWidthBytes = 8):
        _ipc_udp_ip_base.__init__(self, ethName, port, ethType, recvSize, busWidthBytes)
        self.sendPcapName = sendPcapName
        self.recvPcapName = recvPcapName

        self.sendPcapData = rdpcap(self.sendPcapName) if self.sendPcapName is not None else []
        self.totalSendPackets = len(self.sendPcapData)
        self.sendPcapIdx = 0

    def _get_next_packet(self):
        p = self.sendPcapData[self.sendPcapIdx]
        self.sendPcapIdx += 1
        return p

    def _get_num_send_packets(self):
        return self.totalSendPackets

    def _process_out_packet(self, data):
        if self.recvPcapName is not None:
            wrpcap(self.recvPcapName, data, append=True)


'''
Class for emulating TCP IP module with pcap stimulus
'''
class ipc_tcp_ip_pcap(_ipc_tcp_ip_base):
    def __init__(self, ethName, port, ethType, sendPcapName = None, recvPcapName = None, recvSize = None, busWidthBytes = 8):
        _ipc_tcp_ip_base.__init__(self, ethName, port, ethType, recvSize, busWidthBytes)
        self.sendPcapName = sendPcapName
        self.recvPcapName = recvPcapName

        self.sendPcapData = rdpcap(self.sendPcapName) if self.sendPcapName is not None else []
        self.totalSendPackets = len(self.sendPcapData)
        self.sendPcapIdx = 0

    def _get_next_packet(self):
        p = self.sendPcapData[self.sendPcapIdx]
        self.sendPcapIdx += 1
        return p

    def _get_num_send_packets(self):
        return self.totalSendPackets

    def _process_out_packet(self, data):
        if self.recvPcapName is not None:
            wrpcap(self.recvPcapName, data, append=True)


'''
Class for emulation UDP IP send module with proceduraly generated stimulus with random waits between packets
'''
class ipc_udp_ip_rnd_gen(_ipc_udp_ip_base):
    def __init__(self, ethName, port, sendSize, minRandWait = 0, maxRandWait = 0, recvSize = 0, busWidthBytes = 8):
        _ipc_udp_ip_base.__init__(self, ethName, port, ipc_eth_type.SEND, recvSize, busWidthBytes)
        self.sendSize = sendSize
        self.minRandWait = int(min(minRandWait, maxRandWait))
        self.maxRandWait = int(max(maxRandWait, minRandWait))
        self.waitDelta = self.maxRandWait - self.minRandWait

    def _get_num_send_packets(self):
        return self.sendSize

    def _get_next_packet(self):
        if self.waitDelta > 0:
            waitMs = random.randrange(self.minRandWait, self.maxRandWait)
        else:
            waitMs = self.waitDelta
        if waitMs > 0:
            time.sleep(float(waitMs) / 1000.0)
        return self._create_next_packet()

    @abstractmethod
    def _create_next_packet(self):
        # To be overriden by child classes
        pass

    def _process_out_packet(self, data):
        # To be overriden by child classes, if needed
        pass

'''
Utility class to create a UDP packet with a specified payload
'''
def create_udp_packet(dstMacAddr, dstIpAddr, dstUdpPort, payload,
        srcMacAddr = "05:05:05:05:05:05", srcIpAddr="192.168.0.1", srcUdpPort=10000):
    ether = Ether(src=srcMacAddr, dst=dstMacAddr)
    ip = IP(dst=dstIpAddr, src=srcIpAddr)
    udp = UDP(dport=dstUdpPort, sport=srcUdpPort)
    pld = Raw(load=payload)
    pkt = ether / ip / udp / pld
    return pkt
