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

/** @ingroup igmp_report_gen
 *  Multicast table FSM.  Can be triggered on either host operations to modify the record table
 * or a gen_report command from an igmp_timer expiring.
 *  @param[out]     ipTupleFifoOut
 *  @param[out]     igmp_v2_report_meta
 *  @param[out]     igmp_v3_report_meta
 *  @param[out]     is_v3
 *  @param[in]      gen_report
 *  @param[in]      host_op_toggle
 *  @param[in]      host_opcode
 *  @param[in]      host_ipAddr
 *  @param[in]      host_tableAddr
 *  @param[in]      igmp_ver
 *  @param[out]     host_readVal
 *  @param[in]      enable
 */
void igmp_record_table(hls::stream<igmpQuery>& gen_report,
                 hls::stream<igmpReportMeta>& igmp_v2_report_meta,
                 hls::stream<igmpReportMeta>& igmp_v3_report_meta,
                 hls::stream<bool>& is_v3,
                 bool host_op_toggle,
                 ap_uint<2> host_opcode,
                 uint32_t host_ipAddr,
                 uint8_t host_tableAddr,
                 uint8_t igmp_ver,
                 uint32_t& host_readVal,
                 bool enable) {
#pragma HLS PIPELINE II = 1
#define TABLE_ADDR_MASK 0xFF

    enum igmpReportFsmStateType {
        IDLE,
        ADD_ENTRY,
        DELETE_ENTRY,
        READ_ENTRY,
        CURRENT_STATE_RECORDS,
        GROUP_MATCH,
        WRITE_META
    };
    static igmpReportFsmStateType igmpReportFsmState = IDLE;
    igmpReportMeta ReportMeta;
    static igmpQuery query;
    static bool toggle;
    static bool is_state_record;
    static uint32_t ipTable[MAX_IP_ENTRIES];
    static uint8_t nEntries = 0;
    static uint8_t nCurrEntry = 0;
    static uint8_t nRecords = 0;
    static bool fMatch = false;
    static bool fInsert = false;
    static ap_uint<2> _opcode = IP_READ;
    static uint32_t _hostIp = 0;
    static uint8_t _tableAddr = 0;
    static uint8_t _igmp_ver = 3;
    static uint8_t mode = 0;
    static uint8_t candidateEntry = 0;
    static ap_uint<32> ipAddr;
    static ap_uint<32> group_address;
    //ap_uint<32> group_address;
    switch (igmpReportFsmState) {
        case IDLE:
            is_state_record = false;
            nCurrEntry = 0;
            if (host_op_toggle != toggle) {
                // Take local copies of the register in case it changes
                // during the operation
                _hostIp = host_ipAddr;
                _opcode = host_opcode;
                _tableAddr = host_tableAddr;
                _igmp_ver = igmp_ver;

                candidateEntry = 0;
                fMatch = false;
                fInsert = false;
                switch (_opcode) {
                    case IP_ADD:
                        if (enable) {
                            igmpReportFsmState = ADD_ENTRY;
                        }
                        break;
                    case IP_DELETE:
                        if (enable) {
                            igmpReportFsmState = DELETE_ENTRY;
                        }
                        break;
                    case IP_READ:
                        igmpReportFsmState = READ_ENTRY;
                        break;
                }
                toggle = host_op_toggle;
                if(_igmp_ver==3){
                    is_v3.write(true);
                } else {
                    is_v3.write(false);
                }
            } else if (!gen_report.empty()) {
                gen_report.read(query);
                if (enable) {
                    if (query.groupAddr != 0) {
                        igmpReportFsmState = GROUP_MATCH;
                    } else if (nEntries > 0) {
                        is_state_record = true;
                        igmpReportFsmState = CURRENT_STATE_RECORDS;
                    }
                }
            }
            break;
        case ADD_ENTRY:
            // First Check if it already exists in the table, then check if it
            // is a free entry
            if (ipTable[nCurrEntry] == _hostIp) {
                fMatch = true;
            } else if (ipTable[nCurrEntry] == 0) {
                candidateEntry = nCurrEntry;
                fInsert = true;
            }

            if (nCurrEntry == LAST_IP_ENTRY) {
                if (fInsert && !fMatch) {
                    ipTable[candidateEntry] = _hostIp;
                    ipAddr = _hostIp;
                    mode = _opcode;
                    nEntries++;
                    igmpReportFsmState = WRITE_META;
                } else {
                    // Failed to insert an entry - just return to IDLE
                    igmpReportFsmState = IDLE;
                }
            }
            nCurrEntry++;
            break;
        case DELETE_ENTRY:
            if (ipTable[nCurrEntry] == _hostIp) {
                ipTable[nCurrEntry] = 0;
                nEntries--;
                fMatch = true;
            }
            if (nCurrEntry == LAST_IP_ENTRY) {
                if (fMatch) {
                    ipAddr = _hostIp;
                    mode = _opcode;
                    igmpReportFsmState = WRITE_META;
                } else {
                    igmpReportFsmState = IDLE;
                }
            }
            nCurrEntry++;
            break;
        case READ_ENTRY:
            host_readVal = ipTable[(_tableAddr & TABLE_ADDR_MASK) >> 2];
            igmpReportFsmState = IDLE;
            break;
        case CURRENT_STATE_RECORDS:
            ipAddr = ipTable[nCurrEntry];
            if (ipAddr != 0) {
                nRecords++;
                mode = 0;
                igmpReportFsmState = WRITE_META;
            } else if (nCurrEntry == LAST_IP_ENTRY) {
                igmpReportFsmState = IDLE;
            }
            nCurrEntry++;
            break;
        case GROUP_MATCH:
            // See if we match this group
            ipAddr = ipTable[nCurrEntry];
            if (ipAddr == query.groupAddr) {
                group_address = query.groupAddr;
                nRecords = 0; // TODO: check if necessary
                mode = 0;
                igmpReportFsmState = WRITE_META;
            } else if (nCurrEntry == LAST_IP_ENTRY) {
                igmpReportFsmState = IDLE;
            }
            nCurrEntry++;
            break;
        case WRITE_META:
            ReportMeta.version = _igmp_ver;
            ReportMeta.is_state_record = is_state_record;
            ReportMeta.ip_address = ipAddr;
            ReportMeta.group_address = group_address;
            ReportMeta.mode = mode; 
            ReportMeta.nEntries = nEntries;
            if(_igmp_ver == 2){
                igmp_v2_report_meta.write(ReportMeta);
            } else if (_igmp_ver == 3){
                igmp_v3_report_meta.write(ReportMeta);
            }
            // If its a group record we need to go back to
            // group match again
            if (is_state_record==true && nEntries != nRecords){
                igmpReportFsmState = CURRENT_STATE_RECORDS;
            } else {
                nRecords = 0;
                igmpReportFsmState = IDLE;
            }
            break;
    }
}

/** @ingroup igmp_report_gen
 *  v2 report generation function. 
 *  generates a v2 report based on trigger from igmp_report
 *  @param[in]      igmp_v3_report_meta
 *  @param[out]     mcastReportData
 *  @param[out]     ipTupleFifoOut
 */
void igmp_v2_report(hls::stream<igmpReportMeta>& igmp_v2_report_meta,
                 hls::stream<twoTuple>& ipTupleFifoOut,
                 hls::stream<axiWord>& mcastReportData) {
#pragma HLS PIPELINE II = 1

    enum igmpV2ReportFsmStateType {
        IDLE,
        MEMBERSHIP_QUERY,
        LEAVE_QUERY,
        WRITE_META
    };

    static igmpV2ReportFsmStateType igmpV2ReportFsmState = IDLE;
    axiWord currWord;
    static igmpReportMeta report_meta; 
    static ap_uint<32> destAddr;
    ap_uint<32> ipAddr; // TODO: move to static?

    switch (igmpV2ReportFsmState) {
        case IDLE:
            if (!igmp_v2_report_meta.empty()) {
                igmp_v2_report_meta.read(report_meta);
                if(report_meta.version==2){
                    switch (report_meta.mode) {
                        case IP_NOOP:
                            igmpV2ReportFsmState = MEMBERSHIP_QUERY;
                            break;
                        case IP_ADD:
                            igmpV2ReportFsmState = MEMBERSHIP_QUERY;
                            break;
                        case IP_DELETE:
                            igmpV2ReportFsmState = LEAVE_QUERY;
                            break;
                    }
                }
            }
            break;
        case MEMBERSHIP_QUERY:
            ipAddr = report_meta.ip_address;
            destAddr = report_meta.ip_address;
            currWord.data(7, 0)   = 0x16;
            currWord.data(15, 8)  = 0x00;    // Reserved
            currWord.data(31, 16) = 0x0000; // Checksum, to be filled in later
            currWord.data(63, 56) = ipAddr(7, 0);
            currWord.data(55, 48) = ipAddr(15, 8);
            currWord.data(47, 40) = ipAddr(23, 16);
            currWord.data(39, 32) = ipAddr(31, 24); // Multicast Address
            currWord.keep = 0xFF;
            currWord.last = 1;
            mcastReportData.write(currWord);
            igmpV2ReportFsmState = WRITE_META;
            break;
        case LEAVE_QUERY:
            ipAddr = report_meta.ip_address;
            destAddr = IP_V2_LEAVE_MULTICAST;
            currWord.data(7, 0)   = 0x17;
            currWord.data(15, 8)  = 0x00;    // Reserved
            currWord.data(31, 16) = 0x0000; // Checksum, to be filled in later
            currWord.data(63, 56) = ipAddr(7, 0);
            currWord.data(55, 48) = ipAddr(15, 8);
            currWord.data(47, 40) = ipAddr(23, 16);
            currWord.data(39, 32) = ipAddr(31, 24); // Multicast Address
            currWord.keep = 0xFF;
            currWord.last = 1;
            mcastReportData.write(currWord);
            igmpV2ReportFsmState = WRITE_META;
            break;
        case WRITE_META:
            ipTupleFifoOut.write(twoTuple(8, destAddr));
            igmpV2ReportFsmState = IDLE;
            break;
    }
}


/** @ingroup igmp_report_gen
 *  v3 report generation function. 
 *  generates a v3 report based on trigger from igmp_report
 *  @param[in]      igmp_v3_report_meta
 *  @param[out]     mcastReportData
 *  @param[out]     ipTupleFifoOut
 */
void igmp_v3_report(hls::stream<igmpReportMeta>& igmp_v3_report_meta,
                 hls::stream<twoTuple>& ipTupleFifoOut,
                 hls::stream<axiWord>& mcastReportData) {
#pragma HLS PIPELINE II = 1

    enum igmpV3ReportFsmStateType {
        IDLE,
        FILTER_CHANGE_QUERY,
        GENERAL_QUERY,
        FILTER_CHANGE_RECORD,
        CURRENT_STATE_RECORDS,
        WRITE_META,
        GROUP_MATCH,
        GROUP_QUERY,
        GROUP_RECORD
    };

    static igmpV3ReportFsmStateType igmpV3ReportFsmState = IDLE;
    axiWord currWord;
    static ap_uint16_t nLength = 0;
    uint8_t mode = 0;
    static uint8_t nEntries = 0;
    static uint8_t nRecords = 0;
    static igmpReportMeta report_meta; 
    static bool is_first_record = true;
    static uint32_t ipTable[MAX_IP_ENTRIES];
    ap_uint<32> ipAddr; // TODO: make it static?

    switch (igmpV3ReportFsmState) {
        case IDLE:
            if (!igmp_v3_report_meta.empty()) {
                igmp_v3_report_meta.read(report_meta);
                if(report_meta.version==3){
                    switch (report_meta.mode) {
                        case IP_NOOP:
                            if(report_meta.is_state_record==true){
                                ipTable[nRecords] = report_meta.ip_address;
                                if(is_first_record==true){
                                    igmpV3ReportFsmState = GENERAL_QUERY;
                                } else{
                                    igmpV3ReportFsmState = CURRENT_STATE_RECORDS;
                                }
                            } else {
                                igmpV3ReportFsmState = GROUP_QUERY;
                            }
                            break;
                        case IP_ADD:
                            igmpV3ReportFsmState = FILTER_CHANGE_QUERY;
                            break;
                        case IP_DELETE:
                            igmpV3ReportFsmState = FILTER_CHANGE_QUERY;
                            break;
                    }
                }
            }
            break;
        case FILTER_CHANGE_QUERY:
            currWord.data(7, 0)   = 0x22;
            currWord.data(15, 8)  = 0x00;    // Reserved
            currWord.data(31, 16) = 0x0000; // Checksum, to be filled in later
            currWord.data(47, 32) = 0x0000; // Reserved
            currWord.data(55, 48) = 0x0;
            currWord.data(63, 56) = 0x1; // 1 Record (filter change)
            currWord.keep = 0xFF;
            currWord.last = 0;
            mcastReportData.write(currWord);
            nLength = 8;
            igmpV3ReportFsmState = FILTER_CHANGE_RECORD;
            break;
        case FILTER_CHANGE_RECORD:
            ipAddr = report_meta.ip_address;
            mode = 0;
            if (report_meta.mode == IP_ADD) {  // Exclude Nothing
                mode = 0x04;                   // CHANGE_TO_EXCLUDE
            } else if (report_meta.mode == IP_DELETE) { // Include Nothing
                mode = 0x03;                            // CHANGE_TO_INCLUDE
            }
            currWord.data(7, 0)   = mode;
            currWord.data(15, 8)  = 0x00;  // Reserved
            currWord.data(31, 16) = 0x00; // Number of Sources
            currWord.data(63, 56) = ipAddr(7, 0);
            currWord.data(55, 48) = ipAddr(15, 8);
            currWord.data(47, 40) = ipAddr(23, 16);
            currWord.data(39, 32) = ipAddr(31, 24); // Multicast Address
            currWord.keep = 0xFF;
            currWord.last = 1;
            nLength += 8;
            mcastReportData.write(currWord);
            igmpV3ReportFsmState = WRITE_META;
            break;
        case GROUP_QUERY:
            currWord.data(7, 0)   = 0x22;
            currWord.data(15, 8)  = 0x00;    // Reserved
            currWord.data(31, 16) = 0x0000; // Checksum, to be filled in later
            currWord.data(47, 32) = 0x0000; // Reserved
            currWord.data(55, 48) = 0x0;
            currWord.data(63, 56) = 1; // Number of Group Records
            currWord.keep = 0xFF;
            currWord.last = 0;
            nLength = 8;
            igmpV3ReportFsmState = GROUP_RECORD;
            mcastReportData.write(currWord);
            nRecords = 0;
            break;
        case GROUP_RECORD:
            currWord.data(7, 0)   = 0x02;   // MODE_IS_EXCLUDE
            currWord.data(15, 8)  = 0x00;  // Reserved
            currWord.data(31, 16) = 0x00; // Number of Sources
            currWord.data(63, 56) = report_meta.group_address(7, 0);
            currWord.data(55, 48) = report_meta.group_address(15, 8);
            currWord.data(47, 40) = report_meta.group_address(23, 16);
            currWord.data(39, 32) = report_meta.group_address(31, 24);
            currWord.keep = 0xFF;
            currWord.last = 1;
            mcastReportData.write(currWord);
            nLength += 8;
            igmpV3ReportFsmState = WRITE_META;
            break;
        case GENERAL_QUERY:
            is_first_record=false;
            nEntries = report_meta.nEntries;
            currWord.data(7, 0) = 0x22;
            currWord.data(15, 8) = 0x00;    // Reserved
            currWord.data(31, 16) = 0x0000; // Checksum, to be filled in later
            currWord.data(47, 32) = 0x0000; // Reserved
            currWord.data(55, 48) = 0x0;
            currWord.data(63, 56) = nEntries; // Number of Group Records
            currWord.keep = 0xFF;
            currWord.last = (nEntries == 0);
            nLength = 8;
            if (nEntries == 0) { // Should never go into this one tbh
                igmpV3ReportFsmState = WRITE_META;
            } else {
                igmpV3ReportFsmState = CURRENT_STATE_RECORDS;
            }
            mcastReportData.write(currWord);
            nRecords = 0;
            break;
        case CURRENT_STATE_RECORDS:
            ipAddr = ipTable[nRecords];
            nRecords++;
            currWord.data(7, 0)   = 0x02;   // MODE_IS_EXCLUDE
            currWord.data(15, 8)  = 0x00;  // Reserved
            currWord.data(31, 16) = 0x00; // Number of Sources
            currWord.data(63, 56) = ipAddr(7, 0);
            currWord.data(55, 48) = ipAddr(15, 8);
            currWord.data(47, 40) = ipAddr(23, 16);
            currWord.data(39, 32) = ipAddr(31, 24); // Multicast Address
            currWord.keep = 0xFF;
            currWord.last = (nRecords == nEntries);
            mcastReportData.write(currWord);
            nLength += 8;
            if (nRecords == nEntries) {
                is_first_record = true;
                igmpV3ReportFsmState = WRITE_META;
            } else {
                igmpV3ReportFsmState = IDLE;
            }
            break;
        case WRITE_META:
            ipTupleFifoOut.write(twoTuple(nLength, IP_ALL_ROUTERS_MULTICAST));
            nLength = 0;
            igmpV3ReportFsmState = IDLE;
            break;
    }
}

/** @ingroup igmp_report_gen
 *  report arbitration function. 
 *  dispatch the tuple and mcast data depending on v2 or v3
 *  @param[in]      tuple_v2
 *  @param[in]      mcast_v2
 *  @param[in]      tuple_v3
 *  @param[in]      mcast_v3
 *  @param[out]     ipTupleFifoOut
 *  @param[out]     mcastReportData
 */
void igmp_report_arbitrate(hls::stream<twoTuple>& tuple_v2,
                           hls::stream<axiWord>& mcast_v2,
                           hls::stream<twoTuple>& tuple_v3,
                           hls::stream<axiWord>& mcast_v3,
                           hls::stream<twoTuple>& ipTupleFifoOut,
                           hls::stream<axiWord>& mcastReportData) {
#pragma HLS PIPELINE II = 1

    twoTuple currTuple;
    axiWord  currMcast;
    /*
        It is important to note that IGMP Host will NOT do v3 and v2 
        at the same time. It's one or the other.
        So we don't really have to worry about v2 tuple being mixed with 
        v3 mcast and vice-versa
    */
    if (!tuple_v2.empty()) {
      tuple_v2.read(currTuple);
      ipTupleFifoOut.write(currTuple);
    } else if (!tuple_v3.empty()){
      tuple_v3.read(currTuple);
      ipTupleFifoOut.write(currTuple);    
    }
    if (!mcast_v2.empty()) {
      mcast_v2.read(currMcast);
      mcastReportData.write(currMcast);
    } else if (!mcast_v3.empty()){
      mcast_v3.read(currMcast);
      mcastReportData.write(currMcast);    
    }

}

/** @ingroup igmp_report_gen
 *  Reads the checksum and inserts it into
 *  @param[in]      dataIn
 *  @param[out]     dataOut
 *  @param[in]      ipChecksumFifoIn
 */
void insert_checksum(hls::stream<axiWord>& inData,
                     hls::stream<axiWord>& outData,
                     hls::stream<ap_uint16_t>& inChecksum) {
#pragma HLS pipeline II = 1

    static ap_uint<1> ic_wordCount = 0;
    axiWord currWord;
    ap_uint16_t checksum;

    switch (ic_wordCount) {
        case 0:
            if (!inChecksum.empty() && !inData.empty() && !outData.full()) {
                checksum = inChecksum.read();
                currWord = inData.read();
                currWord.data(31, 24) = checksum(7, 0);
                currWord.data(23, 16) = checksum(15, 8);
                outData.write(currWord);
                if (!currWord.last) { // Check for Single word (IGMP Membership report, 0 entries)
                    ic_wordCount = 1;
                }
            }
            break;
        case 1:
            if (!inData.empty() && !outData.full()) {
                currWord = inData.read();
                outData.write(currWord);
                if (currWord.last) {
                    ic_wordCount = 0;
                }
            }
            break;
        default:
            ic_wordCount = 0;
    }
}

/** @ingroup igmp_report_gen
 *  Reads the IP header metadata and the IP addresses. From this data it generates the IP header and streams it out.
 *  @param[in]      igmp_TupleFifoIn
 *  @param[out]     igmp_ipHeaderBufferOut
 *  @param[in]      srcIP
 */
void ipHeaderConstruction(hls::stream<twoTuple>& igmp_TupleFifoIn,
                          hls::stream<axiWord>& igmp_ipHeaderBufferOut,
                          uint32_t srcIp)

{
#pragma HLS INLINE off
#pragma HLS pipeline II = 1

    static ap_uint<2> ihc_currWord = 0;
    static twoTuple ihc_tuple;

    axiWord sendWord;
    ap_uint<16> length = 0;
    ap_uint<32> srcIpTmp;

    switch (ihc_currWord) {
        case 0:
            if (!igmp_TupleFifoIn.empty()) {
                igmp_TupleFifoIn.read(ihc_tuple);
                length = ihc_tuple.nLength;
                sendWord.data.range(7, 0) = 0x46;
                sendWord.data.range(15, 8) = 0;
                length = length + 24;
                sendWord.data.range(23, 16) = length(15, 8); // length
                sendWord.data.range(31, 24) = length(7, 0);
                sendWord.data.range(47, 32) = 0;
                sendWord.data.range(50, 48) = 0;   // Flags
                sendWord.data.range(63, 51) = 0x0; // Fragment Offset //FIXME why is this here
                sendWord.keep = 0xFF;
                sendWord.last = 0;
                igmp_ipHeaderBufferOut.write(sendWord);
                ihc_currWord++;
            }
            break;
        case 1:
            srcIpTmp = srcIp;
            sendWord.data.range(7, 0) = 0x01; // TTL=1
            sendWord.data.range(15, 8) = IGMP_PROTOCOL;
            sendWord.data.range(31, 16) = 0; // CS
            sendWord.data.range(63, 32) = srcIpTmp;
            sendWord.keep = 0xFF;
            sendWord.last = 0;
            igmp_ipHeaderBufferOut.write(sendWord);
            ihc_currWord++;
            break;
        case 2:
            sendWord.data.range(7, 0) = ihc_tuple.dstIp.range(31, 24);  // dstIp
            sendWord.data.range(15, 8) = ihc_tuple.dstIp.range(23, 16); // dstIp
            sendWord.data.range(23, 16) = ihc_tuple.dstIp.range(15, 8); // dstIp
            sendWord.data.range(31, 24) = ihc_tuple.dstIp.range(7, 0);  // dstIp
            sendWord.data.range(39, 32) = 0x94;
            sendWord.data.range(47, 40) = 0x04;
            sendWord.data.range(63, 48) = 0x0000;
            sendWord.keep = 0xFF;
            sendWord.last = 1;
            igmp_ipHeaderBufferOut.write(sendWord);
            ihc_currWord = 0;
            break;
    } // switch
}

/** @ingroup igmp_report_gen
 *  Reads the IP header stream and the payload stream, and combine them
 *  @param[in]      headerIn
 *  @param[in]      payloadIn
 *  @param[out]     dataOut
 */
void pkgStitcher(hls::stream<axiWord>& ipHeaderBufferIn,
                 hls::stream<axiWord>& payloadIn,
                 hls::stream<ap_axiu<64,0,0,0> >& ipTxDataOut) {
//#pragma HLS INLINE off
#pragma HLS pipeline II = 1

    static ap_uint<1> ps_wordCount = 0;
    axiWord headWord, dataWord;
    ap_axiu<64,0,0,0> sendWord;

    switch (ps_wordCount) {
        case 0:
            if (!ipHeaderBufferIn.empty()) {
                ipHeaderBufferIn.read(headWord);
                sendWord.data = headWord.data;
                sendWord.keep = headWord.keep;
                sendWord.last = 0;
                ipTxDataOut.write(sendWord);
                if (headWord.last) {
                    ps_wordCount++;
                }
            }
            break;
        case 1:
            if (!payloadIn.empty()) {
                payloadIn.read(dataWord);
                sendWord.data = dataWord.data;
                sendWord.keep = dataWord.keep;
                sendWord.last = dataWord.last;
                ipTxDataOut.write(sendWord);
                if (dataWord.last) {
                    ps_wordCount = 0;
                }
            }
            break;
        default:
            ps_wordCount = 0;
    }
}

void igmp_report_gen(hls::stream<ap_axiu<64,0,0,0> >& m_axis_data,
                     hls::stream<igmpQuery>& gen_report,
                     hls::stream<bool>& is_v3,
                     bool host_op_toggle,
                     ap_uint<2> host_opcode,
                     uint32_t host_ipAddr,
                     uint32_t srcIpAddr,
                     uint8_t host_tableAddr,
                     uint8_t igmp_ver,
                     uint32_t& host_readVal,
                     bool enable) {
    #pragma HLS DATAFLOW disable_start_propagation
    #pragma HLS INTERFACE axis port = m_axis_data
    static hls::stream<axiWord> report2subchecksum_v2("igmp_report2subchecksum_v2");
    #pragma HLS STREAM variable = report2subchecksum_v2 depth = 4
    static hls::stream<axiWord> report2subchecksum_v3("igmp_report2subchecksum_v3");
    #pragma HLS STREAM variable = report2subchecksum_v3 depth = 4
    static hls::stream<axiWord> report2subchecksum("igmp_report2subchecksum");
    #pragma HLS STREAM variable = report2subchecksum depth = 4
    static hls::stream<axiWord> subchecksum2stitcher("igmp_subchecksum2stitcher");
    #pragma HLS STREAM variable = subchecksum2stitcher depth = 64
    static hls::stream<subSums<4> > igmp_report_gen_subChecksumsFifo("igmp_report_gen_subChecksumsFifo");
    #pragma HLS stream variable = igmp_report_gen_subChecksumsFifo depth = 4
    static hls::stream<ap_uint16_t> igmp_report_gen_ChecksumFifo("igmp_report_genChecksumFifo");
    #pragma HLS stream variable = igmp_report_gen_ChecksumFifo depth = 4
    static hls::stream<axiWord> checksum2ipv4Fifo("igmp_report_gen_checksum2ipv4Fifo");
    #pragma HLS stream variable = checksum2ipv4Fifo depth = 64
    static hls::stream<twoTuple> twoTupleFIFO_v2("igmpTwoTupleFIFO_v2");
    #pragma HLS stream variable = twoTupleFIFO_v2 depth = 4
    static hls::stream<twoTuple> twoTupleFIFO_v3("igmpTwoTupleFIFO_v3");
    #pragma HLS stream variable = twoTupleFIFO_v3 depth = 4
    static hls::stream<twoTuple> twoTupleFIFO("igmpTwoTupleFIFO");
    #pragma HLS stream variable = twoTupleFIFO depth = 4
    static hls::stream<axiWord> ipHeaderBuffer("igmpIpHeaderFIFO");
    #pragma HLS stream variable = ipHeaderBuffer depth = 8
    static hls::stream<igmpReportMeta> igmp_v2_report_meta("igmpv2report_meta");
    #pragma HLS stream variable = ipHeaderBuffer depth = 4
    static hls::stream<igmpReportMeta> igmp_v3_report_meta("igmpv3report_meta");
    #pragma HLS stream variable = ipHeaderBuffer depth = 4

    igmp_record_table(gen_report,igmp_v2_report_meta, igmp_v3_report_meta, is_v3, host_op_toggle, host_opcode, host_ipAddr, host_tableAddr,
                igmp_ver, host_readVal, enable);
    igmp_v2_report(igmp_v2_report_meta, twoTupleFIFO_v2, report2subchecksum_v2);
    igmp_v3_report(igmp_v3_report_meta, twoTupleFIFO_v3, report2subchecksum_v3);
    igmp_report_arbitrate(twoTupleFIFO_v2, report2subchecksum_v2, twoTupleFIFO_v3, report2subchecksum_v3, twoTupleFIFO, report2subchecksum);
    two_complement_subchecksums<64, 14>(report2subchecksum, subchecksum2stitcher, igmp_report_gen_subChecksumsFifo);
    finalize_ipv4_checksum(igmp_report_gen_subChecksumsFifo, igmp_report_gen_ChecksumFifo);
    insert_checksum(subchecksum2stitcher, checksum2ipv4Fifo, igmp_report_gen_ChecksumFifo);
    ipHeaderConstruction(twoTupleFIFO, ipHeaderBuffer, srcIpAddr);
    pkgStitcher(ipHeaderBuffer, checksum2ipv4Fifo, m_axis_data);
}
