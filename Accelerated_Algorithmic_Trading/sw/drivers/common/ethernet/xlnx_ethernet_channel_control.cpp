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

#include "xlnx_ethernet.h"
#include "xlnx_ethernet_address_map.h"
#include "xlnx_ethernet_error_codes.h"
#include "xlnx_ethernet_types.h"
using namespace XLNX;








uint32_t Ethernet::SetMACFilterAddress(uint32_t channel, uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = CheckChannelConfigurationAllowed(channel);
    }

    if (retval == XLNX_OK)
    {
        value = 0;
        value |= (c << 24);
        value |= (d << 16);
        value |= (e << 8);
        value |= (f << 0);

        retval = WriteReg32(XLNX_ETHERNET_CHANNEL_CONTROL_MAC_ADDRESS_LOWER_OFFSET(channel), value);
    }



    if (retval == XLNX_OK)
    {
        value = 0;
        value |= (a << 8);
        value |= (b << 0);

        retval = WriteReg32(XLNX_ETHERNET_CHANNEL_CONTROL_MAC_ADDRESS_UPPER_OFFSET(channel), value);
    }

    return retval;
}






uint32_t Ethernet::GetMACFilterAddress(uint32_t channel, uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d, uint8_t* e, uint8_t* f)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }


    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_CONTROL_MAC_ADDRESS_LOWER_OFFSET(channel), &value);
    }


    if (retval == XLNX_OK)
    {
        *c = ((value >> 24) & 0xFF);
        *d = ((value >> 16) & 0xFF);
        *e = ((value >> 8) & 0xFF);
        *f = ((value >> 0) & 0xFF);
    }

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_CONTROL_MAC_ADDRESS_UPPER_OFFSET(channel), &value);
    }


    if (retval == XLNX_OK)
    {
        *a = ((value >> 8) & 0xFF);
        *b = ((value >> 0) & 0xFF);
    }


    return retval;
}









uint32_t Ethernet::SetTxFIFOReset(uint32_t channel, bool bInReset)
{
    uint32_t retval = XLNX_OK;
    uint32_t shift = 16;
    uint32_t value;
    uint32_t mask = 0x01;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }


    if (retval == XLNX_OK)
    {
        if (bInReset)
        {
            value = 1;
        }
        else
        {
            value = 0;
        }

        value = value << shift;
        mask = mask << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_CHANNEL_CONTROL_TRAFFIC_PROC_RESET_OFFSET(channel), value, mask);
    }
   


    return retval;
}




uint32_t Ethernet::GetTxFIFOReset(uint32_t channel, bool* pbInReset)
{
    uint32_t retval = XLNX_OK;
    uint32_t shift = 16;
    uint32_t value;
    uint32_t mask = 0x01;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_CONTROL_TRAFFIC_PROC_RESET_OFFSET(channel), &value);
    }


    if (retval == XLNX_OK)
    {
        *pbInReset = (bool)((value >> shift) & mask);
    }

    
    return retval;
}









uint32_t Ethernet::SetRxFIFOReset(uint32_t channel, bool bInReset)
{
    uint32_t retval = XLNX_OK;
    uint32_t shift = 0;
    uint32_t value;
    uint32_t mask = 0x01;


    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }


    if (retval == XLNX_OK)
    {
        if (bInReset)
        {
            value = 1;
        }
        else
        {
            value = 0;
        }

        value = value << shift;
        mask = mask << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_CHANNEL_CONTROL_TRAFFIC_PROC_RESET_OFFSET(channel), value, mask);
    }


    return retval;
}





uint32_t Ethernet::GetRxFIFOReset(uint32_t channel, bool* pbInReset)
{
    uint32_t retval = XLNX_OK;
    uint32_t shift = 0;
    uint32_t value;
    uint32_t mask = 0x01;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_CONTROL_TRAFFIC_PROC_RESET_OFFSET(channel), &value);
    }


    if (retval == XLNX_OK)
    {
        *pbInReset = (bool)((value >> shift) & mask);
    }


    return retval;
}





uint32_t Ethernet::SetTxTrafficProcessorReset(uint32_t channel, bool bInReset)
{
    uint32_t retval = XLNX_OK;
    uint32_t shift = 17;
    uint32_t value;
    uint32_t mask = 0x01;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }


    if (retval == XLNX_OK)
    {
        if (bInReset)
        {
            value = 1;
        }
        else
        {
            value = 0;
        }

        value = value << shift;
        mask = mask << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_CHANNEL_CONTROL_TRAFFIC_PROC_RESET_OFFSET(channel), value, mask);
    }



    return retval;
}





uint32_t Ethernet::GetTxTrafficProcessorReset(uint32_t channel, bool* pbInReset)
{
    uint32_t retval = XLNX_OK;
    uint32_t shift = 17;
    uint32_t value;
    uint32_t mask = 0x01;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_CONTROL_TRAFFIC_PROC_RESET_OFFSET(channel), &value);
    }


    if (retval == XLNX_OK)
    {
        *pbInReset = (bool)((value >> shift)& mask);
    }


    return retval;
}





uint32_t Ethernet::SetRxTrafficProcessorReset(uint32_t channel, bool bInReset)
{
    uint32_t retval = XLNX_OK;
    uint32_t shift = 1;
    uint32_t value;
    uint32_t mask = 0x01;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }


    if (retval == XLNX_OK)
    {
        if (bInReset)
        {
            value = 1;
        }
        else
        {
            value = 0;
        }

        value = value << shift;
        mask = mask << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_CHANNEL_CONTROL_TRAFFIC_PROC_RESET_OFFSET(channel), value, mask);
    }

    return retval;
}




uint32_t Ethernet::GetRxTrafficProcessorReset(uint32_t channel, bool* pbInReset)
{
    uint32_t retval = XLNX_OK;
    uint32_t shift = 1;
    uint32_t value;
    uint32_t mask = 0x01;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_CONTROL_TRAFFIC_PROC_RESET_OFFSET(channel), &value);
    }


    if (retval == XLNX_OK)
    {
        *pbInReset = (bool)((value >> shift)& mask);
    }


    return retval;
}








uint32_t Ethernet::SetUnicastPromiscuousMode(uint32_t channel, bool bEnabled)
{
    uint32_t retval = XLNX_OK;
    uint32_t shift = 0;
    uint32_t value;
    uint32_t mask;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = CheckChannelConfigurationAllowed(channel);
    }

    if (retval == XLNX_OK)
    {
        if (bEnabled)
        {
            value = 1;
        }
        else
        {
            value = 0;
        }

        value = value << shift;
        mask = 1 << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_CHANNEL_CONTROL_TRAFFIC_PROC_CONFIG_OFFSET(channel), value, mask);
    }

    return retval;
}







uint32_t Ethernet::GetUnicastPromiscuousMode(uint32_t channel, bool* pbEnabled)
{
    uint32_t retval = XLNX_OK;
    uint32_t shift = 0;
    uint32_t value;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_CONTROL_TRAFFIC_PROC_CONFIG_OFFSET(channel), &value);
    }

    if (retval == XLNX_OK)
    {
        value = ((value >> shift) & 0x01);

        if (value != 0)
        {
            *pbEnabled = true;
        }
        else
        {
            *pbEnabled = false;
        }
    }

    return retval;
}









uint32_t Ethernet::SetMulticastPromiscuousMode(uint32_t channel, bool bEnabled)
{
    uint32_t retval = XLNX_OK;
    uint32_t shift = 1;
    uint32_t value;
    uint32_t mask;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = CheckChannelConfigurationAllowed(channel);
    }

    if (retval == XLNX_OK)
    {
        if (bEnabled)
        {
            value = 1;
        }
        else
        {
            value = 0;
        }

        value = value << shift;
        mask = 1 << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_CHANNEL_CONTROL_TRAFFIC_PROC_CONFIG_OFFSET(channel), value, mask);
    }

    return retval;
}








uint32_t Ethernet::GetMulticastPromiscuousMode(uint32_t channel, bool* pbEnabled)
{
    uint32_t retval = XLNX_OK;
    uint32_t shift = 1;
    uint32_t value;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_CONTROL_TRAFFIC_PROC_CONFIG_OFFSET(channel), &value);
    }

    if (retval == XLNX_OK)
    {
        value = ((value >> shift) & 0x01);

        if (value != 0)
        {
            *pbEnabled = true;
        }
        else
        {
            *pbEnabled = false;
        }
    }

    return retval;
}




uint32_t Ethernet::SetTxFIFOThreshold(uint32_t channel, uint8_t thresholdValue)
{
    uint32_t retval = XLNX_OK;


    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = CheckChannelConfigurationAllowed(channel);
    }


    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_ETHERNET_CHANNEL_CONTROL_TX_FIFO_THRESHOLD_OFFSET(channel), thresholdValue);
    }

    return retval;
}





uint32_t Ethernet::GetTxFIFOThreshold(uint32_t channel, uint8_t* pThresholdValue)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_CONTROL_TX_FIFO_THRESHOLD_OFFSET(channel), &value);
    }

    if (retval == XLNX_OK)
    {
        *pThresholdValue = (uint8_t)value;
    }

    return retval;
}





uint32_t Ethernet::SetRxCutThroughFIFO(uint32_t channel, bool bEnabled)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x1;
    uint32_t shift = 2;


    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = CheckChannelConfigurationAllowed(channel);
    }


    if (retval == XLNX_OK)
    {
        if (bEnabled)
        {
            value = 0x01;
        }
        else
        {
            value = 0x00;
        }

        value = value << shift;
        mask = mask << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_CHANNEL_CONTROL_TRAFFIC_PROC_CONFIG_OFFSET(channel), value, mask);
    }

 
    return retval;
}




uint32_t Ethernet::GetRxCutThroughFIFO(uint32_t channel, bool* pbEnabled)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 2;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_CONTROL_TRAFFIC_PROC_CONFIG_OFFSET(channel), &value);
    }

    if (retval == XLNX_OK)
    {
        *pbEnabled = (bool)((value >> shift) & mask);
    }

    return retval;
}

