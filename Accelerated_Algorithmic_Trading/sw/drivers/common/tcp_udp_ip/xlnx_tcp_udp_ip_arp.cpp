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

#include "xlnx_tcp_udp_ip.h"
#include "xlnx_tcp_udp_ip_address_map.h"
#include "xlnx_tcp_udp_ip_error_codes.h"

using namespace XLNX;

//--------------
// ARP OpCodes
//--------------
static const uint32_t ARP_OPCODE_ADD    = 0x01;
static const uint32_t ARP_OPCODE_DELETE = 0x02;
static const uint32_t ARP_OPCODE_READ   = 0x03;




uint32_t TCPUDPIP::ReadARPEntry(uint32_t index, ARPEntry* pARPEntry, bool* pbEntryValid)
{
    uint32_t retval = XLNX_OK;
    uint32_t ipAddrRegValue;
    uint32_t macAddrUpperRegValue;
    uint32_t macAddrLowerRegValue;
    uint32_t entryValidRegValue;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        if (index >= NUM_ARP_ENTRIES_SUPPORTED)
        {
            retval = XLNX_TCP_UDP_IP_ERROR_ARP_ENTRY_INDEX_OUT_OF_RANGE;
        }
    }



    //Set the bin index we want to read from...
    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_TCP_UDP_IP_ARP_ENTRY_BIN_REG_OFFSET, index);
    }


    //Set our operation to READ...
    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_TCP_UDP_IP_ARP_OPCODE_REG_OFFSET, ARP_OPCODE_READ);
    }



    //Toggle the "go" bit to get HW to populate the registers with values...
    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_TCP_UDP_IP_ARP_OPERATION_TOGGLE_REG_OFFSET, 0x01);
    }


    //..and clear it again, ready for next time...
    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_TCP_UDP_IP_ARP_OPERATION_TOGGLE_REG_OFFSET, 0x00);
    }



    ///////////
    // At this point the "IP Address", "MAC Address" and "Entry Valid" registers should have been be populated...
    ///////////




    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_TCP_UDP_IP_ARP_IP_ADDRESS_OUTPUT_REG_OFFSET, &ipAddrRegValue);
    }


    if (retval == XLNX_OK)
    {
        UnpackIPAddress(ipAddrRegValue, &pARPEntry->IPAddress.a, &pARPEntry->IPAddress.b, &pARPEntry->IPAddress.c, &pARPEntry->IPAddress.d);
    }





    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_TCP_UDP_IP_ARP_MAC_ADDRESS_OUTPUT_1_REG_OFFSET, &macAddrLowerRegValue);
    }


   
    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_TCP_UDP_IP_ARP_MAC_ADDRESS_OUTPUT_2_REG_OFFSET, &macAddrUpperRegValue);
    }


    if (retval == XLNX_OK)
    {
        UnpackMACAddress(macAddrUpperRegValue, 
                         macAddrLowerRegValue, 
                         &pARPEntry->MACAddress.a,
                         &pARPEntry->MACAddress.b,
                         &pARPEntry->MACAddress.c,
                         &pARPEntry->MACAddress.d,
                         &pARPEntry->MACAddress.e,
                         &pARPEntry->MACAddress.f);
    }






    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_TCP_UDP_IP_ARP_ENTRY_VALID_OUTPUT_REG_OFFSET, &entryValidRegValue);
    }

    if (retval == XLNX_OK)
    {
        if (entryValidRegValue != 0)
        {
            *pbEntryValid = true;
        }
        else
        {
            *pbEntryValid = false;
        }
    }



    return retval;
}








uint32_t TCPUDPIP::AddARPEntry(ARPEntry* pARPEntry)
{
    uint32_t retval = XLNX_OK;
    uint32_t ipAddrRegValue;
    uint32_t macAddrUpperRegValue;
    uint32_t macAddrLowerRegValue;



    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckConfigurationIsAllowed();
    }



    //When adding an entry, the HW will automatically choose the bin (index) to put it in.
    //It seems to use the bottom byte of the IP address as the index...




    //Set our operation to ADD...
    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_TCP_UDP_IP_ARP_OPCODE_REG_OFFSET, ARP_OPCODE_ADD);
    }





    if (retval == XLNX_OK)
    {
        PackIPAddress(pARPEntry->IPAddress.a,
                      pARPEntry->IPAddress.b,
                      pARPEntry->IPAddress.c,
                      pARPEntry->IPAddress.d,
                      &ipAddrRegValue);


        PackMACAddress(pARPEntry->MACAddress.a,
                       pARPEntry->MACAddress.b,
                       pARPEntry->MACAddress.c,
                       pARPEntry->MACAddress.d,
                       pARPEntry->MACAddress.e,
                       pARPEntry->MACAddress.f,
                       &macAddrUpperRegValue,
                       &macAddrLowerRegValue);

    }



   



    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_TCP_UDP_IP_ARP_IP_ADDRESS_INPUT_REG_OFFSET, ipAddrRegValue);
    }
   

    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_TCP_UDP_IP_ARP_MAC_ADDRESS_INPUT_1_REG_OFFSET, macAddrLowerRegValue);
    }



    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_TCP_UDP_IP_ARP_MAC_ADDRESS_INPUT_2_REG_OFFSET, macAddrUpperRegValue);
    }



    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_TCP_UDP_IP_ARP_ENTRY_VALID_INPUT_REG_OFFSET, 0x00000001);
    }



    //Toggle the "go" bit...
    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_TCP_UDP_IP_ARP_OPERATION_TOGGLE_REG_OFFSET, 0x01);
    }



    //..and clear it again, ready for next time...
    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_TCP_UDP_IP_ARP_OPERATION_TOGGLE_REG_OFFSET, 0x00);
    }
   

    return retval;
}








uint32_t TCPUDPIP::DeleteARPEntry(uint32_t index)
{
    uint32_t retval = XLNX_OK;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckConfigurationIsAllowed();
    }

    if (retval == XLNX_OK)
    {
        if (index >= NUM_ARP_ENTRIES_SUPPORTED)
        {
            retval = XLNX_TCP_UDP_IP_ERROR_ARP_ENTRY_INDEX_OUT_OF_RANGE;
        }
    }


    //Set the bin index we want to write to...
    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_TCP_UDP_IP_ARP_ENTRY_BIN_REG_OFFSET, index);
    }


    //Set our operation to DELETE...
    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_TCP_UDP_IP_ARP_OPCODE_REG_OFFSET, ARP_OPCODE_DELETE);
    }

    //Toggle the "go" bit...
    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_TCP_UDP_IP_ARP_OPERATION_TOGGLE_REG_OFFSET, 0x01);
    }


    //..and clear it again, ready for next time...
    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_TCP_UDP_IP_ARP_OPERATION_TOGGLE_REG_OFFSET, 0x00);
    }
   


    return retval;
}








uint32_t TCPUDPIP::DeleteAllARPEntries(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t i;


    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckConfigurationIsAllowed();
    }

    if (retval == XLNX_OK)
    {
        for (i = 0; i < NUM_ARP_ENTRIES_SUPPORTED; i++)
        {
            retval = DeleteARPEntry(i);

            if (retval != XLNX_OK)
            {
                break; //out of loop
            }
        }
    }

    return retval;
}


