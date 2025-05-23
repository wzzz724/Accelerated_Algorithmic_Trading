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
using namespace XLNX;


uint32_t TCPUDPIP::SetICMPEnabled(bool bEnabled)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;
    uint32_t mask;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckConfigurationIsAllowed();
    }
    
    if (retval == XLNX_OK)
    {
        offset = XLNX_TCP_UDP_IP_ICMP_ENABLE_REG_OFFSET;
        mask = 0x00000001;

        if (bEnabled)
        {
            value = 0x00000001;
        }
        else
        {
            value = 0x00000000;
        }

        retval = WriteRegWithMask32(offset, value, mask);
    }

    
  
    return retval;
}









uint32_t TCPUDPIP::GetICMPEnabled(bool* pbEnabled)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;
    uint32_t mask;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        offset = XLNX_TCP_UDP_IP_ICMP_ENABLE_REG_OFFSET;
        mask = 0x00000001;


        retval = ReadReg32(offset, &value);

        if (retval == XLNX_OK)
        {
            if ((value & mask) != 0)
            {
                *pbEnabled = true;
            }
            else
            {
                *pbEnabled = false;
            }
        }
    }
    

    return retval;
}

