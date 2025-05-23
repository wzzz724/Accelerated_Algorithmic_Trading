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


#ifndef XLNX_NETWORK_CAPTURE_PCAP_WRITER_H
#define XLNX_NETWORK_CAPTURE_PCAP_WRITER_H

#include <cstdio>
#include <cstdint>


namespace XLNX
{

class PCAPWriter
{

public:
    PCAPWriter();
    virtual ~PCAPWriter();



public:
    uint32_t Start(char* filePath);
    uint32_t Stop();



public:
    static const uint32_t MAX_PACKET_LENGTH = 65536;
   
    uint32_t WritePacket(uint8_t* pPacketData, uint32_t dataLength, uint64_t timestampNanoseconds);


protected:
    uint32_t CheckFileIsOpen(void);
    uint32_t WriteFileHeader(void);

protected:
    FILE* m_pFile;




};


}



#endif