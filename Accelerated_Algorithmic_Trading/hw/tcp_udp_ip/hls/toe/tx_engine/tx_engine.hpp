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
#include "../toe.hpp"
#include "../toe_internals.hpp"

/** @ingroup tx_engine
 *
 */
struct tx_engine_meta // same as rxEngine
{
    ap_uint<32> seqNumb;
    ap_uint<32> ackNumb;
    ap_uint<16> window_size;
#if (WINDOW_SCALE)
    ap_uint<4> win_shift;
#endif
    ap_uint<16> length;
    ap_uint<1> ack;
    ap_uint<1> rst;
    ap_uint<1> syn;
    ap_uint<1> fin;
    tx_engine_meta() {}
    tx_engine_meta(ap_uint<1> ack, ap_uint<1> rst, ap_uint<1> syn, ap_uint<1> fin)
        : seqNumb(0), ackNumb(0), window_size(0), length(0), ack(ack), rst(rst), syn(syn), fin(fin) {}
    tx_engine_meta(
        ap_uint<32> seqNumb, ap_uint<32> ackNumb, ap_uint<1> ack, ap_uint<1> rst, ap_uint<1> syn, ap_uint<1> fin)
        : seqNumb(seqNumb), ackNumb(ackNumb), window_size(0), length(0), ack(ack), rst(rst), syn(syn), fin(fin) {}
};

/** @ingroup tx_engine
 * All fields are stored in network order - Big Endian.
 */
struct twoTuple {
    ap_uint<32> srcIp;
    ap_uint<32> dstIp;
    twoTuple() {}
    twoTuple(ap_uint<32> srcIp, ap_uint<32> dstIp) : srcIp(srcIp), dstIp(dstIp) {}
};

/** @defgroup tx_engine TX Engine
 *  @ingroup tcp_module
 *  @image html tx_engine.png
 *  Explain the TX Engine
 *  The @ref tx_engine contains a state machine with a state for each Event Type.
 *  It then loads and generates the necessary metadata to construct the packet. If the packet
 *  contains any payload the data is retrieved from the Memory and put into the packet. The
 *  complete packet is then streamed out of the @ref tx_engine.
 */
template <int WIDTH>
void tx_engine(hls::stream<extendedEvent>& eventEng2txEng_event,
               hls::stream<rxSarReply>& rxSar2txEng_upd_rsp,
               hls::stream<txTxSarReply>& txSar2txEng_upd_rsp,
               hls::stream<ap_axiu<WIDTH,0,0,0> >& txBufferReadData,
#if (TCP_NODELAY)
               hls::stream<net_axis<WIDTH> >& txApp2txEng_data_stream,
#endif
               hls::stream<fourTuple>& sLookup2txEng_rev_rsp,
               hls::stream<ap_uint<16> >& txEng2rxSar_upd_req,
               hls::stream<txTxSarQuery>& txEng2txSar_upd_req,
               hls::stream<txRetransmitTimerSet>& txEng2timer_setRetransmitTimer,
               hls::stream<ap_uint<16> >& txEng2timer_setProbeTimer,
               hls::stream<ap_axiu<128,0,0,0> >& txBufferReadCmd,
               hls::stream<ap_uint<16> >& txEng2sLookup_rev_req,
               hls::stream<ap_axiu<WIDTH,0,0,0> >& ipTxData,
               hls::stream<ap_uint<1> >& readCountFifo,
               uint32_t& stats_tcpOutSegs,
               uint32_t& stats_tcpRetransSegs);
