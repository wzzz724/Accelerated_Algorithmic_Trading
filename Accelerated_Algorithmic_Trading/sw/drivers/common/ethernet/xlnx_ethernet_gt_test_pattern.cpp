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



uint32_t Ethernet::CheckTestPattern(GTTestPattern testPattern)
{
    uint32_t retval = XLNX_OK;

    switch (testPattern)
    {
        case(GTTestPattern::NO_PATTERN):
        case(GTTestPattern::PRBS_7):
        case(GTTestPattern::PRBS_9):
        case(GTTestPattern::PRBS_15):
        case(GTTestPattern::PRBS_23):
        case(GTTestPattern::PRBS_31):
        {
            //OK;
            break;
        }

        default:
        {
            retval = XLNX_ETHERNET_ERROR_INVALID_TEST_PATTERN;
        }
    }


    return retval;
}






uint32_t Ethernet::SetTxTestPattern(uint32_t channel, GTTestPattern testPattern)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t shift = 0;
    uint32_t mask = 0xF;

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }


    if (retval == XLNX_OK)
    {
        retval = CheckTestPattern(testPattern);
    }


    if (retval == XLNX_OK)
    {

        value = (uint32_t)testPattern;

        value = value << shift;
        mask = mask << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_TX_PRBS_CONTROL_OFFSET(channel), value, mask);
    }


    return retval;
}






uint32_t Ethernet::GetTxTestPattern(uint32_t channel, GTTestPattern* pTestPattern)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t shift = 0;
    uint32_t mask = 0xF;

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }


    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_TX_PRBS_CONTROL_OFFSET(channel), &value);
    }


    if (retval == XLNX_OK)
    {
        *pTestPattern = (GTTestPattern)((value >> shift) & mask);   
    }


    return retval;
}







uint32_t Ethernet::SetTxErrorInjection(uint32_t channel, bool bEnabled)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t shift = 8;
    uint32_t mask = 0x01;

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
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
        mask = mask << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_TX_PRBS_CONTROL_OFFSET(channel), value, mask);
    }


    return retval;
}






uint32_t Ethernet::GetTxErrorInjection(uint32_t channel, bool* pbEnabled)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t shift = 8;
    uint32_t mask = 0x1;

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }


    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_TX_PRBS_CONTROL_OFFSET(channel), &value);
    }


    if (retval == XLNX_OK)
    {
        value = ((value >> shift) & mask);

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




uint32_t Ethernet::SetRxExpectedTestPattern(uint32_t channel, GTTestPattern testPattern)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t shift = 0;
    uint32_t mask = 0xF;

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }


    if (retval == XLNX_OK)
    {
        retval = CheckTestPattern(testPattern);
    }


    if (retval == XLNX_OK)
    {

        value = (uint32_t)testPattern;

        value = value << shift;
        mask = mask << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_RX_PRBS_CONTROL_OFFSET(channel), value, mask);
    }


    return retval;


}


uint32_t Ethernet::GetRxExpectedTestPattern(uint32_t channel, GTTestPattern* pTestPattern)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t shift = 0;
    uint32_t mask = 0xF;

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }


    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_RX_PRBS_CONTROL_OFFSET(channel), &value);
    }


    if (retval == XLNX_OK)
    {
        *pTestPattern = (GTTestPattern)((value >> shift)& mask);
    }


    return retval;
}





uint32_t Ethernet::GetRxTestPatternStatus(uint32_t channel, bool* pbLocked, bool* pbLiveError, bool* pbLatchedError)
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
        retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_PRBS_STATUS_OFFSET(channel), &value);
    }

    if (retval == XLNX_OK)
    {
        *pbLocked       = (bool)(value & (1 << 0));
        *pbLiveError    = (bool)(value & (1 << 8));
        *pbLatchedError = (bool)(value & (1 << 24));
    }

    return retval;
}





uint32_t Ethernet::ClearRxTestPatternErrorLatch(uint32_t channel)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 24;

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }


    if (retval == XLNX_OK)
    {
        value = 1 << shift;
        mask = mask << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_PRBS_STATUS_OFFSET(channel), value, mask);
    }

    return retval;
}






uint32_t Ethernet::ResetRxTestPatternErrorCount(uint32_t channel)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t shift = 8;
    uint32_t mask = 0x01;

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }


    if(retval == XLNX_OK)
    {
        value = 1 << shift;
        mask = mask << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_RX_PRBS_CONTROL_OFFSET(channel), value, mask);
    }


    if (retval == XLNX_OK)
    {
        value = 0;

        retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_RX_PRBS_CONTROL_OFFSET(channel), value, mask);
    }

    return retval;
}

