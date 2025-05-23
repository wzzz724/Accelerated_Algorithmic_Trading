/*
 * Copyright (c) 2019-2020, Xilinx, Inc.
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

#include "igmp.hpp"
#include "../ipv4/ipv4.hpp"
#include "../toe/two_complement_subchecksums.hpp"

bool isRouterAlertOption(ap_uint32_t word) {
#pragma HLS INLINE
    return (word == ROUTER_ALERT_NETWORK_ORDER);
}

bool isAllSystemsMulticast(uint32_t word) {
#pragma HLS INLINE
    return (word == IP_ALL_SYSTEMS_MULTICAST);
}
/** @ingroup igmp
 *  Removes the IP header from the packet. Also extracts the length of the IGMP data packet
 *  @param[in]		dataIn
 *  @param[out]		dataOut
 *  @param[out]     igmpIpMetaOut
 */
void remove_ip_header(hls::stream<ap_axiu<64,0,0,0> >& dataIn,
                      hls::stream<axiWord>& dataOut,
                      hls::stream<igmpIpMeta>& igmpIpMetaOut) {
#pragma HLS inline off
#pragma HLS pipeline II = 1
    enum riph_StateType { HEADER, DATA, LAST };
    static riph_StateType riph_state = HEADER;
    static ap_uint<3> riph_wordCount = 0;
    static ap_uint<4> ihl = 0;
    static bool shift = false;
    static ap_uint<3> cs_state = 0;
    static igmpIpMeta meta;
    axiWord prevWord, sendWord;
    ap_axiu<64,0,0,0> currWord;
    ap_uint32_t tmpAddr;
    ap_uint16_t tmpLength;

    switch (riph_state) {
        case HEADER:
            if (!dataIn.empty() && !dataOut.full()) {
                dataIn.read(currWord);
                switch (riph_wordCount) {
                    case 0:
                        ihl = currWord.data(3, 0);
                        tmpLength(15, 8) = currWord.data(23, 16);
                        tmpLength(7, 0) = currWord.data(31, 24);
                        meta.length = tmpLength;
                        meta.length -= (ihl * 4);
                        riph_wordCount++;
                        ihl -= 2;
                        break;
                    case 1:
                        ihl -= 2;
                        riph_wordCount++;
                        break;
                    case 2:
                        // Destination IP address
                        tmpAddr(7, 0) = currWord.data(31, 24);
                        tmpAddr(15, 8) = currWord.data(23, 16);
                        tmpAddr(23, 16) = currWord.data(15, 8);
                        tmpAddr(31, 24) = currWord.data(7, 0);
                        meta.dstAddr = tmpAddr;
                        if (ihl == 1) { // No options field
                            shift = true;
                            riph_state = DATA;
                            meta.routerAlertOption = false;
                        } else if (ihl == 2) {
                            shift = false;
                            riph_state = DATA;
                        }

                        if (ihl >= 2) {
                            meta.routerAlertOption = isRouterAlertOption(currWord.data(63, 32));
                            ihl -= 2;
                            riph_wordCount++;
                        }
                        break;
                    case 3: // Keep parsing options
                        if (ihl == 1) {
                            shift = true;
                            riph_state = DATA;
                        } else if (ihl == 2) {
                            shift = false;
                            riph_state = DATA;
                        }

                        if (ihl >= 1) {
                            meta.routerAlertOption |= isRouterAlertOption(currWord.data(31, 0));
                        }

                        if (ihl >= 2) {
                            ihl -= 2;
                            meta.routerAlertOption |= isRouterAlertOption(currWord.data(63, 32));
                        }
                        break;
                }
                prevWord.data = currWord.data;
                prevWord.keep = currWord.keep;
                prevWord.last = currWord.last;
                if (currWord.last) {
                    // If we get here then the IHL is incorrect. Restart
                    riph_wordCount = 0;
                    riph_state = HEADER;
                }
            }
            break;
        case DATA:
            riph_wordCount = 0;
            if (!dataIn.empty() && !dataOut.full()) {
                dataIn.read(currWord);
                if (shift) {
                    sendWord.data(31, 0) = prevWord.data(63, 32);
                    sendWord.keep(3, 0) = prevWord.keep(7, 4);
                    sendWord.data(63, 32) = currWord.data(31, 0);
                    sendWord.keep(7, 4) = currWord.keep(3, 0);
                    sendWord.last = (currWord.keep[4] == 0);
                } else {
                    sendWord.data = currWord.data;
                    sendWord.keep = currWord.keep;
                    sendWord.last = currWord.last;
                }
                dataOut.write(sendWord);
                prevWord.data = currWord.data;
                prevWord.keep = currWord.keep;
                prevWord.last = currWord.last;

                if (currWord.last) {
                    if (sendWord.last) {
                        riph_state = HEADER;
                        igmpIpMetaOut.write(meta);
                    } else {
                        riph_state = LAST;
                    }
                }
            }
            break;
        case LAST:
            // Send the last 4 bytes
            sendWord.data(31, 0) = prevWord.data(63, 32);
            sendWord.keep(3, 0) = prevWord.keep(7, 4);
            sendWord.data(63, 32) = 0;
            sendWord.keep(7, 4) = 0;
            sendWord.last = 1;
            dataOut.write(sendWord);
            igmpIpMetaOut.write(meta);
            riph_state = HEADER;
            break;
    }
}

/** @ingroup igmp
 *  For each packet it reads the checksum value from @param csum
 *  If the packet is valid the data stream & length are passed on
 *  If it is not valid it is dropped
 *  @param[in]		inData, incoming data stream
 *  @param[in]		inMeta metadata of the IGMP packet
 *  @param[out]		outData, outgoing data stream
 *  @param[out]		outMeta, metadata of the IGMP packet
 *  @param[in]		csum, Valid FIFO indicating if current packet is valid
 */
void drop_invalid(hls::stream<axiWord>& inData,
                  hls::stream<igmpIpMeta>& inMeta,
                  hls::stream<axiWord>& outData,
                  hls::stream<igmpIpMeta>& outMeta,
                  hls::stream<ap_uint16_t>& csum,
                  uint32_t& stats_invalid_csum) {
#pragma HLS INLINE off
#pragma HLS pipeline II = 1
    enum di_StateType { GET_CSUM, FWD, DROP };
    static di_StateType di_state = GET_CSUM;
    static uint32_t cnt_invalid_csum = 0;

    switch (di_state) {
        case GET_CSUM:
            if (!csum.empty() && !inMeta.empty()) {
                ap_uint16_t tmp_csum = csum.read();
                if (tmp_csum == 0) {
                    di_state = FWD;
                    outMeta.write(inMeta.read());
                } else {
                    di_state = DROP;
                    inMeta.read();
                    cnt_invalid_csum++;
                }
            }
            break;
        case FWD:
            if (!inData.empty()) {
                axiWord currWord = inData.read();
                outData.write(currWord);
                if (currWord.last) {
                    di_state = GET_CSUM;
                }
            }
            break;
        case DROP:
            if (!inData.empty()) {
                axiWord currWord = inData.read();
                if (currWord.last) {
                    di_state = GET_CSUM;
                }
            }
            break;
    } // switch
    stats_invalid_csum = cnt_invalid_csum;
}

/** @ingroup igmp
 *  Parse the IGMP packet and create the IGMP Query.
 *  Pick off the Type code. Max resp code and group address.
 *  Supports general query & group-specific queries.
 *  Does not support group-and-source-specific queries yet.
 *  @param[in]		inData, incoming data stream
 *  @param[in]		inMeta metadata of the IGMP packet
 *  @param[out]		outIgmpQuery, resulting query
 */
void parse_igmp(hls::stream<axiWord>& inData,
                hls::stream<igmpIpMeta>& inMeta,
                hls::stream<igmpQuery>& outIgmpQuery,
                uint32_t& stats_igmp_query,
                uint32_t& stats_invalid_query) {
#pragma HLS inline off
#pragma HLS pipeline II = 1

    axiWord currWord;
    static igmpIpMeta meta;
    static uint8_t maxRespCode = 0;
    static uint8_t typeNumber = 0;
    static ap_uint32_t groupAddr = 0;
    static uint32_t cnt_igmp_query = 0;
    static uint32_t cnt_invalid_query = 0;

    enum parse_StateType { GET_META, EXTRACT, CONSUME, DISPATCH, DROP };
    static parse_StateType parse_state = GET_META;

    switch (parse_state) {
        case GET_META:
            if (!inMeta.empty()) {
                inMeta.read(meta);
                parse_state = EXTRACT;
            }
            break;
        case EXTRACT:
            if (!inData.empty()) {
                if (!outIgmpQuery.full()){
                    inData.read(currWord);
                    typeNumber = currWord.data(7, 0);
                    maxRespCode = currWord.data(15, 8);
                    groupAddr(31, 24) = currWord.data(39, 32);
                    groupAddr(23, 16) = currWord.data(47, 40);
                    groupAddr(15, 8) = currWord.data(55, 48);
                    groupAddr(7, 0) = currWord.data(63, 56);
                    if (currWord.last) {
                        parse_state = DISPATCH;
                    }
                    else {
                        parse_state = CONSUME;
                    }
                } else {
                    parse_state = DROP;
                }
            }
            break;
        case DROP:
            if (!inData.empty()) {
                inData.read(currWord);
                if (currWord.last) {
                    parse_state = GET_META;
                }
            }
            break;
        case CONSUME:
            /*
              The IGMP standard tells that the IGMP payload might not contain
              only IGMP payload. Might contain more data that we should discard
              (see 4.1.10 in RFC3376)
              So we need to go through the payload until we see the tlast
              We drop the data anyways
            */
            if (!inData.empty()) {
                inData.read(currWord);
                // TODO: add SSM grabbing if necessary 
                if (currWord.last) {
                    parse_state = DISPATCH;
                }
            }
            break;
        case DISPATCH:
            igmpQuery tmp;
            bool valid = false;
            // Per RFC3376 these are the conditions for a valid IGMP query
            if (typeNumber == 0x11) {
                if ((meta.length == 8) && (maxRespCode == 0)) {
                    // v1 is exacly 8 so we can use the length 
                    valid = false; // we don't support V1
                } else { // We can't differentiate between 2 and 3 here 
                    valid = (meta.routerAlertOption);
                    // Now check if it is a general query
                    // If so the destination address should be 224.0.0.1
                    if (groupAddr == 0) {
                        valid &= isAllSystemsMulticast(meta.dstAddr);
                    }
                }
                if (valid) {
                    cnt_igmp_query++;
                    // TODO: need to move this on the timer and get the version
                    tmp.maxRespTime = maxRespCode;
                    tmp.groupAddr = groupAddr;
                    outIgmpQuery.write(tmp);
                } else {
                    cnt_invalid_query++;
                }
            }
            parse_state = GET_META;
            break;
    }
    stats_igmp_query = cnt_igmp_query;
    stats_invalid_query = cnt_invalid_query;
}

/** @ingroup igmp
 *  Parse valid IPv4 IGMP data for IGMP queries.
 *  @param[in]		inData, incoming data stream
 *  @param[out]		outIgmpQuery - valid IGMP query requests
 */
void igmp_parser(hls::stream<ap_axiu<64,0,0,0> >& inData,
                 hls::stream<igmpQuery>& outIgmpQuery,
                 uint32_t& stats_invalid_csum,
                 uint32_t& stats_igmp_query,
                 uint32_t& stats_invalid_query) {
#pragma HLS DATAFLOW disable_start_propagation
#pragma HLS INLINE off

    static hls::stream<axiWord> rih2csum("rih2csum");
    static hls::stream<axiWord> csum2drop("csum2drop"); // Must hold an entire IGMP frame
    static hls::stream<igmpIpMeta> meta2drop("meta2drop");
    static hls::stream<igmpIpMeta> meta2parse("meta2parse");
    static hls::stream<axiWord> data2parse("data2parse");
    static hls::stream<subSums<4> > igmp_parser_subChecksumsFifo("igmp_parser_subChecksumsFifo");
    static hls::stream<ap_uint16_t> igmp_parser_checksumFifo("igmp_parser_checksumFifo");

#pragma HLS stream variable = csum2drop depth = 64 // Must be big enough to hold entire IGMP IPv4 datagram
#pragma HLS stream variable = rih2csum depth = 4
#pragma HLS stream variable = meta2drop depth = 4
#pragma HLS stream variable = meta2parse depth = 4
#pragma HLS stream variable = data2parse depth = 64
#pragma HLS stream variable = igmp_parser_subChecksumsFifo depth = 4
#pragma HLS stream variable = igmp_parser_checksumFifo depth = 4

    remove_ip_header(inData, rih2csum, meta2drop);
    two_complement_subchecksums<64, 12>(rih2csum, csum2drop, igmp_parser_subChecksumsFifo);
    finalize_ipv4_checksum(igmp_parser_subChecksumsFifo, igmp_parser_checksumFifo);
    drop_invalid(csum2drop, meta2drop, data2parse, meta2parse, igmp_parser_checksumFifo, stats_invalid_csum);
    parse_igmp(data2parse, meta2parse, outIgmpQuery, stats_igmp_query, stats_invalid_query);
}
