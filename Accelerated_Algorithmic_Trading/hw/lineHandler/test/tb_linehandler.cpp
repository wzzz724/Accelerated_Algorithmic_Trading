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

#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "linehandler_kernels.hpp"

#define PACKET_LENGTH (64)

#define IP_ADDR_SP0_0 (0xcdd1d44b)
#define PORT_SP0_0    (0x8000)
#define IP_ADDR_SP1_0 (0xcdd1d44d)
#define PORT_SP1_0    (0x9000)

#define IP_ADDR_SP0_1 (0xcdd1d44c)
#define PORT_SP0_1    (0x8001)
#define IP_ADDR_SP1_1 (0xcdd1d44e)
#define PORT_SP1_1    (0x9001)

hls::stream<axiWordExt_t> inputDataStrm0;
hls::stream<axiWordExt_t> outputDataStrm0;
hls::stream<axiWordExt_t> inputDataStrm1;
hls::stream<axiWordExt_t> outputDataStrm1;
hls::stream<axiWordExt_t> arbDataStrm;
hls::stream<ipUdpMetaPackExt_t> inputMetaStrm0;
hls::stream<ipUdpMetaPackExt_t> outputMetaStrm0;
hls::stream<ipUdpMetaPackExt_t> inputMetaStrm1;
hls::stream<ipUdpMetaPackExt_t> outputMetaStrm1;
hls::stream<clockTickGeneratorEvent_t> eventStrm;

static void preparePacket(hls::stream<axiWordExt_t> &data,
                          hls::stream<ipUdpMetaPackExt_t> &meta,
                          uint32_t seq,
                          uint32_t ipAddr,
                          uint32_t port)
{
    const static unsigned pLen=(PACKET_LENGTH/8);

    axiWord_t axiw;
    ipUdpMeta_t metaw;
    ipUdpMetaPack_t metapackw;
    mmInterface intf;

    metaw.srcAddress = ipAddr;
    metaw.srcPort = port;
    intf.udpMetaPack(&metaw, &metapackw);
    meta.write(metapackw);

    axiw.keep = -1;
    axiw.last = false;
    axiw.data = ap_uint<32>(seq);
    std::cout << std::endl << axiw.data;

    data.write(axiw);
    for(unsigned i=1; i<pLen; ++i)
    {
        axiw.data = rand();
        axiw.last = (i == (pLen-1));
        data.write(axiw);
        std::cout << " " << axiw.data;
    }
}

static std::vector<uint32_t> gatherArbitratedSeqs(hls::stream<axiWordExt_t> &strm)
{
    axiWord_t axiw;
    std::vector<uint32_t> ret;
    bool isFirst=true;

    while(!strm.empty())
    {
        axiw = strm.read();
        if(isFirst)
        {
            uint32_t seqNum = axiw.data(31, 0);
            ret.emplace_back(seqNum);
            std::cout << std::endl << axiw.data;
        }
        else
        {
            std::cout << " " << axiw.data;
        }

        isFirst = axiw.last;
    }

    return ret;
}

static void prepareTestCase(std::vector<uint32_t> seq0,
                            std::vector<uint32_t> seq1,
                            uint32_t ip0,
                            uint32_t ip1,
                            uint32_t port0,
                            uint32_t port1)
{
    assert(seq0.size() == seq1.size());

    for(unsigned i=0; i<seq0.size(); ++i)
    {
        preparePacket(inputDataStrm0, inputMetaStrm0, seq0[i], ip0, port0);
        preparePacket(inputDataStrm1, inputMetaStrm1, seq1[i], ip1, port1);
    }
}

int main()
{
    lineHandlerRegControl_t regControl = {0};
    lineHandlerRegStatus_t regStatus = {0};
    regPortFilterContainer_t regPortFilter;

    std::cout << "LineHandler Test" << std::endl;
    std::cout << "----------------" << std::endl;

    // set control registers for port filter
    regControl.controlPort0 = 0;
    regControl.controlPort1 = 0;

    memset(&regPortFilter, 0, sizeof(regPortFilter));

    regPortFilter.filterAddress0[0] = IP_ADDR_SP0_0;
    regPortFilter.filterPort0[0] = PORT_SP0_0;
    regPortFilter.filterSplitId0[0] = 0;
    regPortFilter.filterAddress0[1] = IP_ADDR_SP1_0;
    regPortFilter.filterPort0[1] = PORT_SP1_0;
    regPortFilter.filterSplitId0[1] = 1;

    regPortFilter.filterAddress1[0] = IP_ADDR_SP0_1;
    regPortFilter.filterPort1[0] = PORT_SP0_1;
    regPortFilter.filterSplitId1[0] = 0;
    regPortFilter.filterAddress1[1] = IP_ADDR_SP1_1;
    regPortFilter.filterPort1[1] = PORT_SP1_1;
    regPortFilter.filterSplitId1[1] = 1;

    // set control registers for arbitration logic
    regControl.controlArb = 0;
    regControl.resetTimerInterval = 32; // clock cycles

    // prepare dummy sequence of packets
    std::cout << "Preparing test packets ...";
    prepareTestCase({42, 45, 44, 45, 46, 48, 48, 0, 1},
                    {42, 43, 44, 46, 47, 47, 50, 51, 0},
                    IP_ADDR_SP0_0,
                    IP_ADDR_SP0_1,
                    PORT_SP0_0,
                    PORT_SP0_1);

    prepareTestCase({42, 45, 44, 45, 46, 0, 1},
                    {42, 43, 44, 46, 47, 48, 0},
                    IP_ADDR_SP1_0,
                    IP_ADDR_SP1_1,
                    PORT_SP1_0,
                    PORT_SP1_1);

    std::cout << std::endl << "Invoking kernel ..." << std::endl;
    while(!inputDataStrm0.empty() || !inputDataStrm1.empty())
    {
        lineHandlerTop(regControl,
                       regStatus,
                       regPortFilter,
                       inputDataStrm0,
                       inputMetaStrm0,
                       outputDataStrm0,
                       outputMetaStrm0,
                       inputDataStrm1,
                       inputMetaStrm1,
                       outputDataStrm1,
                       outputMetaStrm1,
                       arbDataStrm,
                       eventStrm);
    }

    // dummy drain
    for(unsigned i=0; i<164; ++i)
    {
        lineHandlerTop(regControl,
                       regStatus,
                       regPortFilter,
                       inputDataStrm0,
                       inputMetaStrm0,
                       outputDataStrm0,
                       outputMetaStrm0,
                       inputDataStrm1,
                       inputMetaStrm1,
                       outputDataStrm1,
                       outputMetaStrm1,
                       arbDataStrm,
                       eventStrm);
    }

    // drain and print sequence messages
    std::cout << "Examining output packets ..." << std::endl;
    auto arbSeqs = gatherArbitratedSeqs(arbDataStrm);

    std::cout << std::endl << "Arbitrated feed seq numbers: ";
    for (auto &seq : arbSeqs)
    {
        std::cout << seq << ", ";
    }
    std::cout << std::endl;

    std::cout << "--" << std::endl;
    std::cout << "REG STATUS" << std::endl;
    std::cout << "rxWord0="       << regStatus.rxWord0       << std::endl;
    std::cout << "rxMeta0="       << regStatus.rxMeta0       << std::endl;
    std::cout << "dropWord0="     << regStatus.dropWord0     << std::endl;
    std::cout << "debugAddress0=" << regStatus.debugAddress0 << std::endl;
    std::cout << "debugPort0="    << regStatus.debugPort0    << std::endl;
    std::cout << "rxWord1="       << regStatus.rxWord1       << std::endl;
    std::cout << "rxMeta1="       << regStatus.rxMeta1       << std::endl;
    std::cout << "dropWord1="     << regStatus.dropWord1     << std::endl;
    std::cout << "debugAddress1=" << regStatus.debugAddress1 << std::endl;
    std::cout << "debugPort1="    << regStatus.debugPort1    << std::endl;
    std::cout << "totalSent="     << regStatus.totalSent     << std::endl;
    std::cout << "totalMissed="   << regStatus.totalMissed   << std::endl;
    std::cout << "rxFeed0="       << regStatus.rxFeed0       << std::endl;
    std::cout << "rxFeed1="       << regStatus.rxFeed1       << std::endl;
    std::cout << "txFeed0="       << regStatus.txFeed0       << std::endl;
    std::cout << "txFeed1="       << regStatus.txFeed1       << std::endl;
    std::cout << "discarded0="    << regStatus.discarded0    << std::endl;
    std::cout << "discarded1="    << regStatus.discarded1    << std::endl;
    std::cout << std::endl;

    // check received packets are in expected order
    const std::vector<unsigned> expectedPackets = {42, 45, 46, 47, 48, 50, 0, 1, 42, 45, 46, 47, 0, 1};
    int error = 0;
    if(arbSeqs.size() != expectedPackets.size())
    {
        std::cerr << "ERROR: Expected number of output packets mismatch: " << arbSeqs.size() << " != " << expectedPackets.size() << std::endl;
        error = 1;
    }
    else
    {
        for(unsigned i=0; i<expectedPackets.size(); ++i)
        {
            if(arbSeqs[i] != expectedPackets[i])
            {
                std::cerr << "ERROR: Expected seq " << expectedPackets[i] << " but got " << arbSeqs[i] << std::endl;
                error = 1;
            }
        }
    }

    if(error)
    {
        std::cout << "FAILURE!" << std::endl;
    }
    else
    {
        std::cout << "SUCCESS!" << std::endl;
    }

    return error;
};
