/*
 * Copyright 2021 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include "linehandler.hpp"

void LineFilter::portFilter(ap_uint<32> regControl,
                            ap_uint<32> regEchoAddress,
                            ap_uint<32> regEchoPort,
                            ap_uint<32> regFilterAddress[NUM_FILTERS],
                            ap_uint<32> regFilterPort[NUM_FILTERS],
                            ap_uint<32> regFilterSplitIdx[NUM_FILTERS],
                            ap_uint<32> &regRxWord,
                            ap_uint<32> &regRxMeta,
                            ap_uint<32> &regDropWord,
                            ap_uint<32> &regDebugAddress,
                            ap_uint<32> &regDebugPort,
                            hls::stream<axiWordExt_t> &inputStream,
                            hls::stream<ipUdpMetaPackExt_t> &inputMetaStream,
                            hls::stream<axiWordExt_t> &echoStream,
                            hls::stream<ipUdpMetaPackExt_t> &echoMetaStream,
                            hls::stream<axiWord_t> &outputStream,
                            hls::stream<lhSplitId_t> &portIdStream)
{
#pragma HLS pipeline II=1 style=flp

    mmInterface intf;
    ipUdpMetaPackExt_t currMetaPackExt, echoMetaPackExt;
    axiWordExt_t currDataExt;
    ipUdpMeta_t currMeta, echoMeta;
    ipUdpMetaPack_t currMetaPack, echoMetaPack;
    axiWord_t currData;

    ap_uint<NUM_FILTERS> filterChk=0;

    switch (iid_state)
    {
        case GET_VALID:
            if(!inputMetaStream.empty())
            {
                inputMetaStream.read(currMetaPackExt);
                currMetaPack.data = currMetaPackExt.data;
                currMetaPack.keep = currMetaPackExt.keep;
                currMetaPack.last = currMetaPackExt.last;
                intf.udpMetaUnpack(&currMetaPack, &currMeta);
                ++countRxMeta;

                regDebugAddress = currMeta.srcAddress;
                regDebugPort = 0x0000ffff & currMeta.srcPort;

                if(LH_FILTER_DISABLE & regControl)
                {
                    // use splitId = 0 by default
                    portIdStream.write(0);
                    iid_state = FWD;
                }
                else
                {
                    // parallel lookup of configured filters for address and port match
                    for(unsigned i=0; i<NUM_FILTERS; ++i)
                    {
#pragma HLS UNROLL
                        filterChk[(NUM_FILTERS-1)-i] =
                            ((currMeta.srcAddress == regFilterAddress[i]) && (currMeta.srcPort == regFilterPort[i]));
                    }

                    if(filterChk != 0)
                    {
                        // filterChk register contains a list of all the filters that match the IP addr & port in a way
                        // that a 1 in position n means that the n-th filter contains a match. To figure out to which
                        // split id to associate with this packet we perform a leading one detect e.g:
                        //     filterChk = '0001000' (4th filter from the MSB is valid)
                        //     filterChk.countLeadingZeros() == 3
                        const lhSplitId_t chkIdx = lhSplitId_t(filterChk.countLeadingZeros());
                        portIdStream.write(regFilterSplitIdx[chkIdx]);
                        iid_state = FWD;
                    }
                    else
                    {
                        iid_state = DROP;
                    }
                }

                // debug functionality to send all ingress UDP packets back to the tx stream
                if(LH_ECHO_ENABLE & regControl)
                {
                    echoMeta.srcAddress = regEchoAddress;
                    echoMeta.srcPort = regEchoPort;
                    intf.udpMetaPack(&echoMeta, &echoMetaPack);

                    echoMetaPackExt.data = echoMetaPack.data;
                    echoMetaPackExt.keep = -1;
                    echoMetaPackExt.last = true;

                    echoMetaStream.write(echoMetaPackExt);
                }
            }
            break;

        case FWD:
            if(!inputStream.empty() && !outputStream.full())
            {
                inputStream.read(currDataExt);
                currData.data = currDataExt.data;
                currData.keep = currDataExt.keep;
                currData.last = currDataExt.last;
                ++countRxWord;
                outputStream.write(currData);

                if(currData.last)
                {
                    iid_state = GET_VALID;
                }

                if(LH_ECHO_ENABLE & regControl)
                {
                    echoStream.write(currDataExt);
                }
            }
            break;

        case DROP:
            if(!inputStream.empty())
            {
                inputStream.read(currDataExt);
                ++countDropWord;

                if(currDataExt.last)
                {
                    iid_state = GET_VALID;
                }

                if(LH_ECHO_ENABLE & regControl)
                {
                    echoStream.write(currDataExt);
                }
            }
            break;
    }

    regRxWord   = countRxWord;
    regRxMeta   = countRxMeta;
    regDropWord = countDropWord;

    return;
}

void LineHandler::lineArbitrator(ap_uint<32> regControlArb,
                                 ap_uint<32> regResetTimerInterval,
                                 ap_uint<32> &regTotalSent,
                                 ap_uint<32> &regTotalWordSent,
                                 ap_uint<32> &regTotalMissed,
                                 ap_uint<32> &regRxFeed0,
                                 ap_uint<32> &regRxFeed1,
                                 ap_uint<32> &regTxFeed0,
                                 ap_uint<32> &regTxFeed1,
                                 ap_uint<32> &regDiscarded0,
                                 ap_uint<32> &regDiscarded1,
                                 hls::stream<axiWord_t> &port0Strm,
                                 hls::stream<axiWord_t> &port1Strm,
                                 hls::stream<lhSplitId_t> &splitIdStrm0,
                                 hls::stream<lhSplitId_t> &splitIdStrm1,
                                 hls::stream<axiWordExt_t> &outputFeed)
{
#pragma HLS PIPELINE II=1 style=flp

    enum iid_StateType
    {
        ARB_FETCH,
        ARB_DECODE,
        ARB_FORWARD_0,
        ARB_FORWARD_1,
        ARB_DROP_0,
        ARB_DROP_1
    };
    static iid_StateType iid_state = ARB_FETCH;

    static axiWord_t wordIn;
    static axiWordExt_t wordOut;
    static ap_uint<32> resetTimerCounter=0;
    static ap_uint<1> activePort=1;
    static lhSplitId_t splitId=0;
    static seqNum_t seqNumReceived=0;
    static seqNum_t seqNumExpected[NUM_SPLITS] = {0};
    static bool arbForward=false;

    static ap_uint<32> countTotalSent=0;
    static ap_uint<32> countTotalWordSent=0;
    static ap_uint<32> countTotalMissed=0;
    static ap_uint<32> countRxFeed0=0;
    static ap_uint<32> countRxFeed1=0;
    static ap_uint<32> countTxFeed0=0;
    static ap_uint<32> countTxFeed1=0;
    static ap_uint<32> countDiscarded0=0;
    static ap_uint<32> countDiscarded1=0;

#pragma HLS ARRAY_PARTITION variable=seqNumExpected complete

    // update sequence reset timer, used to ignore reset on second line
    // TODO: this is a global timer, there should be one per split
    if(resetTimerCounter)
    {
        --resetTimerCounter;
    }

    // support sequence number reset across all splits via control register
    if(LH_RESET_SEQ_NUM & regControlArb)
    {
        for(unsigned i=0; i<NUM_SPLITS; ++i)
        {
            seqNumExpected[i] = 0;
        }
    }

    // main state machine for line arbitration
    switch(iid_state)
    {
        case ARB_FETCH:
        {
            // wait for valid SOP, try to service ports in round robin manner by
            // checking opposite port from which last packet was processed
            // TODO: better way to do round robin without code duplication?
            if(1 == activePort)
            {
                if(!port0Strm.empty())
                {
                    activePort = 0;
                    wordIn = port0Strm.read();
                    splitId = splitIdStrm0.read();
                    seqNumReceived = wordIn.data(SEQ_NUM_WIDTH-1, 0);
                    iid_state = ARB_DECODE;
                }
                else if(!port1Strm.empty())
                {
                    activePort = 1;
                    wordIn = port1Strm.read();
                    splitId = splitIdStrm1.read();
                    seqNumReceived = wordIn.data(SEQ_NUM_WIDTH-1, 0);
                    iid_state = ARB_DECODE;
                }
            }
            else
            {
                if(!port1Strm.empty())
                {
                    activePort = 1;
                    wordIn = port1Strm.read();
                    splitId = splitIdStrm1.read();
                    seqNumReceived = wordIn.data(SEQ_NUM_WIDTH-1, 0);
                    iid_state = ARB_DECODE;
                }
                else if(!port0Strm.empty())
                {
                    activePort = 0;
                    wordIn = port0Strm.read();
                    splitId = splitIdStrm0.read();
                    seqNumReceived = wordIn.data(SEQ_NUM_WIDTH-1, 0);
                    iid_state = ARB_DECODE;
                }
            }

            break;
        }
        case ARB_DECODE:
        {
            // process sequence number to make arbitration decision
            if(0 == seqNumReceived)
            {
                // implied sequence number reset
                if(resetTimerCounter)
                {
                   // previous reset seen within configurable interval
                   LH_DBG("RST[" << activePort << "]: Discarding " << seqNumReceived);
                   arbForward = false;
                }
                else
                {
                    // new reset, load timer with configurable interval
                    LH_DBG("RST[" << activePort << "]: Forwarding " << seqNumReceived);
                    seqNumExpected[splitId] = seqNumReceived + 1;
                    resetTimerCounter = regResetTimerInterval;
                    arbForward = true;
                }
            }
            else if(seqNumReceived < seqNumExpected[splitId])
            {
                // sequence number lower than expected, drop as duplicate
                LH_DBG("DUP[" << activePort << "]: Expected " << seqNumExpected[splitId] << ", Discarding " << seqNumReceived);
                arbForward = false;
            }
            else if(seqNumReceived == seqNumExpected[splitId])
            {
                // sequence number matches expected, forward downstream
                LH_DBG("FWD[" << activePort << "]: Forwarding " << seqNumReceived);
                seqNumExpected[splitId] = seqNumReceived + 1;
                arbForward = true;
            }
            else
            {
                // sequence number higher than expected, mark as gap and forward
                // downstream as long as reset timer is not active
                if(resetTimerCounter)
                {
                    LH_DBG("DUP[" << activePort << "]: Expected " << seqNumExpected[splitId] << ", Discarding " << seqNumReceived);
                    arbForward = false;
                }
                else
                {
                    LH_DBG("GAP[" << activePort << "]: Expected " << seqNumExpected[splitId] << ", Forwarding " << seqNumReceived);
                    seqNumExpected[splitId] = seqNumReceived + 1;
                    arbForward = true;
                    ++countTotalMissed;
                }
            }

            // one state transition per packet, can update RX counters here
            if(0 == activePort)
            {
                ++countRxFeed0;
            }
            else
            {
                ++countRxFeed1;
            }

            // forward or drop the first packet frame read in ARB_FETCH
            if(arbForward)
            {
                wordOut.data = wordIn.data;
                wordOut.keep = wordIn.keep;
                wordOut.last = wordIn.last;
                outputFeed.write(wordOut);
                ++countTotalWordSent;
                ++countTotalSent;

                // single 'ARB_FORWARD' state could be used here to reduce code
                // duplication but splitting reduces nested conditional checks
                // which reduces logic levels on counter logic which helps to
                // improve timing
                if(0 == activePort)
                {
                    iid_state = ARB_FORWARD_0;
                }
                else
                {
                    iid_state = ARB_FORWARD_1;
                }
            }
            else
            {
                // single 'ARB_DROP' state could be used here to reduce code
                // duplication but splitting reduces nested conditional checks
                // which reduces logic levels on counter logic which helps to
                // improve timing
                if(0 == activePort)
                {
                    iid_state = ARB_DROP_0;

                }
                else
                {
                    iid_state = ARB_DROP_1;
                }
            }

            break;
        }
        case ARB_FORWARD_0:
        {
            // read and forward all frames from physical port 0 up until EOP
            if(!port0Strm.empty())
            {
                wordIn = port0Strm.read();
                wordOut.data = wordIn.data;
                wordOut.keep = wordIn.keep;
                wordOut.last = wordIn.last;
                outputFeed.write(wordOut);
                ++countTotalWordSent;
            }

            if(wordIn.last)
            {
                iid_state = ARB_FETCH;
                ++countTxFeed0;
                ++countTotalSent;
            }

            break;
        }
        case ARB_FORWARD_1:
        {
            // read and forward all frames from physical port 1 up until EOP
            if(!port1Strm.empty())
            {
                wordIn = port1Strm.read();
                wordOut.data = wordIn.data;
                wordOut.keep = wordIn.keep;
                wordOut.last = wordIn.last;
                outputFeed.write(wordOut);
                ++countTotalWordSent;
            }

            if(wordIn.last)
            {
                iid_state = ARB_FETCH;
                ++countTxFeed1;
                ++countTotalSent;
            }

            break;
        }
        case ARB_DROP_0:
        {
            // read and drop all frames from physical port 0 up until EOP
            if(!port0Strm.empty())
            {
                wordIn = port0Strm.read();
            }

            if(wordIn.last)
            {
                iid_state = ARB_FETCH;
                ++countDiscarded0;
            }

            break;
        }
        case ARB_DROP_1:
        {
            // read and drop all frames from physical port 1 up until EOP
            if(!port1Strm.empty())
            {
                wordIn = port1Strm.read();
            }

            if(wordIn.last)
            {
                iid_state = ARB_FETCH;
                ++countDiscarded1;
            }

            break;
        }
    }

    // assign to register map
    regTotalSent     = countTotalSent;
    regTotalWordSent = countTotalWordSent;
    regTotalMissed   = countTotalMissed;
    regRxFeed0       = countRxFeed0;
    regRxFeed1       = countRxFeed1;
    regTxFeed0       = countTxFeed0;
    regTxFeed1       = countTxFeed1;
    regDiscarded0    = countDiscarded0;
    regDiscarded1    = countDiscarded1;
}

void LineHandler::eventHandler(ap_uint<32> &regRxEvent,
                               hls::stream<clockTickGeneratorEvent_t> &eventStream)
{
#pragma HLS PIPELINE II=1 style=flp

    clockTickGeneratorEvent_t tickEvent;

    static ap_uint<32> countRxEvent=0;

    if(!eventStream.empty())
    {
        tickEvent = eventStream.read();
        ++countRxEvent;

        // event notification has been received from programmable clock tick
        // generator, handling currently limited to incrementing a counter,
        // placeholder for user to extend with custom event handling code
    }

    regRxEvent = countRxEvent;

    return;
}
