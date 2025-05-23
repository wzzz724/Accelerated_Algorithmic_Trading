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


#include "xlnx_network_capture_pcap_headers.h"
#include "xlnx_network_capture_pcap_writer.h"
#include "xlnx_network_capture_error_codes.h"

using namespace XLNX;


PCAPWriter::PCAPWriter()
{
    m_pFile = nullptr;
}



PCAPWriter::~PCAPWriter()
{
    Stop();
}



uint32_t PCAPWriter::Start(char* filePath)
{
    uint32_t retval = XLNX_OK;
   

    m_pFile = fopen(filePath, "wb");

    if (m_pFile != nullptr)
    {
        retval = WriteFileHeader();
    }
    else
    {
        retval = XLNX_NETWORK_CAPTURE_ERROR_FAILED_TO_OPEN_PCAP_FILE_FOR_WRITING;
    }
   


    return retval;
}




uint32_t PCAPWriter::Stop()
{
    uint32_t retval = XLNX_OK;

    if (m_pFile != nullptr)
    {
        fflush(m_pFile);
        fclose(m_pFile);
        m_pFile = nullptr;
    }


    return retval;
}





uint32_t PCAPWriter::CheckFileIsOpen(void)
{
    uint32_t retval = XLNX_OK;

    if (m_pFile == nullptr)
    {
        retval = XLNX_NETWORK_CAPTURE_ERROR_PCAP_FILE_NOT_OPEN;
    }

    return retval;
}




uint32_t PCAPWriter::WriteFileHeader(void)
{
    uint32_t retval = XLNX_OK;
    pcap_hdr_t fileHeader;


    retval = CheckFileIsOpen();

    if (retval == XLNX_OK)
    {
        fileHeader.magic_number     = 0xA1B23C4D;   //nanosecond variant
        fileHeader.version_major    = 2;
        fileHeader.version_minor    = 4;
        fileHeader.thiszone         = 0;
        fileHeader.sigfigs          = 0;
        fileHeader.snaplen          = MAX_PACKET_LENGTH;
        fileHeader.network          = LINKTYPE_ETHERNET;

        fwrite(&fileHeader, sizeof(fileHeader), 1, m_pFile);
        fflush(m_pFile);
    }
   

    return retval;
    
}




uint32_t PCAPWriter::WritePacket(uint8_t* pPacketData, uint32_t dataLength, uint64_t timestampNanoseconds)
{
    uint32_t retval = XLNX_OK;
    pcaprec_hdr_t packetHeader;
    
    uint64_t seconds;
    uint64_t nanoseconds;

    retval = CheckFileIsOpen();

    if (retval == XLNX_OK)
    {
        
        seconds     = timestampNanoseconds / 1000000000;
        nanoseconds = timestampNanoseconds - (((uint64_t)seconds) * 1000000000);

        packetHeader.ts_sec     = (uint32_t)seconds;
        packetHeader.ts_nsec    = (uint32_t)nanoseconds;
        packetHeader.orig_len   = dataLength;

        if (dataLength > MAX_PACKET_LENGTH)
        {
            packetHeader.incl_len = MAX_PACKET_LENGTH;
        }
        else
        {
            packetHeader.incl_len = dataLength;
        }
   
        //write the packet header...
        fwrite(&packetHeader, sizeof(packetHeader), 1, m_pFile);

        //...and then the data...
        fwrite(pPacketData, 1, packetHeader.incl_len, m_pFile);

        fflush(m_pFile);
    }


    return retval;
}

