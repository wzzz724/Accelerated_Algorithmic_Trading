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

#ifndef IGMP_HPP_INCLUDED
#define IGMP_HPP_INCLUDED
#include <stdint.h>
#include <hls_stream.h>
#include "ap_int.h"
#include "../axi_utils.hpp"
#include "ap_axi_sdata.h"

#define MAX_IP_ENTRIES 4
#define LAST_IP_ENTRY (MAX_IP_ENTRIES - 1)

const uint32_t IP_ALL_ROUTERS_MULTICAST = 0xE0000016; // 224.0.0.22
const uint32_t IP_ALL_SYSTEMS_MULTICAST = 0xE0000001; // 224.0.0.1
const uint32_t IP_V2_LEAVE_MULTICAST    = 0xE0000002; // 224.0.0.2
const uint32_t ROUTER_ALERT_NETWORK_ORDER = 0x00000494;
const uint8_t  IGMP_PROTOCOL = 0x02;
const uint8_t  IGMP_ROBUSTNESS = 1;

#ifndef __SYNTHESIS__
const uint32_t TIMER_NUM_CLOCKS = 20;
#else
const uint32_t PERIOD_NS = 4;
const uint32_t TIMER_PERIOD_NS = 1e8; // 0.1s in ns
const uint32_t TIMER_NUM_CLOCKS = TIMER_PERIOD_NS / PERIOD_NS;
#endif

/**
 * @defgroup igmp IGMP Server
 * @{
 */

enum opCodes {
    IP_NOOP = 0,
    IP_ADD = 1,
    IP_DELETE = 2,
    IP_READ = 3,
};

typedef ap_uint<32> ap_uint32_t;
typedef ap_uint<16> ap_uint16_t;
typedef ap_uint<1> ap_uint1_t;

struct igmpQuery {
    uint32_t maxRespTime;
    ap_uint32_t groupAddr;
    igmpQuery() {}
    igmpQuery(uint32_t groupAddr) : maxRespTime(0), groupAddr(groupAddr) {}
};

struct igmpTimerEntry {
    uint32_t time;
    uint32_t groupAddr;
    bool fActive;
};

struct igmpIpMeta {
    uint32_t dstAddr;
    uint16_t length;
    bool routerAlertOption;
};

struct igmpReportMeta {
    uint8_t version;
    bool is_state_record;
    ap_uint32_t ip_address;
    ap_uint32_t group_address;
    uint8_t mode;
    uint8_t nEntries;
    igmpReportMeta() {}
    igmpReportMeta(uint8_t version, bool is_state_record, ap_uint32_t ip_address, ap_uint32_t group_address, 
        uint8_t mode, uint8_t nEntries) : 
        version(version), is_state_record(is_state_record), ip_address(ip_address), group_address(group_address),
        mode(mode), nEntries(nEntries) {}
};


/** }@ */

/** @defgroup igmp_report_gen IGMP Report Generator
 * @ingroup igmp
 */
void igmp_report_gen(hls::stream<ap_axiu<64,0,0,0> >& m_axis_data,
                     hls::stream<igmpQuery>& gen_report,
                     hls::stream<bool>& is_v3,
                     bool host_op_toggle,
                     ap_uint<2> host_opcode,
                     uint32_t host_ipAddr,
                     uint32_t srcIpAddr,
                     uint8_t host_tableAddr,
                     uint8_t igmp_ver,
                     uint32_t& readVal,
                     bool enable);

void igmp_parser(hls::stream<ap_axiu<64,0,0,0> >& inData,
                 hls::stream<igmpQuery>& outIgmpQuery,
                 uint32_t& stats_invalid_csum,
                 uint32_t& stats_igmp_query,
                 uint32_t& stats_invalid_query);

void igmp_100ms_count(hls::stream<bool>& tick100ms);

void igmp_process_timer(hls::stream<bool>& tick100ms,
                        hls::stream<bool>& is_v3,
                        hls::stream<igmpQuery>& inIgmpQuery,
                        hls::stream<igmpQuery>& outIgmpReport);

void igmp(hls::stream<ap_axiu<64,0,0,0> >& s_axis_data,
          hls::stream<ap_axiu<64,0,0,0> >& m_axis_data,
          bool host_op_toggle,
          ap_uint<2> host_opcode,
          uint32_t host_ipAddr,
          uint32_t srcIpAddr,
          uint8_t host_tableAddr,
          uint32_t& readVal,
          bool enable,
          uint32_t &invalid_csum,
          uint32_t &igmp_query,
          uint32_t &invalid_query,
          uint8_t igmp_ver);

/** @ingroup igmp
 */
struct twoTuple {
    ap_uint<16> nLength;
    ap_uint<32> dstIp;
    twoTuple() {}
    twoTuple(ap_uint<16> nLength, ap_uint<32> dstIp) : nLength(nLength), dstIp(dstIp) {}
};

#endif
