/*
 * Copyright (c) 2018, Systems Group, ETH Zurich
 * Copyright (c) 2019-2020, Xilinx, Inc.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "udp.hpp"

/** @ingroup udp
 * Converts from ap_axi<176,0,0,0> to native udpMeta structure
 */
ipUdpMeta axiu2udpMeta(ap_axiu<176, 0, 0, 0> inword) {
#pragma HLS inline
    ipUdpMeta tmp_udpMeta;
    tmp_udpMeta.their_address = inword.data(127, 0);
    tmp_udpMeta.their_port = inword.data(143, 128);
    tmp_udpMeta.my_port = inword.data(159, 144);
    tmp_udpMeta.length = inword.data(175, 160);
    return tmp_udpMeta;
}

/** @ingroup udp
 * Converts from native udpMeta structure to ap_axi<176,0,0,0>
 */
ap_axiu<176, 0, 0, 0> udpMeta2axiu(ipUdpMeta inword) {
#pragma HLS inline
    ap_axiu<176, 0, 0, 0> tmp_axiu;
    tmp_axiu.data(127, 0) = inword.their_address;
    tmp_axiu.data(143, 128) = inword.their_port;
    tmp_axiu.data(159, 144) = inword.my_port;
    tmp_axiu.data(175, 160) = inword.length;
    tmp_axiu.last = 1;
    return tmp_axiu;
}

net_axis<64> axiu2word(ap_axiu<64, 0, 0, 0> inWord) {
    net_axis<64> tmpWord;
    tmpWord.data = inWord.data;
    tmpWord.last = inWord.last;
    tmpWord.keep = inWord.keep;
    return tmpWord;
}

/** @ingroup udp
 * Converts from ap_axi<64,0,0,0> to native ipMeta structure
 */
ipMeta axiu2ipMeta(ap_axiu<64, 0, 0, 0> inword) {
#pragma HLS inline
    ipMeta tmp_ipMeta;
    tmp_ipMeta.their_address = inword.data(31, 0);
    tmp_ipMeta.length = inword.data(47, 32);
    return tmp_ipMeta;
}

/** @ingroup udp
 * Converts from ipMeta to ap_axi<64,0,0,0> 
 */
ap_axiu<64,0,0,0> ipMeta2axiu(ipMeta inword) {
#pragma HLS inline
    ap_axiu<64,0,0,0> tmp_axiu;
    tmp_axiu.data(31, 0) = inword.their_address;
    tmp_axiu.data(47, 32) = inword.length;
    return tmp_axiu;
}

template <int WIDTH>
void process_udp(hls::stream<ap_axiu<WIDTH,0,0,0> >& input,
                 hls::stream<udpMeta>& metaOut,
                 hls::stream<uint16_t>& portOut,
                 hls::stream<net_axis<WIDTH> >& output,
                 uint32_t& datagrams_recv) {
#pragma HLS inline off
#pragma HLS pipeline II = 1

    static udpHeader<WIDTH> pu_header;
    static bool metaWritten = false;
    ap_axiu<WIDTH,0,0,0> inWord;
    net_axis<WIDTH> currWord;
    static bool validPkt = false;
    static uint32_t pktsRecv = 0;

    if (!input.empty()) {
        input.read(inWord);
        currWord.data = inWord.data;
        currWord.keep = inWord.keep;
        currWord.last = inWord.last;
        pu_header.parseWord(currWord.data);
        if (pu_header.isReady()) {
            if (!metaWritten) {
                // Check Dst Port
                uint16_t dstPort = pu_header.getDstPort();
                portOut.write(dstPort);
                std::cout << "UDP dst Port: " << (uint16_t)dstPort << std::endl;
                metaOut.write(udpMeta(pu_header.getSrcPort(), dstPort, pu_header.getLength() - 8, false));
                metaWritten = true;
            } else {
                output.write(currWord);
            }
        }
        
        if (currWord.last) {
            metaWritten = false;
            validPkt = true;
            pu_header.clear();
            pktsRecv++;
        }    
    }    
    datagrams_recv = pktsRecv;
}

/** @ingroup udp
 * The destination port lookup will take 2 clock cycles
 */
void lookup(hls::stream<uint16_t> &dstPort,
            hls::stream<bool> &validMeta,
            hls::stream<bool> &validData,
            uint8_t arrPorts[8192],
            uint32_t& datagrams_recv_invalid_port            
            ) {
    static uint32_t invalidPktsRecv = 0;                
    if (!dstPort.empty()) {
        uint16_t l_dstPort = dstPort.read();
        // Convert the port to a byte location in memory
        uint8_t bitField = arrPorts[l_dstPort / 8];        
        // Convert the port to our expected bit fieldWID
        uint8_t expectedBit = 1 << (l_dstPort % 8);
        bool fPortMatch = bitField & expectedBit;
        validMeta.write(fPortMatch);
        validData.write(fPortMatch);
        if (!fPortMatch) {
         invalidPktsRecv++;   
        }
    }
    datagrams_recv_invalid_port = invalidPktsRecv;            
}

/** @ingroup udp
 * Updates the udp header valid field
 */
void updateUdpMeta(hls::stream<udpMeta>& metaIn,              
                 hls::stream<bool>& validIn,
                 hls::stream<udpMeta>& metaOut
                 ) {

    udpMeta meta;
    if (!validIn.empty() && !metaIn.empty()) {
        metaIn.read(meta);
        meta.valid = validIn.read();
        metaOut.write(meta);
    }
}

template <int WIDTH>
void dropData(hls::stream<net_axis<WIDTH> >& input,
              hls::stream<bool>& validIn,
              hls::stream<net_axis<WIDTH> >& output
            ) {
#pragma HLS pipeline II = 1                
    enum fsmState { IDLE, FWD, DROP};
    static fsmState state = IDLE;
    net_axis<WIDTH> currWord;
    bool fValid = false;
    switch (state) {
        case IDLE:
            if (!validIn.empty()) {
                validIn.read(fValid);
                if (fValid) {
                    state = FWD;
                } else {
                    state = DROP;
                }
            }
            break;
        case FWD:
            if (!input.empty()) {
                currWord = input.read();
                output.write(currWord);
                if (currWord.last) {
                    state = IDLE;
                }
            }
            break;
        case DROP:
            if (!input.empty()) {
                currWord = input.read();
                if (currWord.last) {
                    state = IDLE;
                }
            }
            break;
    }
}




template <int WIDTH>
void generate_udp(hls::stream<udpMeta>& metaIn,
                  hls::stream<net_axis<WIDTH> >& input,
                  hls::stream<ap_axiu<64,0,0,0> >& output,
                  uint32_t& datagrams_transmitted) {
#pragma HLS inline off
#pragma HLS pipeline II = 1

    enum fsmState { META, HEADER, PARTIAL_HEADER, BODY };
    static fsmState state = META;
    static udpHeader<WIDTH> header;

    udpMeta meta;
    net_axis<WIDTH> currWord;
    uint8_t nBytesRem = 0;
    static uint32_t pktsSent = 0;
    ap_axiu<64,0,0,0> outWord;

    switch (state) {
        case META:
            if (!metaIn.empty()) {
                metaIn.read(meta);
                header.clear();
                header.setDstPort(meta.their_port);
                header.setSrcPort(meta.my_port);
                header.setLength(meta.length);
                if (UDP_HEADER_SIZE >= WIDTH) {
                    state = HEADER;
                } else {
                    state = PARTIAL_HEADER;
                }
            }
            break;
        case HEADER:
            nBytesRem = header.consumeWord(currWord.data);
            if (nBytesRem == 0) {
                state = BODY;
            } else if (nBytesRem < (WIDTH / 8)) {
                state = PARTIAL_HEADER;
            }
            currWord.keep = 0xFF;
            currWord.last = 0;
            outWord.data = currWord.data;
            outWord.keep = currWord.keep;
            outWord.last = currWord.last;
            output.write(outWord);
            break;
        case PARTIAL_HEADER:
            if (!input.empty()) {
                input.read(currWord);
                header.consumeWord(currWord.data);
                outWord.data = currWord.data;
                outWord.keep = currWord.keep;
                outWord.last = currWord.last;
                output.write(outWord);
                state = BODY;
                if (currWord.last) {
                    pktsSent++;
                    state = META;
                }
            }
            break;
        case BODY:
            if (!input.empty()) {
                input.read(currWord);
                outWord.data = currWord.data;
                outWord.keep = currWord.keep;
                outWord.last = currWord.last;
                output.write(outWord);
                if (currWord.last) {
                    pktsSent++;
                    state = META;
                }
            }
            break;
    }
    datagrams_transmitted = pktsSent;
}

/** @ingroup udp
 * Splits the struct from the user interface into IP Metadata and UDP Metadata
 */
void split_tx_meta(hls::stream<ap_axiu<176, 0, 0, 0> >& metaIn,
                   hls::stream<ap_axiu<64,0,0,0> >& ipMetaOut,
                   hls::stream<udpMeta>& udpMetaOut) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    ipUdpMeta meta;
    if (!metaIn.empty()) {
        meta = axiu2udpMeta(metaIn.read());
        // Add 8 bytes for UDP header
        ap_uint<16> tempLen = meta.length + 8;
        ipMetaOut.write(ipMeta2axiu(ipMeta(meta.their_address, tempLen)));
        udpMetaOut.write(udpMeta(meta.their_port, meta.my_port, tempLen));
    }
}

/** @ingroup udp
 * Combines the IP Metadata and UDP Metadata into the user interface structure.
 */
void merge_rx_meta(hls::stream<ap_axiu<64,0,0,0> >& ipMetaIn,
                   hls::stream<udpMeta>& udpMetaIn,
                   hls::stream<ap_axiu<176, 0, 0, 0> >& metaOut) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    ipMeta l_ipMeta;
    udpMeta l_udpMeta;
    if (!ipMetaIn.empty() && !udpMetaIn.empty()) {
        l_ipMeta = axiu2ipMeta(ipMetaIn.read());
        udpMetaIn.read(l_udpMeta);
        if (l_udpMeta.valid) {
            metaOut.write(udpMeta2axiu(
                ipUdpMeta(l_ipMeta.their_address, l_udpMeta.their_port, l_udpMeta.my_port, l_udpMeta.length)));
        }
    }
}

/**
 * @ingroup udp
 *
 *
 * @tparam WIDTH Data bus width for the UDP function.
 * @param [in]  s_axis_rx_meta
 * @param [in]  s_axis_rx_data
 * @param [out] m_axis_rx_meta
 * @param [out] m_axis_rx_data
 * @param [in]  s_axis_tx_meta
 * @param [in]  s_axis_tx_data
 * @param [out] m_axis_tx_meta
 * @param [in]  m_axis_tx_data
 * @param [in]  arrPorts
 * @param [out] stats
 */
template <int WIDTH>
void udp_body(hls::stream<ap_axiu<64,0,0,0> >& s_axis_rx_meta,
              hls::stream<ap_axiu<64,0,0,0> >& s_axis_rx_data,
              hls::stream<ap_axiu<176, 0, 0, 0> >& m_axis_rx_meta,
              hls::stream<ap_axiu<64,0,0,0> >& m_axis_rx_data,
              hls::stream<ap_axiu<176, 0, 0, 0> >& s_axis_tx_meta,
              hls::stream<ap_axiu<64,0,0,0> >& s_axis_tx_data,
              hls::stream<ap_axiu<64,0,0,0> >& m_axis_tx_meta,
              hls::stream<ap_axiu<64,0,0,0> >& m_axis_tx_data,
              uint32_t &datagrams_transmitted,
              uint32_t &datagrams_recv,
              uint32_t &datagrams_recv_invalid_port,
              uint8_t arrPorts[8192]) {
#pragma HLS INLINE

    /*
     * RX PATH
     */
    static hls::stream<net_axis<WIDTH> > rx_udp2shiftFifo("rx_udp2shiftFifo");
    static hls::stream<net_axis<WIDTH> > rx_udp2dropFifo("rx_udp2dropFifo");
    static hls::stream<udpMeta> rx_udpMetaFifo("rx_udpMetaFifo");
    static hls::stream<udpMeta> rx_udpMeta2Merge("rx_udpMeta2Merge");
    static hls::stream<net_axis<WIDTH> > rxUdpData_in("rxUdpData_in");
    static hls::stream<ipMeta> rxIpMetaUdp("rxIpMetaUdp");
    static hls::stream<uint16_t> dstPort2lookup("dstPort2lookup");
    static hls::stream<bool> valid2update("valid2update");
    static hls::stream<bool> valid2drop("valid2drop");
#pragma HLS STREAM depth = 2 variable = rx_udp2shiftFifo
#pragma HLS STREAM depth = 3 variable = rx_udpMetaFifo
#pragma HLS STREAM depth = 2 variable = rxUdpData_in
#pragma HLS STREAM depth = 2 variable = rxIpMetaUdp
#pragma HLS STREAM depth = 2 variable = dstPort2lookup
#pragma HLS STREAM depth = 2 variable = rx_udpMetaFifo
#pragma HLS STREAM depth = 3 variable = rx_udp2dropFifo
#pragma HLS STREAM depth = 2 variable = valid2update
#pragma HLS STREAM depth = 2 variable = valid2drop

    process_udp(s_axis_rx_data, rx_udpMetaFifo, dstPort2lookup, rx_udp2dropFifo, datagrams_recv);
    lookup(dstPort2lookup,valid2update,valid2drop,arrPorts,datagrams_recv_invalid_port);
    updateUdpMeta(rx_udpMetaFifo, valid2update, rx_udpMeta2Merge);
    dropData(rx_udp2dropFifo,valid2drop,rx_udp2shiftFifo);
    rshiftWordByOctet<AXI_WIDTH, net_axis<WIDTH>, ap_axiu<WIDTH,0,0,0>, 2>(((UDP_HEADER_SIZE % AXI_WIDTH) / 8), rx_udp2shiftFifo,
                                                          m_axis_rx_data);
    merge_rx_meta(s_axis_rx_meta, rx_udpMeta2Merge, m_axis_rx_meta);

    /*
     * TX PATH
     */
    static hls::stream<net_axis<WIDTH> > tx_shift2udpFifo("tx_shift2udpFifo");
    static hls::stream<net_axis<WIDTH> > tx_udp2shiftFifo("tx_udp2shiftFifo");
    static hls::stream<net_axis<WIDTH> > txUdpData_out("txUdpData_out");
    static hls::stream<udpMeta> tx_udpMetaFifo("tx_udpMetaFifo");
    static hls::stream<ipMeta> txIpMetaUdp("txIpMetaUdp");
#pragma HLS STREAM depth = 2 variable = tx_shift2udpFifo
#pragma HLS STREAM depth = 2 variable = tx_udp2shiftFifo
#pragma HLS STREAM depth = 2 variable = tx_udpMetaFifo
#pragma HLS STREAM depth = 2 variable = txIpMetaUdp
#pragma HLS STREAM depth = 2 variable = txUdpData_out

    split_tx_meta(s_axis_tx_meta, m_axis_tx_meta, tx_udpMetaFifo);
    lshiftWordByOctet<WIDTH,ap_axiu<WIDTH,0,0,0>, net_axis<WIDTH>, 1>(((UDP_HEADER_SIZE % AXI_WIDTH) / 8), s_axis_tx_data, tx_shift2udpFifo);
    generate_udp<WIDTH>(tx_udpMetaFifo, tx_shift2udpFifo, m_axis_tx_data, datagrams_transmitted);
};

/**
 * @ingroup udp
 *
 * Simple wrapper around udp_body() to set the WIDTH parameter.
 *
 * @param [in]  s_axis_rx_meta
 * @param [in]  s_axis_rx_data
 * @param [out] m_axis_rx_meta
 * @param [out] m_axis_rx_data
 * @param [in]  s_axis_tx_meta
 * @param [in]  s_axis_tx_data
 * @param [out] m_axis_tx_meta
 * @param [in]  m_axis_tx_data
 * @param [in]  arrPorts
 * @param [out] stats
 */
void udp(hls::stream<ap_axiu<64,0,0,0> >& s_axis_rx_meta,
         hls::stream<ap_axiu<64,0,0,0> >& s_axis_rx_data,
         hls::stream<ap_axiu<176, 0, 0, 0> >& m_axis_rx_meta,
         hls::stream<ap_axiu<64,0,0,0> >& m_axis_rx_data,
         hls::stream<ap_axiu<176, 0, 0, 0> >& s_axis_tx_meta,
         hls::stream<ap_axiu<64,0,0,0> >& s_axis_tx_data,
         hls::stream<ap_axiu<64,0,0,0> >& m_axis_tx_meta,
         hls::stream<ap_axiu<64,0,0,0> >& m_axis_tx_data,
         uint32_t &datagrams_transmitted,
         uint32_t &datagrams_recv,
         uint32_t &datagrams_recv_invalid_port,
         uint8_t arrPorts[8192]) {
#pragma HLS DATAFLOW disable_start_propagation
#pragma HLS INTERFACE ap_ctrl_none port = return

#pragma HLS INTERFACE axis register port = s_axis_rx_meta
#pragma HLS INTERFACE axis register port = s_axis_rx_data
#pragma HLS INTERFACE axis register port = m_axis_rx_meta
#pragma HLS INTERFACE axis register port = m_axis_rx_data
#pragma HLS INTERFACE axis register port = s_axis_tx_meta
#pragma HLS INTERFACE axis register port = s_axis_tx_data
#pragma HLS INTERFACE axis register port = m_axis_tx_meta
#pragma HLS INTERFACE axis register port = m_axis_tx_data
#pragma HLS INTERFACE s_axilite port = arrPorts bundle = control
#pragma HLS INTERFACE s_axilite port = datagrams_transmitted bundle = control
#pragma HLS INTERFACE s_axilite port = datagrams_recv bundle = control
#pragma HLS INTERFACE s_axilite port = datagrams_recv_invalid_port bundle = control

    udp_body<64>(s_axis_rx_meta, s_axis_rx_data, m_axis_rx_meta, m_axis_rx_data, s_axis_tx_meta, s_axis_tx_data,
                 m_axis_tx_meta, m_axis_tx_data, datagrams_transmitted, datagrams_recv, datagrams_recv_invalid_port, arrPorts);
}
