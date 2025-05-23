/*
 * Copyright (c) 2016, 2019-2020, Xilinx, Inc.
 * Copyright (c) 2019, Systems Group, ETH Zurich
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE#
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "rx_engine.hpp"

/** @ingroup rx_engine
 * Converts from native ap_axi<8,0,0,0> structure to mmStatus
 */
mmStatus rx_axiu2mmStatus(ap_axiu<8,0,0,0> inword) {
#pragma HLS inline
    mmStatus tmp_mmStatus;
    tmp_mmStatus.tag = inword.data(3,0);
    tmp_mmStatus.interr = inword.data(4,4);
    tmp_mmStatus.decerr = inword.data(5,5);
    tmp_mmStatus.slverr = inword.data(6,6);
    tmp_mmStatus.okay = inword.data(7,7);
    return tmp_mmStatus;
}

/** @ingroup rx_engine
 * Converts from mmCmd structure to ap_axi<128,0,0,0>
 */
ap_axiu<128,0,0,0> rx_mmCmd2axiu2(mmCmd inword) {
#pragma HLS inline
    ap_axiu<128,0,0,0> tmp_axiu;
    tmp_axiu.data(22,0) = inword.bbt;
    tmp_axiu.data(23,23) = inword.type;
    tmp_axiu.data(29,24) = inword.dsa;
    tmp_axiu.data(30,30) = inword.eof;
    tmp_axiu.data(31,31) = inword.drr;
    tmp_axiu.data(63,32) = inword.saddr;
    tmp_axiu.data(67,64) = inword.tag;
    tmp_axiu.data(71,68) = inword.rsvd;
    tmp_axiu.last = 1;
    return tmp_axiu;
}

/**
 * Convert a network order IPV4 address to string representation
 * @param[in] in - IPV4 Address (network order)
 * @return String representation in numbers and dot format
 **/
std::string rxeng_inet_ntoa(ap_uint<32> in) {
    return (std::to_string(in(7, 0)) + "." + std::to_string(in(15, 8)) + "." + std::to_string(in(23, 16)) + "." +
            std::to_string(in(31, 24)));
}

/**
 * Convert network order long to 'host order'
 * @param[in] netlong
 * @return hostlong
 **/
uint32_t rxeng_ntohl(ap_uint<32> netlong) {
    ap_uint<32> hostlong;
    hostlong(7, 0) = netlong(31, 24);
    hostlong(15, 8) = netlong(23, 16);
    hostlong(23, 16) = netlong(15, 8);
    hostlong(31, 24) = netlong(7, 0);
    return hostlong;
}

/**
 * Convert network order short to 'host order'
 * @param[in] netshort
 * @return hostshort
 **/
uint16_t rxeng_ntohs(ap_uint<16> netshort) {
    ap_uint<16> hostshort;
    hostshort(7, 0) = netshort(15, 8);
    hostshort(15, 8) = netshort(7, 0);
    return hostshort;
}

/**
 * Print the TCP Header (debug)
 * @param[in] meta TCP metadata
 * @param[in] tuple Fourtuple of ports and IP addresses
 **/
void rxeng_printTCPHeader(rxEngineMetaData meta, fourTuple tuple) {
#ifndef __SYNTHESIS__
    std::cout << "Rx Engine: TCP Header" << std::dec << std::endl;
    std::cout << "Source Port: " << rxeng_ntohs(tuple.srcPort) << std::endl;
    std::cout << "Dest Port  : " << rxeng_ntohs(tuple.dstPort) << std::endl;
    std::cout << "Sequence   : " << meta.seqNumb << std::endl;
    std::cout << "Ack Number : " << meta.ackNumb << std::endl;
    std::cout << "Data Offset: " << meta.dataOffset << std::endl;
    std::cout << "Window Size: " << meta.winSize << std::endl;
    std::cout << "SYN        : " << meta.syn << std::endl;
    std::cout << "ACK        : " << meta.ack << std::endl;
    std::cout << "RST        : " << meta.rst << std::endl;
    std::cout << "FIN        : " << meta.fin << std::endl;
#endif
}

/**
 * Determines if the TCP is in a non-Synchronized state.
 * returns true if the fsm state is an 'unsynchronized state', false otherwise.
 * @param[in] fsmstate
 * @return fResult
 **/
bool nonSynchronized(sessionState fsmstate) {
#pragma HLS inline
    bool fResult = false;

    if ((fsmstate == SYN_SENT) || (fsmstate == SYN_RECEIVED) || (fsmstate == CLOSED)) {
        fResult = true;
    }

    return fResult;
}

template <int WIDTH>
void rxeng_printIPv4(ipv4Header<WIDTH> header) {
#ifndef __SYNTHESIS__
    std::cout << std::hex << "Rx Engine: IPV4: SrcIP: " << rxeng_inet_ntoa(header.getSrcAddr())
              << " DestIP: " << rxeng_inet_ntoa(header.getDstAddr()) << " TotLen:" << header.getLength() << std::endl;
#endif
}

/** @ingroup rx_engine
 * Block parses IP header, and removes it
 *  @param[in]      dataIn
 *  @param[out]     metaOut
 *  @param[out]     dataOut
 */
template <int WIDTH>
void process_ipv4(hls::stream<ap_axiu<WIDTH,0,0,0> >& dataIn,
                  hls::stream<pseudoMeta>& metaOut,
                  hls::stream<net_axis<WIDTH> >& dataOut) {
#pragma HLS INLINE off
#pragma HLS pipeline II = 1

    static ipv4Header<WIDTH> header;
    static ap_uint<4> headerWordsDropped = 0;
    ap_axiu<64,0,0,0> inWord;

// ------------------ NEW ------------------------------------
    enum fsmStateType { META, DROP, BODY, SHIFT, LAST};
    static fsmStateType doh_state = META;
    static ap_uint<4> length;

    static net_axis<WIDTH> prevWord;
    net_axis<WIDTH> sendWord;

    switch (doh_state) {
        case META:
            if (!dataIn.empty()) {
                dataIn.read(inWord);
                prevWord.data = inWord.data;
                prevWord.keep = inWord.keep;
                prevWord.last = inWord.last;
                header.parseWord(prevWord.data);
                if (header.isReady()) {
                    length = header.getHeaderLength() - headerWordsDropped;
                    metaOut.write(pseudoMeta(header.getSrcAddr(), header.getDstAddr(),
                                             header.getLength() - header.getHeaderLength() * 4));
                    if (header.getHeaderLength() > 5) {
                        doh_state = DROP;
                    } else {
                        doh_state = SHIFT;
                    }
                }
                headerWordsDropped += (WIDTH / 32);
            }
            break;
        case DROP:
            if (!dataIn.empty()) {
                dataIn.read(inWord);
                prevWord.data = inWord.data;
                prevWord.keep = inWord.keep;
                prevWord.last = inWord.last;
                length -= 2;
                if (length == 1) {
                    doh_state = SHIFT;
                } else if (length == 0) {
                    doh_state = BODY;
                }
            }
            break;
        case BODY:
            if (!dataIn.empty()) {
                dataIn.read(inWord);
                sendWord.data = inWord.data;
                sendWord.keep = inWord.keep;
                sendWord.last = inWord.last;
                dataOut.write(sendWord);
                if (inWord.last) {
                    doh_state = META;
                    header.clear();
                    headerWordsDropped = 0;
                }
            }
            break;
        case SHIFT:
            if (!dataIn.empty()) {
                dataIn.read(inWord);
                sendWord.data(WIDTH - 32 - 1, 0) = prevWord.data(WIDTH - 1, 32);
                sendWord.keep((WIDTH / 8) - 4 - 1, 0) = prevWord.keep((WIDTH / 8) - 1, 4);
                sendWord.data(WIDTH - 1, WIDTH - 32) = inWord.data(31, 0);
                sendWord.keep((WIDTH / 8) - 1, (WIDTH / 8) - 4) = inWord.keep(3, 0);
                sendWord.last = (inWord.keep[4] == 0);
                dataOut.write(sendWord);
                prevWord.data = inWord.data;
                prevWord.keep = inWord.keep;
                prevWord.last = inWord.last;
                if (sendWord.last) {
                    doh_state = META;
                    header.clear();
                    headerWordsDropped = 0;
                } else if (inWord.last) {
                    doh_state = LAST;
                }
            }
            break;

        case LAST:
            sendWord.data(WIDTH - 32 - 1, 0) = prevWord.data(WIDTH - 1, 32);
            sendWord.keep((WIDTH / 8) - 4 - 1, 0) = prevWord.keep((WIDTH / 8) - 1, 4);
            sendWord.data(WIDTH - 1, WIDTH - 32) = 0;
            sendWord.keep((WIDTH / 8) - 1, (WIDTH / 8) - 4) = 0x0;
            sendWord.last = 0x1;
            dataOut.write(sendWord);
            doh_state = META;
            header.clear();
            headerWordsDropped = 0;
            break;
    } // switch
}

/** @ingroup rx_engine
 * Block converts the metadata struct to a network ordered stream of words
 * The pseudo-header consists of the source IP address, the destination IP address, 
 * the protocol number for the TCP protocol (6) and the length of the TCP headers 
 * and payload (in bytes). The Pseudo header is then appened to the data stream 
 *  @param[in]      ipMetaIn
 *  @param[in]      packetIn
 *  @param[out]     output
 */
template <int WIDTH>
void prependPseudoHeader(hls::stream<pseudoMeta>& ipMetaIn,
                         hls::stream<net_axis<WIDTH> >& packetIn,
                         hls::stream<net_axis<WIDTH> >& output) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    enum fsmState { META, HEADER, PAYLOAD };
    static fsmState state = META;
    static net_axis<WIDTH> prevWord;
    static bool firstPayload = true;
    ap_uint<8> remainingLength;
    static tcpPseudoHeader<WIDTH> header; // size 96bits

    switch (state) {
        case META:
            if (!ipMetaIn.empty()) {
                pseudoMeta meta = ipMetaIn.read();
                header.clear();
                header.setSrcAddr(meta.their_address);
                header.setDstAddr(meta.our_address);
                header.setLength(meta.length);
                state = HEADER;
            }
            break;

        case HEADER:
            remainingLength = header.consumeWord(prevWord.data);
            prevWord.keep = ~0;
            prevWord.last = (remainingLength == 0);

            if (!prevWord.last) {
                output.write(prevWord);
            } else {
                prevWord.last = 0;
                firstPayload = true; 
                state = PAYLOAD;
            }
            break;

        case PAYLOAD:
            if (!packetIn.empty()) {
                net_axis<WIDTH> currWord = packetIn.read();

                net_axis<WIDTH> sendWord = currWord;
                if (firstPayload) {
                    if (WIDTH == 64) {
                        sendWord.data(31, 0) = prevWord.data(31, 0);
                    } else {
                        sendWord.data(95, 0) = prevWord.data(95, 0);
                    }
                    firstPayload = false;
                }

                output.write(sendWord);

                if (currWord.last) {
                    state = META;
                }
            }
            break;
    }
}

/** @ingroup rx_engine

 *  @param[in]      dataIn
 *  @param[out]     dataOut
 *  @param[out]     validFifoOut
 *  @param[out]     metaDataFifoOut
 *  @param[out]     tupleFifoOut
 *  @param[out]     portTableOut
 *  @param[out]     ofMetaOut
 */
template <int WIDTH>
void processPseudoHeader(hls::stream<net_axis<WIDTH> >& dataIn,
                         hls::stream<net_axis<WIDTH> >& dataOut,
                         hls::stream<bool>& validFifoIn,
                         hls::stream<rxEngineMetaData>& metaDataFifoOut,
                         hls::stream<fourTuple>& tupleFifoOut,
                         hls::stream<ap_uint<16> >& portTableOut,
                         hls::stream<optionalFieldsMeta>& ofMetaOut,
                         hls::stream<ap_uint<1> >& invalidSeg,
                         hls::stream<ap_uint<1> >& receivedSeg) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    static tcpFullPseudoHeader<WIDTH> header; // width: 256bit
    static bool firstWord = true;
    static bool pkgValid = false;
    static bool metaWritten = false;
    // static uint32_t invalidRecv = 0;
    // static uint32_t recvSeg = 0;

    if (!dataIn.empty() && (!firstWord || !validFifoIn.empty())) {
        net_axis<WIDTH> word = dataIn.read();
        header.parseWord(word.data);
        // When reading the first word we need to ensure that the pkg is valid
        if (firstWord) {
            validFifoIn.read(pkgValid);
            firstWord = false;
            receivedSeg.write(1);
        }

        // Once TCP Header has been processed output payload
        if (metaWritten && pkgValid && WIDTH <= TCP_FULL_PSEUDO_HEADER_SIZE) {
            dataOut.write(word);
        }

        if (header.isReady()) {
            // TCP does not accept broadcast or multicast
            if (header.dstIsMulticast() || header.dstIsBroadcast() || header.srcIsMulticast() ||
                header.srcIsBroadcast()) {
                pkgValid = false;
            }
            // Current 64-bit confioguration will never enter this.
            if (pkgValid && WIDTH > TCP_FULL_PSEUDO_HEADER_SIZE &&
                ((header.getLength() - (header.getDataOffset() * 4) != 0 ||
                  (header.getDataOffset() > 5)))) // && WIDTH == 512)
            {
                dataOut.write(word);
            }
            if (!metaWritten) {
                rxEngineMetaData meta;
                meta.seqNumb = header.getSeqNumb();
                meta.ackNumb = header.getAckNumb();
                meta.winSize = header.getWindowSize();
                meta.length = header.getLength() - (header.getDataOffset() * 4);
                meta.ack = header.getAckFlag();
                meta.rst = header.getRstFlag();
                meta.syn = header.getSynFlag();
                meta.fin = header.getFinFlag();
                meta.dataOffset = header.getDataOffset();
                if (pkgValid) {
                    metaDataFifoOut.write(meta);             // Write out metadata on metaDataFifoOut
                    portTableOut.write(header.getDstPort()); // Send the destination port number to the port_table block
                    // Tuple entries are in NO
                    tupleFifoOut.write(fourTuple(reverse(header.getSrcAddr()), reverse(header.getDstAddr()),
                                                 reverse(header.getSrcPort()), reverse(header.getDstPort())));
                    if (meta.length != 0 || header.getDataOffset() > 5) {
                        ofMetaOut.write(optionalFieldsMeta(header.getDataOffset() - 5, header.getSynFlag()));
                    }
                } else {
                    invalidSeg.write(1);
                }

                metaWritten = true;
            }
        }

        if (word.last) {
            header.clear();
            firstWord = true;
            metaWritten = false;
        }
    }
    // invalidSeg = invalidRecv;
    // receivedSeg = recvSeg;
}

/** @ingroup rx_engine

 *  @param[in]      metaIn
 *  @param[in]      dataIn
 *  @param[out]     dataOut
 *  @param[out]     dataOffsetOut
 *  @param[out]     optionalHeaderFieldsOut
 */
// data offset specifies the number of 32bit words
// the header is between 20 (5 words) and 60 (15 words). 
// This means the optional fields are between 0 and 10 words 
// (0 and 40 bytes)
template <int WIDTH>
void drop_optional_header_fields(hls::stream<optionalFieldsMeta>& metaIn,
                                 hls::stream<net_axis<WIDTH> >& dataIn,
#if (WINDOW_SCALE)
                                 hls::stream<ap_uint<4> >& dataOffsetOut,
                                 hls::stream<ap_uint<320> >& optionalHeaderFieldsOut,
#endif
                                 hls::stream<net_axis<WIDTH> >& dataOut) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    static ap_uint<2> state = 0;
    static ap_uint<4> dataOffset = 0;
    static net_axis<WIDTH> prevWord;
    static tcpOptionalHeader<WIDTH> optionalHeader;
    static bool parseHeader = false;
    static bool headerWritten = false;

    switch (state) {
        case 0:
            if (!metaIn.empty() && !dataIn.empty()) {
                optionalHeader.clear();
                optionalFieldsMeta meta = metaIn.read();
                net_axis<WIDTH> currWord = dataIn.read();
                dataOffset = meta.dataOffset; // - 5;

                optionalHeader.parseWord(currWord.data);
                parseHeader = false;
                // No optional fields, output data
                if (dataOffset == 0) {
                    dataOut.write(currWord);
                } else {
// TODO only works for 512
#if (WINDOW_SCALE)
                    if (meta.syn) {
                        parseHeader = true;
                        dataOffsetOut.write(dataOffset);
                        if (optionalHeader.isReady() || currWord.last) {
                            optionalHeaderFieldsOut.write(optionalHeader.getRawHeader());
                        }
                    }
#endif
                }

                state = 1;
                prevWord = currWord;
                headerWritten = false;
                if (currWord.last) {
                    state = 0;
                    // check if there is any remaining payload based on tkeep
                    if (dataOffset != 0 && (dataOffset * 4 < WIDTH / 8) && currWord.keep[dataOffset * 4] != 0) {
                        state = 2;
                    }
                }
            }
            break;
        case 1:
            if (!dataIn.empty()) {
                net_axis<WIDTH> currWord = dataIn.read();
                net_axis<WIDTH> sendWord;
                sendWord.last = 0;

#if (WINDOW_SCALE)
                optionalHeader.parseWord(currWord.data);
                if ((optionalHeader.isReady() || currWord.last) && parseHeader && !headerWritten) {
                    optionalHeaderFieldsOut.write(optionalHeader.getRawHeader());
                    headerWritten = true;
                }
#endif
                if (dataOffset > WIDTH / 32) {
                    dataOffset -= WIDTH / 32;
                } else if (dataOffset == 0 || dataOffset == 2) {
                // Including offset of 2 here because it is in 32-bit words. The options 
                // have already been dropped in state = 0.
                    sendWord = currWord;
                    dataOut.write(sendWord);
                } else // if (dataOffset == 1) 
                {
                    sendWord.data(WIDTH - (dataOffset * 32) - 1, 0) = prevWord.data(WIDTH - 1, dataOffset * 32);
                    sendWord.keep((WIDTH / 8) - (dataOffset * 4) - 1, 0) = prevWord.keep(WIDTH / 8 - 1, dataOffset * 4);
                    sendWord.data(WIDTH - 1, WIDTH - (dataOffset * 32)) = currWord.data(dataOffset * 32 - 1, 0);
                    sendWord.keep(WIDTH / 8 - 1, (WIDTH / 8) - (dataOffset * 4)) = currWord.keep(dataOffset * 4 - 1, 0);
                    sendWord.last = (currWord.keep[dataOffset * 4] == 0);
                    dataOut.write(sendWord);
                }

                prevWord = currWord;
                if (currWord.last) {
                    state = 0;
                    // This will cover any remaining data in the FIFO.
                    if (currWord.keep[dataOffset * 4] != 0 && dataOffset != 0) {
                        state = 2;
                    }
                }
            }
            break;
        case 2: {
            // Shift and align any remaining data
            net_axis<WIDTH> sendWord;
            sendWord.data(WIDTH - (dataOffset * 32) - 1, 0) = prevWord.data(WIDTH - 1, dataOffset * 32);
            sendWord.keep((WIDTH / 8) - (dataOffset * 4) - 1, 0) = prevWord.keep(WIDTH / 8 - 1, dataOffset * 4);
            sendWord.data(WIDTH - 1, WIDTH - (dataOffset * 32)) = 0;
            sendWord.keep(WIDTH / 8 - 1, (WIDTH / 8) - (dataOffset * 4)) = 0;
            sendWord.last = 1;
            dataOut.write(sendWord);
            state = 0;
            break;
        }       
    } // switch
}

/** @ingroup rx_engine
 * Optional Header Fields are only parse on connection establishment to determine
 * the window scaling factor
 * Available options (during SYN):
 * kind | length | description
 *   0  |     1B | End of options list
 *   1  |     1B | NOP/Padding
 *   2  |     4B | MSS (Maximum segment size)
 *   3  |     3B | Window scale
 *   4  |     2B | SACK permitted (Selective Acknowledgment)
 *
 *  @param[in]      metdataOffsetInaIn
 *  @param[in]      optionalHeaderFieldsIn
 *  @param[out]     windowScaleOut
 */
void parse_optional_header_fields(hls::stream<ap_uint<4> >& dataOffsetIn,
                                  hls::stream<ap_uint<320> >& optionalHeaderFieldsIn,
                                  hls::stream<ap_uint<4> >& windowScaleOut) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    enum fsmStateType { IDLE, PARSE };
    static fsmStateType state = IDLE;
    static ap_uint<4> dataOffset;
    static ap_uint<320> fields;

    switch (state) {
        case IDLE:
            if (!dataOffsetIn.empty() && !optionalHeaderFieldsIn.empty()) {
                dataOffsetIn.read(dataOffset);
                optionalHeaderFieldsIn.read(fields);
                state = PARSE;
            }
            break;
        case PARSE:
            ap_uint<8> optionKind = fields(7, 0);
            ap_uint<8> optionLength = fields(15, 8);

            switch (optionKind) {
                case 0: // End of option list
                    windowScaleOut.write(0);
                    state = IDLE;
                    break;
                case 1:
                    optionLength = 1;
                    break;
                case 3:
                    windowScaleOut.write(fields(19, 16));
                    state = IDLE;
                    break;
                default:
                    if (dataOffset == optionLength) {
                        windowScaleOut.write(0);
                        state = IDLE;
                    }
                    break;
            } // switch
            dataOffset -= optionLength;
            fields = (fields >> (optionLength * 8));
            break;
    } // switch
}

/** @ingroup rx_engine
 *  Sets the winscale field in the metadata
 *  @param[in]      rxEng_winScaleFifo
 *  @param[in]      rxEng_headerMetaFifo
 *  @param[out]     rxEng_metaDataFifo
 */
void merge_header_meta(hls::stream<ap_uint<4> >& rxEng_winScaleFifo,
                       hls::stream<rxEngineMetaData>& rxEng_headerMetaFifo,
                       hls::stream<rxEngineMetaData>& rxEng_metaDataFifo) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    static ap_uint<1> state = 0;
    static rxEngineMetaData meta;

    switch (state) {
        case 0:
            if (!rxEng_headerMetaFifo.empty()) {
                rxEng_headerMetaFifo.read(meta);
                if (meta.syn && meta.dataOffset > 5) {
                    state = 1;
                } else {
                    meta.winScale = 0;
                    rxEng_metaDataFifo.write(meta);
                }
            }
            break;
        case 1:
            if (!rxEng_winScaleFifo.empty()) {
                meta.winScale = rxEng_winScaleFifo.read();
                rxEng_metaDataFifo.write(meta);
                state = 0;
            }
            break;
    } // switch
}

/** @ingroup rx_engine
 * The module's FSM performs two actions:
 * In state META checks to see if the Port requested in the received message is 
 * open or not. If port is closed and the msg is not a RST (see metadata), an 
 * event is sent to the TX engine to generate a RST event. If opened a lookup 
 * request is sent to the session table if SYN or SYN_ACK fields enabled.
 * In LOOKUP state the Session Look Up module notifies if the received message 
 * is from an existing session and generates metadata. If not a RST event is sent 
 * to the Tx engine.
 * @param[in]   metaDataFifoIn
 * @param[in]   sLookup2rxEng_rsp
 * @param[in]   portTable2rxEng_rsp
 * @param[in]   tupleBufferIn
 * @param[out]  rxEng2sLookup_req
 * @param[out]  rxEng2eventEng_setEvent
 * @param[out]  dropDataFifoOut
 * @param[out]  fsmMetaDataFifo
 */
void rxMetadataHandler(hls::stream<rxEngineMetaData>& metaDataFifoIn,
                       hls::stream<sessionLookupReply>& sLookup2rxEng_rsp,
                       hls::stream<bool>& portTable2rxEng_rsp,
                       hls::stream<fourTuple>& tupleBufferIn,
                       hls::stream<sessionLookupQuery>& rxEng2sLookup_req,
                       hls::stream<extendedEvent>& rxEng2eventEng_setEvent,
                       hls::stream<bool>& dropDataFifoOut,
                       hls::stream<rxFsmMetaData>& fsmMetaDataFifo) {
#pragma HLS INLINE off
#pragma HLS pipeline II = 1

    static rxEngineMetaData mh_meta;
    static sessionLookupReply mh_lup;
    enum mhStateType { META, LOOKUP };
    static mhStateType mh_state = META;
    static ap_uint<32> mh_srcIpAddress;
    static ap_uint<16> mh_dstIpPort;

    static fourTuple tuple;
    bool portIsOpen;

    switch (mh_state) {
        case META:
            if (!metaDataFifoIn.empty() && !portTable2rxEng_rsp.empty() && !tupleBufferIn.empty()) {
                metaDataFifoIn.read(mh_meta);
                portTable2rxEng_rsp.read(portIsOpen);
                tupleBufferIn.read(tuple);
                rxeng_printTCPHeader(mh_meta, tuple);

                mh_srcIpAddress = reverse(tuple.srcIp);
                mh_dstIpPort = reverse(tuple.dstPort);
                // Check if port is closed
                if (!portIsOpen) {
                    // SEND RST+ACK
                    if (!mh_meta.rst) {
                        // send necessary tuple through event
                        fourTuple switchedTuple;
                        switchedTuple.srcIp = tuple.dstIp;
                        switchedTuple.dstIp = tuple.srcIp;
                        switchedTuple.srcPort = tuple.dstPort;
                        switchedTuple.dstPort = tuple.srcPort;
                        if (!mh_meta.ack) {
                            // If the ack bit is off, sequence number zero is used. 
                            // <SEQ=0><ACK=SEG.SEQ+SEG.LEN><CTL=RST,ACK>                            
                            uint32_t nAck = mh_meta.seqNumb + mh_meta.length;
                            if (mh_meta.syn || mh_meta.fin) {
                                nAck++;
                            }
                            rxEng2eventEng_setEvent.write(extendedEvent(rstAckEvent(nAck), switchedTuple));
                        } else {
                            // If the ACK bit is on, <SEQ=SEG.ACK><CTL+RST>
                            rxEng2eventEng_setEvent.write(extendedEvent(rstEvent(mh_meta.ackNumb), switchedTuple));
                        }
                    }
                    // else ignore => do nothing
                    if (mh_meta.length != 0) {
                        dropDataFifoOut.write(true);
                    }
                } else {
                    // Make session lookup, only allow creation of new entry when SYN or SYN_ACK
                    rxEng2sLookup_req.write(sessionLookupQuery(tuple, (mh_meta.syn && !mh_meta.rst && !mh_meta.fin)));
                    mh_state = LOOKUP;
                }
            }
            break;
        case LOOKUP: // BIG delay here, waiting for LOOKup
            if (!sLookup2rxEng_rsp.empty()) {
                sLookup2rxEng_rsp.read(mh_lup);
                if (mh_lup.hit) {
                    // Write out lup and meta
                    fsmMetaDataFifo.write(rxFsmMetaData(mh_lup.sessionID, mh_srcIpAddress, mh_dstIpPort, mh_meta));
                } else {
                    // A segment has arrived but the session has not been created. An incoming segment containing a RST
                    // is discarded. An incoming segment not containing a RST causes a RST to be sent in response
                    if (!mh_meta.rst) {
                        fourTuple switchedTuple;
                        switchedTuple.srcIp = tuple.dstIp;
                        switchedTuple.dstIp = tuple.srcIp;
                        switchedTuple.srcPort = tuple.dstPort;
                        switchedTuple.dstPort = tuple.srcPort;    
                        if (!mh_meta.ack) {
                            // If the ack bit is off, sequence number zero is used. 
                            // <SEQ=0><ACK=SEG.SEQ+SEG.LEN><CTL=RST,ACK>                            
                            uint32_t nAck = mh_meta.seqNumb + mh_meta.length;
                            if (mh_meta.syn || mh_meta.fin) {
                                nAck++;
                            }
                            rxEng2eventEng_setEvent.write(extendedEvent(rstAckEvent(nAck), switchedTuple));
                        } else {
                            // If the ACK bit is on, <SEQ=SEG.ACK><CTL+RST>
                            rxEng2eventEng_setEvent.write(extendedEvent(rstEvent(mh_meta.ackNumb), switchedTuple));
                        }
                    }
                }
                if (mh_meta.length != 0) {
                    dropDataFifoOut.write(!mh_lup.hit);
                }
                mh_state = META;                
            }

            break;
    } // switch
}

/** @ingroup rx_engine
 * The module contains 2 state machines nested into each other. The outer state machine
 * loads the metadata and does the session lookup. The inner state machine then evaluates all
 * this data. This inner state machine mostly represents the TCP state machine and contains
 * all the logic how to update the metadata, what events are triggered and so on. It is the key
 * part of the @ref rx_engine.
 * @param[in]   fsmMetaDataFifo
 * @param[in]   stateTable2rxEng_upd_rsp
 * @param[in]   rxSar2rxEng_upd_rsp
 * @param[in]   txSar2rxEng_upd_rsp
 * @param[out]  rxEng2stateTable_upd_req
 * @param[out]  rxEng2rxSar_upd_req
 * @param[out]  rxEng2txSar_upd_req
 * @param[out]  rxEng2timer_clearRetransmitTimer
 * @param[out]  rxEng2timer_clearProbeTimer
 * @param[out]  rxEng2timer_setCloseTimer
 * @param[out]  openConStatusOut
 * @param[out]  rxEng2eventEng_setEvent
 * @param[out]  dropDataFifoOut
 * @param[out]  rxBufferWriteCmd
 * @param[out]  rxEng2rxApp_notification
 */
void rxTcpFSM(hls::stream<rxFsmMetaData>& fsmMetaDataFifo,
              hls::stream<sessionState>& stateTable2rxEng_upd_rsp,
              hls::stream<rxSarEntry>& rxSar2rxEng_upd_rsp,
              hls::stream<rxTxSarReply>& txSar2rxEng_upd_rsp,
              hls::stream<stateQuery>& rxEng2stateTable_upd_req,
              hls::stream<rxSarRecvd>& rxEng2rxSar_upd_req,
              hls::stream<rxTxSarQuery>& rxEng2txSar_upd_req,
              hls::stream<rxRetransmitTimerUpdate>& rxEng2timer_clearRetransmitTimer,
              hls::stream<ap_uint<16> >& rxEng2timer_clearProbeTimer,
              hls::stream<ap_uint<16> >& rxEng2timer_setCloseTimer,
              hls::stream<openStatus>& openConStatusOut,
              hls::stream<event>& rxEng2eventEng_setEvent,
              hls::stream<bool>& dropDataFifoOut,
              hls::stream<ap_uint<1> >& stats_tcpAttemptFails,
              hls::stream<ap_uint<1> >& stats_tcpEstabResets,
#if !(RX_DDR_BYPASS)
              hls::stream<mmCmd>& rxBufferWriteCmd,
              hls::stream<appNotification>& rxEng2rxApp_notification)
#else
              hls::stream<appNotification>& rxEng2rxApp_notification,
              ap_uint<16> rxbuffer_data_count,
              ap_uint<16> rxbuffer_max_data_count)
#endif
{
#pragma HLS INLINE off
#pragma HLS pipeline II = 1

    enum fsmStateType { LOAD, TRANSITION };
    static fsmStateType fsm_state = LOAD;

    static rxFsmMetaData fsm_meta;
    static bool fsm_txSarRequest = false;
    // static uint32_t cnt_tcpAttemptFails = 0;
    // static uint32_t cnt_tcpEstabResets = 0;

    ap_uint<4> control_bits = 0;
    sessionState tcpState;
    rxSarEntry rxSar;
    rxTxSarReply txSar;
    bool fSendRxNotification = false;
    appNotification rxNotification;

    switch (fsm_state) {
        case LOAD:
            if (!fsmMetaDataFifo.empty()) {
                fsmMetaDataFifo.read(fsm_meta);
                // read state
                rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID));
                // Always read rxSar, even though not required for SYN-ACK
                rxEng2rxSar_upd_req.write(rxSarRecvd(fsm_meta.sessionID));
                // read txSar
                if (fsm_meta.meta.ack) { // Do not read for SYN (ACK+ANYTHING)
                    rxEng2txSar_upd_req.write(rxTxSarQuery(fsm_meta.sessionID));
                    fsm_txSarRequest = true;
                }
                fsm_state = TRANSITION;
            }
            break;
        case TRANSITION:
            // Check if transition to LOAD occurs
            if (!stateTable2rxEng_upd_rsp.empty() && !rxSar2rxEng_upd_rsp.empty() &&
                !(fsm_txSarRequest && txSar2rxEng_upd_rsp.empty())) {
                fsm_state = LOAD;
                fsm_txSarRequest = false;
            }

            control_bits[0] = fsm_meta.meta.ack;
            control_bits[1] = fsm_meta.meta.syn;
            control_bits[2] = fsm_meta.meta.fin;
            control_bits[3] = fsm_meta.meta.rst;
            switch (control_bits) {
                case 1: // ACK
                    if (fsm_state == LOAD) {
                        stateTable2rxEng_upd_rsp.read(tcpState);
                        rxSar2rxEng_upd_rsp.read(rxSar);
                        txSar2rxEng_upd_rsp.read(txSar);
                        rxEng2timer_clearRetransmitTimer.write(
                            rxRetransmitTimerUpdate(fsm_meta.sessionID, (fsm_meta.meta.ackNumb == txSar.nextByte)));
                        if (tcpState == ESTABLISHED || tcpState == SYN_RECEIVED || tcpState == FIN_WAIT_1 ||
                            tcpState == CLOSING || tcpState == LAST_ACK) {
                            // Check if new ACK arrived
                            if (fsm_meta.meta.ackNumb == txSar.prevAck && txSar.prevAck != txSar.nextByte) {
                                // Not new ACK increase counter only if it does not contain data
                                if (fsm_meta.meta.length == 0) {
                                    txSar.count++;
                                }
                            } else {
                                // Notify probeTimer about new ACK
                                rxEng2timer_clearProbeTimer.write(fsm_meta.sessionID);
                                // Check for SlowStart & Increase Congestion Window
                                if (txSar.cong_window <= (txSar.slowstart_threshold - MSS)) {
                                    txSar.cong_window += MSS;
                                } else if (txSar.cong_window <= CONGESTION_WINDOW_MAX) { // 0xF7FF
                                    // Increase cong window by 1/4 of Max Seg Size
                                    txSar.cong_window += 365; // TODO replace by approx. of (MSS x MSS) / cong_window
                                }
                                txSar.count = 0;
                                txSar.fastRetransmitted = false;
                            }
                            // TX SAR
                            if ((txSar.prevAck <= fsm_meta.meta.ackNumb && fsm_meta.meta.ackNumb <= txSar.nextByte) ||
                                ((txSar.prevAck <= fsm_meta.meta.ackNumb || fsm_meta.meta.ackNumb <= txSar.nextByte) &&
                                 txSar.nextByte < txSar.prevAck)) {
                                // Update Tx SAR table if ACK is within valid window
                                rxEng2txSar_upd_req.write((rxTxSarQuery(
                                    fsm_meta.sessionID, fsm_meta.meta.ackNumb, fsm_meta.meta.winSize, txSar.cong_window,
                                    txSar.count, ((txSar.count == 3) || txSar.fastRetransmitted))));
                            }

                            // Check if packet contains payload
                            if (fsm_meta.meta.length != 0) {
                                ap_uint<32> newRecvd = fsm_meta.meta.seqNumb + fsm_meta.meta.length;
// Second part makes sure that app pointer is not overtaken
#if !(RX_DDR_BYPASS)
                                ap_uint<WINDOW_BITS> free_space = ((rxSar.appd - rxSar.recvd(WINDOW_BITS - 1, 0)) - 1);
                                // Check if segment in order and if enough free space is available
                                if ((fsm_meta.meta.seqNumb == rxSar.recvd) && (free_space >= fsm_meta.meta.length))
#else
                                // compare received seq number with pointer from Rx Sar table
                                if ((fsm_meta.meta.seqNumb == rxSar.recvd) &&
                                    ((rxbuffer_max_data_count - rxbuffer_data_count) > 375))
#endif
                                {
                                    rxEng2rxSar_upd_req.write(rxSarRecvd(fsm_meta.sessionID, newRecvd));
                                    // Build memory address
                                    ap_uint<32> pkgAddr;
                                    pkgAddr(31, 30) = 0x0;
                                    pkgAddr(29, WINDOW_BITS) = fsm_meta.sessionID(13, 0);
                                    pkgAddr(WINDOW_BITS - 1, 0) = fsm_meta.meta.seqNumb(WINDOW_BITS - 1, 0);
#if !(RX_DDR_BYPASS)
                                    rxBufferWriteCmd.write(mmCmd(pkgAddr, fsm_meta.meta.length));
#endif
                                    // Notify about new data available
                                    fSendRxNotification = true;
                                    rxNotification = appNotification(fsm_meta.sessionID, fsm_meta.meta.length,
                                                                     fsm_meta.srcIpAddress, fsm_meta.dstIpPort);
                                    std::cout << "Rx Engine Writing Frame" << std::endl;
                                    dropDataFifoOut.write(false);
                                } else {
                                    std::cout << "Rx Engine Dropping Frame" << std::endl;
                                    dropDataFifoOut.write(true);
                                }
                            }
#if FAST_RETRANSMIT
                            // After 3 duplicate ACKs we request a RT
                            if (txSar.count == 3 && !txSar.fastRetransmitted) {
                                rxEng2eventEng_setEvent.write(event(RT, fsm_meta.sessionID));
                            } else if (fsm_meta.meta.length != 0)
#else
                            if (fsm_meta.meta.length != 0)
#endif
                            {
                                rxEng2eventEng_setEvent.write(event(ACK, fsm_meta.sessionID));
                            }

                            // Reset Retransmit Timer
                            if (fsm_meta.meta.ackNumb == txSar.nextByte) {
                                switch (tcpState) {
                                    case SYN_RECEIVED:
                                        rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, ESTABLISHED, 1));
                                        if (!fSendRxNotification) {
                                            fSendRxNotification = true;
                                            rxNotification = appNotification(fsm_meta.sessionID, fsm_meta.srcIpAddress,
                                                                             fsm_meta.dstIpPort);
                                        }
                                        rxNotification.opened = true;
                                        break;
                                    case CLOSING:
                                        rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, TIME_WAIT, 1));
                                        rxEng2timer_setCloseTimer.write(fsm_meta.sessionID);
                                        break;
                                    case LAST_ACK:
                                        // Tell Application connection got closed. Add fClosed to notification if
                                        // already creating new data.
                                        if (!fSendRxNotification) {
                                            fSendRxNotification = true;
                                            rxNotification = appNotification(fsm_meta.sessionID, fsm_meta.srcIpAddress,
                                                                             fsm_meta.dstIpPort);
                                        }
                                        rxNotification.closed = true;
                                        rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, CLOSED, 1));
                                        break;
                                    default:
                                        rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, tcpState, 1));
                                        break;
                                }
                            } else { // we have to release the lock
                                // reset rtTimer
                                rxEng2stateTable_upd_req.write(
                                    stateQuery(fsm_meta.sessionID, tcpState, 1)); // or ESTABLISHED
                            }
                            // end state if
                        } else { // state == (CLOSED || SYN_SENT || CLOSE_WAIT || FIN_WAIT_2 || TIME_WAIT)
                                 // TODO if timewait just send ACK, can it be time wait??
                            // SENT RST, RFC 793: fig.11
                            rxEng2eventEng_setEvent.write(
                                rstAckEvent(fsm_meta.sessionID, fsm_meta.meta.seqNumb + fsm_meta.meta.length)); // noACK ?
                            // if data is in the pipe it needs to be dropped
                            if (fsm_meta.meta.length != 0) {
                                dropDataFifoOut.write(true);
                            }
                            rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, tcpState, 1));
                        }
                        // fsm_state = LOAD;
                    }
                    break;
                case 2: // SYN
                    if (fsm_state == LOAD) {
                        stateTable2rxEng_upd_rsp.read(tcpState);
                        rxSar2rxEng_upd_rsp.read(rxSar);
                        if (tcpState == CLOSED ||
                            tcpState ==
                                SYN_SENT) { // Actually this is LISTEN || SYN_SENT but LISTEN has been merged in CLOSED
#if (WINDOW_SCALE)
                            ap_uint<4> rx_win_shift = (fsm_meta.meta.winScale == 0) ? 0 : WINDOW_SCALE_BITS;
                            ap_uint<4> tx_win_shift = fsm_meta.meta.winScale;
                            // Initialize rxSar, SEQ + phantom byte
                            rxEng2rxSar_upd_req.write(
                                rxSarRecvd(fsm_meta.sessionID, fsm_meta.meta.seqNumb + 1, rx_win_shift));
                            // Initialize receive window
                            rxEng2txSar_upd_req.write(
                                (rxTxSarQuery(fsm_meta.sessionID, 0, fsm_meta.meta.winSize, txSar.cong_window, 0, false,
                                              tx_win_shift))); // TODO maybe include count check
#else
                            // Initialize rxSar, SEQ + phantom byte, last '1' for makes sure appd is initialized
                            rxEng2rxSar_upd_req.write(rxSarRecvd(fsm_meta.sessionID, fsm_meta.meta.seqNumb + 1, 1));
                            // Initialize receive window
                            rxEng2txSar_upd_req.write(
                                (rxTxSarQuery(fsm_meta.sessionID, 0, fsm_meta.meta.winSize, txSar.cong_window, 0,
                                              false))); // TODO maybe include count check
#endif
                            // Set SYN_ACK event
                            rxEng2eventEng_setEvent.write(event(SYN_ACK, fsm_meta.sessionID));
                            // Change State to SYN_RECEIVED
                            rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, SYN_RECEIVED, 1));
                        } else if (tcpState ==
                                   SYN_RECEIVED) { // && mh_meta.seqNumb+1 == rxSar.recvd) // Maybe Check for seq
                            // If it is the same SYN, we resend SYN-ACK, almost like quick RT, we could also wait for RT
                            // timer
                            if (fsm_meta.meta.seqNumb + 1 == rxSar.recvd) {
                                // Retransmit SYN_ACK
                                rxEng2eventEng_setEvent.write(event(SYN_ACK, fsm_meta.sessionID, 1));
                                rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, tcpState, 1));
                            } else { // Send RST, RFC 793: fig.9 (old) duplicate SYN(+ACK)
                                rxEng2eventEng_setEvent.write(
                                    rstAckEvent(fsm_meta.sessionID, fsm_meta.meta.seqNumb + 1)); // length == 0
                                rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, CLOSED, 1));
                                stats_tcpAttemptFails.write(1);
                                // cnt_tcpAttemptFails++;
                            }
                        } else { // Any synchronized state
                            // Unexpected SYN arrived, reply with normal ACK, RFC 793: fig.10
                            // RFC5961: Blind Reset Attack Using the SYN Bit. If the SYN bit is set, 
                            // irrespective of the sequence number, TCP MUST send an ACK
                            // (also referred to as challenge ACK) to the remote peer
                            rxEng2eventEng_setEvent.write(event(ACK_NODELAY, fsm_meta.sessionID));
                            rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, tcpState, 1));
                        }
                    }
                    break;
                case 3: // SYN_ACK
                    if (fsm_state == LOAD) {
                        stateTable2rxEng_upd_rsp.read(tcpState);
                        rxSar2rxEng_upd_rsp.read(rxSar);
                        txSar2rxEng_upd_rsp.read(txSar);
                        rxEng2timer_clearRetransmitTimer.write(
                            rxRetransmitTimerUpdate(fsm_meta.sessionID, (fsm_meta.meta.ackNumb == txSar.nextByte)));
                        if ((tcpState == SYN_SENT) && (fsm_meta.meta.ackNumb == txSar.nextByte)) {
#if (WINDOW_SCALE)
                            ap_uint<4> rx_win_shift = (fsm_meta.meta.winScale == 0) ? 0 : WINDOW_SCALE_BITS;
                            ap_uint<4> tx_win_shift = fsm_meta.meta.winScale;
                            rxEng2rxSar_upd_req.write(
                                rxSarRecvd(fsm_meta.sessionID, fsm_meta.meta.seqNumb + 1, rx_win_shift));
                            rxEng2txSar_upd_req.write((rxTxSarQuery(fsm_meta.sessionID, fsm_meta.meta.ackNumb,
                                                                    fsm_meta.meta.winSize, txSar.cong_window, 0, false,
                                                                    tx_win_shift))); // TODO maybe include count check
#else
                            // initialize rx_sar, SEQ + phantom byte, last '1' for appd init
                            rxEng2rxSar_upd_req.write(rxSarRecvd(fsm_meta.sessionID, fsm_meta.meta.seqNumb + 1, 1));

                            rxEng2txSar_upd_req.write(
                                (rxTxSarQuery(fsm_meta.sessionID, fsm_meta.meta.ackNumb, fsm_meta.meta.winSize,
                                              txSar.cong_window, 0, false))); // TODO maybe include count check
#endif
                            // set ACK event
                            rxEng2eventEng_setEvent.write(event(ACK_NODELAY, fsm_meta.sessionID));

                            rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, ESTABLISHED, 1));
                            openConStatusOut.write(openStatus(fsm_meta.sessionID, true));
                        } else if (nonSynchronized(tcpState)) {
                            // Received a duplicate SYN_ACK in an unsynchronized state
                            // RFC 793: Page 36 Reset Generation (2)
                            // If the ACK bit is on, <SEQ=SEG.ACK><CTL+RST>
                            rxEng2eventEng_setEvent.write(rstEvent(fsm_meta.sessionID, fsm_meta.meta.ackNumb));
                            rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, tcpState, 1));
                        } else { // Unexpected segment in a synchronized state
                                 // Reply with normal ACK, RFC 793: fig.10 + Page 37 (3)
                            rxEng2eventEng_setEvent.write(event(ACK_NODELAY, fsm_meta.sessionID));
                            rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, tcpState, 1));
                        }
                    }
                    break;
                case 5: // FIN (_ACK)
                    if (fsm_state == LOAD) {
                        stateTable2rxEng_upd_rsp.read(tcpState);
                        rxSar2rxEng_upd_rsp.read(rxSar);
                        txSar2rxEng_upd_rsp.read(txSar);
                        rxEng2timer_clearRetransmitTimer.write(
                            rxRetransmitTimerUpdate(fsm_meta.sessionID, (fsm_meta.meta.ackNumb == txSar.nextByte)));
                        // Check state and if FIN in order, Current out of order FINs are not accepted
                        if ((tcpState == ESTABLISHED || tcpState == FIN_WAIT_1 || tcpState == FIN_WAIT_2) &&
                            (rxSar.recvd == fsm_meta.meta.seqNumb)) {
                            rxEng2txSar_upd_req.write((rxTxSarQuery(
                                fsm_meta.sessionID, fsm_meta.meta.ackNumb, fsm_meta.meta.winSize, txSar.cong_window,
                                txSar.count, txSar.fastRetransmitted))); // TODO include count check

                            // +1 for phantom byte, there might be data too
                            rxEng2rxSar_upd_req.write(rxSarRecvd(
                                fsm_meta.sessionID, fsm_meta.meta.seqNumb + fsm_meta.meta.length + 1)); // diff to ACK

                            // Clear the probe timer
                            rxEng2timer_clearProbeTimer.write(fsm_meta.sessionID);

                            // Check if there is payload
                            if (fsm_meta.meta.length != 0) {
                                ap_uint<32> pkgAddr;
                                pkgAddr(31, 30) = 0x0;
                                pkgAddr(29, WINDOW_BITS) = fsm_meta.sessionID(13, 0);
                                pkgAddr(WINDOW_BITS - 1, 0) = fsm_meta.meta.seqNumb(WINDOW_BITS - 1, 0);
#if !(RX_DDR_BYPASS)
                                rxBufferWriteCmd.write(mmCmd(pkgAddr, fsm_meta.meta.length));
#endif
                                // Tell Application new data is available
                                fSendRxNotification = true;
                                rxNotification = appNotification(fsm_meta.sessionID, fsm_meta.meta.length,
                                                                 fsm_meta.srcIpAddress, fsm_meta.dstIpPort);
                                dropDataFifoOut.write(false);
                            }

                            // Update state
                            if (tcpState == ESTABLISHED) {
                                rxEng2eventEng_setEvent.write(event(FIN, fsm_meta.sessionID));
                                rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, LAST_ACK, 1));
                            } else {                                           // FIN_WAIT_1 || FIN_WAIT_2
                                if (fsm_meta.meta.ackNumb == txSar.nextByte) { // check if final FIN is ACK'd ->
                                                                               // LAST_ACK
                                    rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, TIME_WAIT, 1));
                                    rxEng2timer_setCloseTimer.write(fsm_meta.sessionID);
                                } else {
                                    rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, CLOSING, 1));
                                }
                                rxEng2eventEng_setEvent.write(event(ACK, fsm_meta.sessionID));
                            }
                        } else { // NOT (ESTABLISHED || FIN_WAIT_1 || FIN_WAIT_2)
                            rxEng2eventEng_setEvent.write(event(ACK, fsm_meta.sessionID));
                            rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, tcpState, 1));
                            // If there is payload we need to drop it
                            if (fsm_meta.meta.length != 0) {
                                dropDataFifoOut.write(true);
                            }
                        }
                    }
                    break;
                default:
                    // stateTable is locked, make sure it is released in at the end
                    // If there is an ACK we read txSar
                    // We always read rxSar
                    if (fsm_state == LOAD) {
                        stateTable2rxEng_upd_rsp.read(tcpState);
                        rxSar2rxEng_upd_rsp.read(rxSar); // TODO not sure nb works
                        txSar2rxEng_upd_rsp.read_nb(txSar);
                        // RST Processing rfc793 pg 37
                        if (fsm_meta.meta.rst) {
                            if (fsm_meta.meta.length != 0) {
                                dropDataFifoOut.write(true);
                            }
                            if (tcpState == SYN_SENT) {                        // TODO this would be a RST,ACK i think #
                                if (fsm_meta.meta.ackNumb == txSar.nextByte) { // Check if matching SYN, pg 37 rfc793
                                    // tell application, could not open connection
                                    openConStatusOut.write(openStatus(fsm_meta.sessionID, false));
                                    rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, CLOSED, 1));
                                    rxEng2timer_clearRetransmitTimer.write(
                                        rxRetransmitTimerUpdate(fsm_meta.sessionID, true));
                                } else {
                                    // Ignore since not matching
                                    rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, tcpState, 1));
                                }
                            } else { // TODO GN: pg 62 rfc793 splits up closing, last-ack and time-wait states
                                // Check if in window
                                if (fsm_meta.meta.seqNumb == rxSar.recvd) {
                                    // Only send a close notification if we have reached ESTABLISHED
                                    // i.e. we have emitted an open notification.
                                    if (tcpState != SYN_RECEIVED) {                                                                    
                                        fSendRxNotification = true;
                                        rxNotification = appNotification(fsm_meta.sessionID, fsm_meta.srcIpAddress,
                                                                         fsm_meta.dstIpPort); // RESET
                                        rxNotification.closed = true;                                                                         
                                    }
                                    rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, CLOSED, 1));
                                    rxEng2timer_clearRetransmitTimer.write(
                                        rxRetransmitTimerUpdate(fsm_meta.sessionID, true));
                                    // cnt_tcpAttemptFails++;
                                    stats_tcpAttemptFails.write(1);
                                    if (tcpState == ESTABLISHED) {
                                        stats_tcpEstabResets.write(1);
                                        // cnt_tcpEstabResets++;
                                    }
                                } else {
                                    // Ignore since not matching window
                                    rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, tcpState, 1));
                                }
                            }
                        } else { // Handle non RST bogus packages
                            // TODO maybe sent RST ourselves, or simply ignore
                            // For now ignore, sent ACK??
                            // eventsOut.write(rstAckEvent(mh_meta.seqNumb, 0, true));
                            rxEng2stateTable_upd_req.write(stateQuery(fsm_meta.sessionID, tcpState, 1));
                        } // if rst
                    }     // if fsm_stat
                    break;
            } // switch control_bits
            break;
    } // switch state

    // Send the notification if one was created.
    if (fSendRxNotification) {
        rxEng2rxApp_notification.write(rxNotification);
    }

    // stats_tcpAttemptFails = cnt_tcpAttemptFails;
    // stats_tcpEstabResets = cnt_tcpEstabResets;
}

/** @ingroup rx_engine
 *  Drops packets if their metadata did not match / are invalid, as indicated by @param dropBuffer
 *  @param[in]      dataIn, incoming data stream
 *  @param[in]      dropFifoIn, Drop-FIFO indicating if packet needs to be dropped
 *  @param[out]     rxBufferDataOut, outgoing data stream
 */
template <int WIDTH>
void rxPackageDropper(hls::stream<net_axis<WIDTH> >& dataIn,
                      hls::stream<bool>& dropFifoIn1,
                      hls::stream<bool>& dropFifoIn2,
                      hls::stream<ap_axiu<WIDTH,0,0,0> >& rxBufferDataOut) {
#pragma HLS INLINE off
#pragma HLS pipeline II = 1

    enum tpfStateType { READ_DROP1, READ_DROP2, FWD, DROP };
    static tpfStateType tpf_state = READ_DROP1;

    bool drop;
    ap_axiu<WIDTH,0,0,0> outWord;

    switch (tpf_state) {
        case READ_DROP1: // Drop1
            if (!dropFifoIn1.empty()) {
                dropFifoIn1.read(drop);
                if (drop) {
                    tpf_state = DROP;
                } else {
                    tpf_state = READ_DROP2;
                }
            }
            break;
        case READ_DROP2:
            if (!dropFifoIn2.empty()) {
                dropFifoIn2.read(drop);
                if (drop) {
                    tpf_state = DROP;
                } else {
                    tpf_state = FWD;
                }
            }
            break;
        case FWD:
            if (!dataIn.empty()) {
                net_axis<WIDTH> currWord = dataIn.read();
                if (currWord.last) {
                    tpf_state = READ_DROP1;
                }
                outWord.data = currWord.data;
                outWord.keep = currWord.keep;
                outWord.last = currWord.last;
                rxBufferDataOut.write(outWord);
            }
            break;
        case DROP:
            if (!dataIn.empty()) {
                net_axis<WIDTH> currWord = dataIn.read();
                if (currWord.last) {
                    tpf_state = READ_DROP1;
                }
            }
            break;
    } // switch
}

/** @ingroup rx_engine
 *  Delays the notifications to the application until the data is actually written to memory
 *  @param[in]      rxWriteStatusIn, the status which we get back from the DATA MOVER it indicates if the write was
 * successful
 *  @param[in]      internalNotificationFifoIn, incoming notifications
 *  @param[out]     notificationOut, outgoing notifications
 *  @TODO Handle unsuccessful write to memory
 */
void rxAppNotificationDelayer(hls::stream<ap_axiu<8,0,0,0> >& rxWriteStatusIn,
                              hls::stream<appNotification>& internalNotificationFifoIn,
                              hls::stream<appNotification>& notificationOut,
                              hls::stream<ap_uint<1> >& doubleAccess) {
#pragma HLS INLINE off
#pragma HLS pipeline II = 1

    static hls::stream<appNotification> rand_notificationBuffer("rand_notificationBuffer");
#pragma HLS STREAM variable = rand_notificationBuffer depth = 32 // depends on memory delay

    static ap_uint<1> rxAppNotificationDoubleAccessFlag = false;
    static ap_uint<5> rand_fifoCount = 0;
    static mmStatus rxAppNotificationStatus1, rxAppNotificationStatus2;
    static appNotification rxAppNotification;

    // TODO what happens if rxAppNotificationStatus1/rxAppNotificationStatus2.okay return 0
    if (rxAppNotificationDoubleAccessFlag == true) {
        if (!rxWriteStatusIn.empty()) {
        	rxAppNotificationStatus2 = rx_axiu2mmStatus(rxWriteStatusIn.read());
            rand_fifoCount--;
            if (rxAppNotificationStatus1.okay && rxAppNotificationStatus2.okay)
                notificationOut.write(rxAppNotification);
            rxAppNotificationDoubleAccessFlag = false;
        }
    } else if (rxAppNotificationDoubleAccessFlag == false) {
        if (!rxWriteStatusIn.empty() && !rand_notificationBuffer.empty() && !doubleAccess.empty()) {
        	rxAppNotificationStatus1 = rx_axiu2mmStatus(rxWriteStatusIn.read());
            rand_notificationBuffer.read(rxAppNotification);
            rxAppNotificationDoubleAccessFlag =
                doubleAccess.read(); // Read the double notification flag. If one then go and w8 for the second status
            if (rxAppNotificationDoubleAccessFlag ==
                0) { // if the memory access was not broken down in two for this segment
                rand_fifoCount--;
                if (rxAppNotificationStatus1.okay) notificationOut.write(rxAppNotification); // Output the notification
            }
        } else if (!internalNotificationFifoIn.empty() && (rand_fifoCount < 31)) {
            internalNotificationFifoIn.read(rxAppNotification);
            if (rxAppNotification.length != 0) {
                rand_notificationBuffer.write(rxAppNotification);
                rand_fifoCount++;
            } else
                notificationOut.write(rxAppNotification);
        }
    }
}

/** @ingroup rx_engine
 *  @param[in]      in1
 *  @param[in]      in2
 *  @param[out]     out
 */
void rxEventMerger(hls::stream<extendedEvent>& in1, hls::stream<event>& in2, hls::stream<extendedEvent>& out) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE OFF

    if (!in1.empty()) {
        out.write(in1.read());
    } else if (!in2.empty()) {
        out.write(in2.read());
    }
}

/** @ingroup rx_engine
 * Controls the writes to memory based on inputs from rxPackageDropper and rxTcpFSM
 *  @param[in]      dataIn, incoming data stream
 *  @param[in]      cmdIn, incoming command from rxTcpFSM
 *  @param[out]     cmdOut, outgoing memory command
 *  @param[out]     dataOut, outgoing data for memory
 *  @param[out]     doubleAccess, outgoing value if 2 writes are required to memory
 */
template <int WIDTH>
void rxEngMemWrite(hls::stream<net_axis<WIDTH> >& dataIn,
                   hls::stream<mmCmd>& cmdIn,
                   hls::stream<ap_axiu<128,0,0,0> >& cmdOut,
                   hls::stream<ap_axiu<WIDTH,0,0,0> >& dataOut,
                   hls::stream<ap_uint<1> >& doubleAccess) {
#pragma HLS pipeline II = 1
#pragma HLS INLINE off

    enum fsmStateType { IDLE, CUT_FIRST, ALIGN_SECOND, FWD_ALIGNED, RESIDUE };
    static fsmStateType rxMemWrState = IDLE;
    static mmCmd cmd;
    static ap_uint<WINDOW_BITS> remainingLength = 0;
    static ap_uint<WINDOW_BITS> lengthFirstPkg;
    static ap_uint<8> offset = 0;
    static net_axis<WIDTH> prevWord;
    ap_axiu<WIDTH,0,0,0> outWord;

    switch (rxMemWrState) {
        case IDLE:
            if (!cmdIn.empty()) {
                cmdIn.read(cmd);

                if ((cmd.saddr(WINDOW_BITS - 1, 0) + cmd.bbt) > BUFFER_SIZE) {
                    lengthFirstPkg = BUFFER_SIZE - cmd.saddr(WINDOW_BITS - 1, 0);
                    remainingLength = lengthFirstPkg;
                    offset = lengthFirstPkg(DATA_KEEP_BITS - 1, 0);

                    doubleAccess.write(true);
                    cmdOut.write(rx_mmCmd2axiu2(mmCmd(cmd.saddr, lengthFirstPkg)));
                    rxMemWrState = CUT_FIRST;
                } else {
                    doubleAccess.write(false);

                    cmdOut.write(rx_mmCmd2axiu2(cmd));
                    rxMemWrState = FWD_ALIGNED;
                }
            }
            break;
        case CUT_FIRST:
            if (!dataIn.empty()) {
                dataIn.read(prevWord);
                net_axis<WIDTH> sendWord = prevWord;

                if (remainingLength > (WIDTH / 8)) {
                    remainingLength -= (WIDTH / 8);
                }
                // This means that the second packet is aligned
                else if (remainingLength == (WIDTH / 8)) {
                    sendWord.last = 1;

                    cmd.saddr(WINDOW_BITS - 1, 0) = 0;
                    cmd.bbt -= lengthFirstPkg;
                    cmdOut.write(rx_mmCmd2axiu2(cmd));
                    rxMemWrState = FWD_ALIGNED;
                } else {
                    sendWord.keep = lenToKeep(remainingLength);
                    sendWord.last = 1;

                    cmd.saddr(WINDOW_BITS - 1, 0) = 0;
                    cmd.bbt -= lengthFirstPkg;
                    cmdOut.write(rx_mmCmd2axiu2(cmd));
                    rxMemWrState = ALIGN_SECOND;
                    // If only part of a word is left
                    if (prevWord.last) {
                        rxMemWrState = RESIDUE;
                    }
                }
                outWord.data = sendWord.data;
                outWord.keep = sendWord.keep;
                outWord.last = sendWord.last;
                dataOut.write(outWord);
            }
            break;
        case FWD_ALIGNED: // This is the non-realignment state
            if (!dataIn.empty()) {
                net_axis<WIDTH> currWord = dataIn.read();
                outWord.data = currWord.data;
                outWord.keep = currWord.keep;
                outWord.last = currWord.last;
                dataOut.write(outWord);
                if (currWord.last) {
                    rxMemWrState = IDLE;
                }
            }
            break;
        case ALIGN_SECOND: // We go into this state when we need to realign things
            if (!dataIn.empty()) {
                net_axis<WIDTH> currWord = dataIn.read();
                net_axis<WIDTH> sendWord;
                sendWord.data(WIDTH - 1, WIDTH - (offset * 8)) = currWord.data(offset * 8 - 1, 0);
                sendWord.data(WIDTH - (offset * 8) - 1, 0) = prevWord.data(WIDTH - 1, offset * 8);
                sendWord.keep(WIDTH / 8 - 1, WIDTH / 8 - (offset)) = currWord.keep(offset - 1, 0);
                sendWord.keep(WIDTH / 8 - (offset)-1, 0) = prevWord.keep(WIDTH / 8 - 1, offset);
                sendWord.last = (currWord.keep[offset] == 0);
                outWord.data = sendWord.data;
                outWord.keep = sendWord.keep;
                outWord.last = sendWord.last;
                dataOut.write(outWord);                
                prevWord = currWord;
                if (currWord.last) {
                    rxMemWrState = IDLE;
                    if (!sendWord.last) {
                        rxMemWrState = RESIDUE;
                    }
                }
            }
            break;
        case RESIDUE: // last word
            net_axis<WIDTH> sendWord;
#ifndef __SYNTHESIS__
            sendWord.data(WIDTH - 1, WIDTH - (offset * 8)) = 0;
#endif
            sendWord.data(WIDTH - (offset * 8) - 1, 0) = prevWord.data(WIDTH - 1, offset * 8);
            sendWord.keep(WIDTH / 8 - 1, WIDTH / 8 - (offset)) = 0;
            sendWord.keep(WIDTH / 8 - (offset)-1, 0) = prevWord.keep(WIDTH / 8 - 1, offset);
            sendWord.last = 1;
            outWord.data = sendWord.data;
            outWord.keep = sendWord.keep;
            outWord.last = sendWord.last;
            dataOut.write(outWord);                        
            rxMemWrState = IDLE;
            break;
    } // switch
}

/** @ingroup rx_engine
 * Increments stat counters based on flag FIFOs
 *  @param[in]      dataIn, incoming data stream
 *  @param[in]      cmdIn, incoming command from rxTcpFSM
 *  @param[out]     cmdOut, outgoing memory command
 *  @param[out]     dataOut, outgoing data for memory
 *  @param[out]     doubleAccess, outgoing value if 2 writes are required to memory
 */
void statCounters(hls::stream<ap_uint<1> >& stats_tcpAttemptFails_in,
                  hls::stream<ap_uint<1> >& stats_tcpEstabResets_in,
                  hls::stream<ap_uint<1> >& stats_tcpInSegs_in,
                  hls::stream<ap_uint<1> >& stats_tcpInErrs_in,
                  uint32_t& stats_tcpAttemptFails,
                  uint32_t& stats_tcpEstabResets,
                  uint32_t& stats_tcpInSegs,
                  uint32_t& stats_tcpInErrs) {
#pragma HLS pipeline II = 1
#pragma HLS INLINE off

    static uint32_t cnt_tcpAttemptFails = 0;
    static uint32_t cnt_tcpEstabResets = 0;
    static uint32_t invalidRecv = 0;
    static uint32_t recvSeg = 0;

    if (!stats_tcpAttemptFails_in.empty()) {
        stats_tcpAttemptFails_in.read();
        cnt_tcpAttemptFails++;
    }
    if (!stats_tcpEstabResets_in.empty()) {
        stats_tcpEstabResets_in.read();
        cnt_tcpEstabResets++;
    }
    if (!stats_tcpInSegs_in.empty()) {
        stats_tcpInSegs_in.read();
        recvSeg++;
    }
    if (!stats_tcpInErrs_in.empty()) {
        stats_tcpInErrs_in.read();
        invalidRecv++;
    }    
    stats_tcpAttemptFails = cnt_tcpAttemptFails;
    stats_tcpEstabResets  = cnt_tcpEstabResets;
    stats_tcpInSegs       = recvSeg;
    stats_tcpInErrs       = invalidRecv;
}

/** @ingroup rx_engine
 *  The @ref rx_engine is processing the data packets on the receiving path.
 *  When a new packet enters the engine its TCP checksum is tested, afterwards the header is parsed
 *  and some more checks are done. Before it is evaluated by the main TCP state machine which triggers Events
 *  and updates the data structures depending on the packet. If the packet contains valid payload it is stored
 *  in memory and the application is notified about the new data.
 *  @param[in]      ipRxData
 *  @param[in]      sLookup2rxEng_rsp
 *  @param[in]      stateTable2rxEng_upd_rsp
 *  @param[in]      portTable2rxEng_rsp
 *  @param[in]      rxSar2rxEng_upd_rsp
 *  @param[in]      txSar2rxEng_upd_rsp
 *  @param[in]      rxBufferWriteStatus
 *
 *  @param[out]     rxBufferWriteData
 *  @param[out]     rxEng2sLookup_req
 *  @param[out]     rxEng2stateTable_upd_req
 *  @param[out]     rxEng2portTable_req
 *  @param[out]     rxEng2rxSar_upd_req
 *  @param[out]     rxEng2txSar_upd_req
 *  @param[out]     rxEng2timer_clearRetransmitTimer
 *  @param[out]     rxEng2timer_setCloseTimer
 *  @param[out]     openConStatusOut
 *  @param[out]     rxEng2eventEng_setEvent
 *  @param[out]     rxBufferWriteCmd
 *  @param[out]     rxEng2rxApp_notification
 */
template <int WIDTH>
void rx_engine(hls::stream<ap_axiu<WIDTH,0,0,0> >& ipRxData,
               hls::stream<sessionLookupReply>& sLookup2rxEng_rsp,
               hls::stream<sessionState>& stateTable2rxEng_upd_rsp,
               hls::stream<bool>& portTable2rxEng_rsp,
               hls::stream<rxSarEntry>& rxSar2rxEng_upd_rsp,
               hls::stream<rxTxSarReply>& txSar2rxEng_upd_rsp,
#if !(RX_DDR_BYPASS)
               hls::stream<ap_axiu<8,0,0,0> >& rxBufferWriteStatus,
#endif
               hls::stream<ap_axiu<WIDTH,0,0,0> >& rxBufferWriteData,
               hls::stream<sessionLookupQuery>& rxEng2sLookup_req,
               hls::stream<stateQuery>& rxEng2stateTable_upd_req,
               hls::stream<ap_uint<16> >& rxEng2portTable_req,
               hls::stream<rxSarRecvd>& rxEng2rxSar_upd_req,
               hls::stream<rxTxSarQuery>& rxEng2txSar_upd_req,
               hls::stream<rxRetransmitTimerUpdate>& rxEng2timer_clearRetransmitTimer,
               hls::stream<ap_uint<16> >& rxEng2timer_clearProbeTimer,
               hls::stream<ap_uint<16> >& rxEng2timer_setCloseTimer,
               hls::stream<openStatus>& openConStatusOut,
               hls::stream<extendedEvent>& rxEng2eventEng_setEvent,
               uint32_t& stats_tcpInSegs,
               uint32_t& stats_tcpInErrs,
               uint32_t& stats_tcpAttemptFails,
               uint32_t& stats_tcpEstabResets,
#if !(RX_DDR_BYPASS)
               hls::stream<ap_axiu<128,0,0,0> >& rxBufferWriteCmd,
               hls::stream<appNotification>& rxEng2rxApp_notification)
#else
               hls::stream<appNotification>& rxEng2rxApp_notification,
               ap_uint<16> rxbuffer_data_count,
               ap_uint<16> rxbuffer_max_data_count)
#endif
{
#pragma HLS INLINE

    // Axi Streams
    static hls::stream<net_axis<WIDTH> > rxEng_dataBuffer1("rxEng_dataBuffer1");
    static hls::stream<net_axis<WIDTH> > rxEng_dataBuffer2("rxEng_dataBuffer2");
    static hls::stream<net_axis<WIDTH> > rxEng_dataBuffer3("rxEng_dataBuffer3");
    static hls::stream<net_axis<WIDTH> > rxEng_dataBuffer3a("rxEng_dataBuffer3a");
    static hls::stream<net_axis<WIDTH> > rxEng_dataBuffer3b("rxEng_dataBuffer3b");
#pragma HLS stream variable = rxEng_dataBuffer1 depth = 8
#pragma HLS stream variable = rxEng_dataBuffer2 depth = 256 // critical, tcp checksum computation
#pragma HLS stream variable = rxEng_dataBuffer3 depth = 32
#pragma HLS stream variable = rxEng_dataBuffer3a depth = 8
#pragma HLS stream variable = rxEng_dataBuffer3b depth = 8

    // Meta Streams/FIFOs
    static hls::stream<rxEngineMetaData> rxEng_metaDataFifo("rx_metaDataFifo");
    static hls::stream<rxFsmMetaData> rxEng_fsmMetaDataFifo("rxEng_fsmMetaDataFifo");
    static hls::stream<fourTuple> rxEng_tupleBuffer("rx_tupleBuffer");
    static hls::stream<pseudoMeta> rxEng_ipMetaFifo("rxEng_ipMetaFifo");
#pragma HLS stream variable = rxEng_metaDataFifo depth = 2
#pragma HLS stream variable = rxEng_fsmMetaDataFifo depth = 2
#pragma HLS stream variable = rxEng_tupleBuffer depth = 2
#pragma HLS stream variable = rxEng_ipMetaFifo depth = 3

    static hls::stream<extendedEvent> rxEng_metaHandlerEventFifo("rxEng_metaHandlerEventFifo");
    static hls::stream<event> rxEng_fsmEventFifo("rxEng_fsmEventFifo");
#pragma HLS stream variable = rxEng_metaHandlerEventFifo depth = 3
#pragma HLS stream variable = rxEng_fsmEventFifo depth = 2

    static hls::stream<bool> rxEng_metaHandlerDropFifo("rxEng_metaHandlerDropFifo");
    static hls::stream<bool> rxEng_fsmDropFifo("rxEng_fsmDropFifo");
#pragma HLS stream variable = rxEng_metaHandlerDropFifo depth = 3
#pragma HLS stream variable = rxEng_fsmDropFifo depth = 4 // Increased from 2

    static hls::stream<appNotification> rx_internalNotificationFifo("rx_internalNotificationFifo");
#pragma HLS stream variable = rx_internalNotificationFifo depth = 8 // This depends on the memory delay

    static hls::stream<mmCmd> rxTcpFsm2wrAccessBreakdown("rxTcpFsm2wrAccessBreakdown");
#pragma HLS stream variable = rxTcpFsm2wrAccessBreakdown depth = 8

    static hls::stream<net_axis<WIDTH> > rxPkgDrop2rxMemWriter("rxPkgDrop2rxMemWriter");
#pragma HLS stream variable = rxPkgDrop2rxMemWriter depth = 16

    static hls::stream<ap_uint<1> > rxEngDoubleAccess("rxEngDoubleAccess");
#pragma HLS stream variable = rxEngDoubleAccess depth = 8

    static hls::stream<net_axis<WIDTH> > rxEng_dataBuffer4("rxEng_dataBuffer4");
    static hls::stream<net_axis<WIDTH> > rxEng_dataBuffer5("rxEng_dataBuffer5");
#pragma HLS stream variable = rxEng_dataBuffer4 depth = 8
#pragma HLS stream variable = rxEng_dataBuffer5 depth = 8

    static hls::stream<ap_uint<1> > stats_tcpAttemptFails_fifo("stats_tcpAttemptFails_fifo");
    static hls::stream<ap_uint<1> > stats_tcpEstabResets_fifo("stats_tcpEstabResets_fifo");
    static hls::stream<ap_uint<1> > stats_tcpInSegs_fifo("stats_tcpInSegs_fifo");
    static hls::stream<ap_uint<1> > stats_tcpInErrs_fifo("stats_tcpInErrs_fifo");
#pragma HLS STREAM depth = 8 variable = stats_tcpAttemptFails_fifo
#pragma HLS STREAM depth = 8 variable = stats_tcpEstabResets_fifo
#pragma HLS STREAM depth = 8 variable = stats_tcpInSegs_fifo
#pragma HLS STREAM depth = 8 variable = stats_tcpInErrs_fifo

    process_ipv4<WIDTH>(ipRxData, rxEng_ipMetaFifo, rxEng_dataBuffer4);
    // align
    lshiftWordByOctet<WIDTH, net_axis<WIDTH>, net_axis<WIDTH>, 2>(((TCP_PSEUDO_HEADER_SIZE % WIDTH) / 8), rxEng_dataBuffer4, rxEng_dataBuffer5);

    prependPseudoHeader<WIDTH>(rxEng_ipMetaFifo, rxEng_dataBuffer5, rxEng_dataBuffer1);

    static hls::stream<subSums<WIDTH / 16> > subSumFifo("subSumFifo");
    static hls::stream<bool> rxEng_checksumValidFifo("rxEng_checksumValidFifo");
    static hls::stream<optionalFieldsMeta> rxEng_optionalFieldsMetaFifo("rxEng_optionalFieldsMetaFifo");
#pragma HLS stream variable = subSumFifo depth = 2
#pragma HLS stream variable = rxEng_checksumValidFifo depth = 2
#pragma HLS stream variable = rxEng_optionalFieldsMetaFifo depth = 8
#if (WINDOW_SCALE)
    static hls::stream<rxEngineMetaData> rxEng_headerMetaFifo("rxEng_headerMetaFifo");
    static hls::stream<ap_uint<4> > rxEng_dataOffsetFifo("rxEng_dataOffsetFifo");
    static hls::stream<ap_uint<320> > rxEng_optionalFieldsFifo("rxEng_optionalFieldsFifo");
    static hls::stream<ap_uint<4> > rxEng_winScaleFifo("rxEng_winScaleFifo");
#pragma HLS stream variable = rxEng_headerMetaFifo depth = 16
#pragma HLS stream variable = rxEng_dataOffsetFifo depth = 2
#pragma HLS stream variable = rxEng_optionalFieldsFifo depth = 2
#pragma HLS stream variable = rxEng_winScaleFifo depth = 2
#endif

    two_complement_subchecksums<WIDTH, 11>(rxEng_dataBuffer1, rxEng_dataBuffer2, subSumFifo);
    check_ipv4_checksum(subSumFifo, rxEng_checksumValidFifo);
    processPseudoHeader<WIDTH>(rxEng_dataBuffer2, rxEng_dataBuffer3a, rxEng_checksumValidFifo,
#if !(WINDOW_SCALE)
                               rxEng_metaDataFifo,
#else
                               rxEng_headerMetaFifo,
#endif
                               rxEng_tupleBuffer, rxEng2portTable_req, rxEng_optionalFieldsMetaFifo, stats_tcpInErrs_fifo,
                               stats_tcpInSegs_fifo);

    rshiftWordByOctet<WIDTH, net_axis<WIDTH>, net_axis<WIDTH>, 3>(((TCP_FULL_PSEUDO_HEADER_SIZE % WIDTH) / 8), rxEng_dataBuffer3a,
                                                 rxEng_dataBuffer3b);

    drop_optional_header_fields<WIDTH>(rxEng_optionalFieldsMetaFifo, rxEng_dataBuffer3b,
#if (WINDOW_SCALE)
                                       rxEng_dataOffsetFifo, rxEng_optionalFieldsFifo,
#endif
                                       rxEng_dataBuffer3);

#if (WINDOW_SCALE)
    parse_optional_header_fields(rxEng_dataOffsetFifo, rxEng_optionalFieldsFifo, rxEng_winScaleFifo);
    merge_header_meta(rxEng_winScaleFifo, rxEng_headerMetaFifo, rxEng_metaDataFifo);
#endif

    rxMetadataHandler(rxEng_metaDataFifo, sLookup2rxEng_rsp, portTable2rxEng_rsp, rxEng_tupleBuffer, rxEng2sLookup_req,
                      rxEng_metaHandlerEventFifo, rxEng_metaHandlerDropFifo, rxEng_fsmMetaDataFifo);

    rxTcpFSM(rxEng_fsmMetaDataFifo, stateTable2rxEng_upd_rsp, rxSar2rxEng_upd_rsp, txSar2rxEng_upd_rsp,
             rxEng2stateTable_upd_req, rxEng2rxSar_upd_req, rxEng2txSar_upd_req, rxEng2timer_clearRetransmitTimer,
             rxEng2timer_clearProbeTimer, rxEng2timer_setCloseTimer, openConStatusOut, rxEng_fsmEventFifo,
             rxEng_fsmDropFifo, stats_tcpAttemptFails_fifo, stats_tcpEstabResets_fifo,
#if !(RX_DDR_BYPASS)
             rxTcpFsm2wrAccessBreakdown, rx_internalNotificationFifo);
#else
             rxEng2rxApp_notification, rxbuffer_data_count, rxbuffer_max_data_count);
#endif

#if !(RX_DDR_BYPASS)
    rxPackageDropper<WIDTH>(rxEng_dataBuffer3, rxEng_metaHandlerDropFifo, rxEng_fsmDropFifo, rxPkgDrop2rxMemWriter);

    rxEngMemWrite<WIDTH>(rxPkgDrop2rxMemWriter, rxTcpFsm2wrAccessBreakdown, rxBufferWriteCmd, rxBufferWriteData,
                         rxEngDoubleAccess);

    rxAppNotificationDelayer(rxBufferWriteStatus, rx_internalNotificationFifo, rxEng2rxApp_notification,
                             rxEngDoubleAccess);
#else
    rxPackageDropper(rxEng_dataBuffer3, rxEng_metaHandlerDropFifo, rxEng_fsmDropFifo, rxBufferWriteData);
#endif
    rxEventMerger(rxEng_metaHandlerEventFifo, rxEng_fsmEventFifo, rxEng2eventEng_setEvent);

    statCounters(stats_tcpAttemptFails_fifo, stats_tcpEstabResets_fifo, stats_tcpInSegs_fifo, stats_tcpInErrs_fifo, stats_tcpAttemptFails, stats_tcpEstabResets, stats_tcpInSegs, stats_tcpInErrs);
}

template void rx_engine<DATA_WIDTH>(hls::stream<ap_axiu<DATA_WIDTH,0,0,0> >& ipRxData,
                                    hls::stream<sessionLookupReply>& sLookup2rxEng_rsp,
                                    hls::stream<sessionState>& stateTable2rxEng_upd_rsp,
                                    hls::stream<bool>& portTable2rxEng_rsp,
                                    hls::stream<rxSarEntry>& rxSar2rxEng_upd_rsp,
                                    hls::stream<rxTxSarReply>& txSar2rxEng_upd_rsp,
#if !(RX_DDR_BYPASS)
                                    hls::stream<ap_axiu<8,0,0,0> >& rxBufferWriteStatus,
#endif
                                    hls::stream<ap_axiu<DATA_WIDTH,0,0,0> >& rxBufferWriteData,
                                    hls::stream<sessionLookupQuery>& rxEng2sLookup_req,
                                    hls::stream<stateQuery>& rxEng2stateTable_upd_req,
                                    hls::stream<ap_uint<16> >& rxEng2portTable_req,
                                    hls::stream<rxSarRecvd>& rxEng2rxSar_upd_req,
                                    hls::stream<rxTxSarQuery>& rxEng2txSar_upd_req,
                                    hls::stream<rxRetransmitTimerUpdate>& rxEng2timer_clearRetransmitTimer,
                                    hls::stream<ap_uint<16> >& rxEng2timer_clearProbeTimer,
                                    hls::stream<ap_uint<16> >& rxEng2timer_setCloseTimer,
                                    hls::stream<openStatus>& openConStatusOut,
                                    hls::stream<extendedEvent>& rxEng2eventEng_setEvent,
                                    uint32_t& stats_tcpInSegs,
                                    uint32_t& stats_tcpInErrs,
                                    uint32_t& stats_tcpAttemptFails,
                                    uint32_t& stats_tcpEstabResets,
#if !(RX_DDR_BYPASS)
                                    hls::stream<ap_axiu<128,0,0,0> >& rxBufferWriteCmd,
                                    hls::stream<appNotification>& rxEng2rxApp_notification);
#else
                                    hls::stream<appNotification>& rxEng2rxApp_notification,
                                    ap_uint<16> rxbuffer_data_count,
                                    ap_uint<16> rxbuffer_max_data_count);
#endif
