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

#include <cstring> //for memset

#include "xlnx_ethernet.h"
#include "xlnx_ethernet_address_map.h"
#include "xlnx_ethernet_error_codes.h"
#include "xlnx_ethernet_types.h"
using namespace XLNX;



uint32_t Ethernet::GetRxBlockLock(uint32_t channel, RxBlockLock* pStatus)
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
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_STATUS_BLOCK_LOCK_OFFSET(channel), &value);
    }

    if (retval == XLNX_OK)
    {
        pStatus->bLockedLive        = (value & (1 << 0));
        pStatus->bLockedLatchedLow  = (value & (1 << 16));
    }

    return retval;
}





uint32_t Ethernet::ClearRxBlockLockLatches(uint32_t channel)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 16;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        value   = 0; //W0C

        value = value << shift;
        mask = mask << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_CHANNEL_STATUS_BLOCK_LOCK_OFFSET(channel), value, mask);
    }

    return retval;
}







uint32_t Ethernet::GetRxBufStatus(uint32_t channel, RxBufStatus* pStatus)
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
        retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_RX_RESET_INIT_STATUS_OFFSET(channel), &value);
    }

    if (retval == XLNX_OK)
    {
        pStatus->statusLive         = (GTRxBufStatus)((value >> 4) & 0x07);
        pStatus->bUnderflowLatched  = (bool)(value >> 20) & 0x01;
        pStatus->bOverflowLatched   = (bool)(value >> 21) & 0x01;
    }

    return retval;
}




uint32_t Ethernet::ClearRxBufStatusLatches(uint32_t channel)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t shift = 20;
    uint32_t mask = 0x03;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        value = mask; //W1C

        value = value << shift;
        mask = mask << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_RX_RESET_INIT_STATUS_OFFSET(channel), value, mask);
    }

    return retval;
}





uint32_t Ethernet::GetTxBufStatus(uint32_t channel, TxBufStatus* pStatus)
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
        retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_TX_RESET_INIT_STATUS_OFFSET(channel), &value);
    }

    if (retval == XLNX_OK)
    {
        pStatus->bFIFOHalfFull                      = (bool)((value >> 0) & 0x01);
        pStatus->bFIFOOverflowUnderflow             = (bool)((value >> 1) & 0x01);

        pStatus->bFIFOHalfFullLatchedHigh           = (bool)((value >> 20) & 0x01);
        pStatus->bFIFOOverflowUnderflowLatchedHigh  = (bool)((value >> 21) & 0x01);
    }

    return retval;
}





uint32_t Ethernet::ClearTxBufStatusLatches(uint32_t channel)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x03;
    uint32_t shift = 20;


    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        value = mask; //W1C

        value = value << shift;
        mask = mask << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_TX_RESET_INIT_STATUS_OFFSET(channel), value, mask);
    }

    return retval;
}





uint32_t Ethernet::GetGTPowerGoodStatus(uint32_t channel, GTPowerGood* pStatus)
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
        retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_TX_RESET_INIT_STATUS_OFFSET(channel), &value);
    }

    if (retval == XLNX_OK)
    {
        pStatus->bPowerGoodLive = (bool)((value >> 6) & 0x01);
        pStatus->bPowerGoodLatchedLow = (bool)((value >> 22) & 0x01);
    }

    return retval;
}




uint32_t Ethernet::ClearGTPowerGoodLatches(uint32_t channel)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 22;


    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        value = 0; //W0C

        value = value << shift;
        mask = mask << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_TX_RESET_INIT_STATUS_OFFSET(channel), value, mask);
    }

    return retval;
}






uint32_t Ethernet::GetRxTrafficProcessorStatus(uint32_t channel, RxTrafficProcessorStatus* pStatus)
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
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_STATUS_RX_TRAFFIC_PROC_STATUS_OFFSET(channel), &value);
    }

    if (retval == XLNX_OK)
    {
        pStatus->bDataFIFOOverflowLive          = (bool)((value >> 0) & 0x01);
        pStatus->bCommandFIFOOverflowLive       = (bool)((value >> 1) & 0x01);

        pStatus->bDataFIFOOverflowLatched       = (bool)((value >> 16) & 0x01);
        pStatus->bCommandFIFOOverflowLatched    = (bool)((value >> 17) & 0x01);
    }

    return retval;
}




uint32_t Ethernet::ClearRxTrafficProcessorStatusLatches(uint32_t channel)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x03;
    uint32_t shift = 16;


    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        value = mask; //W1C

        value = value << shift;
        mask = mask << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_CHANNEL_STATUS_RX_TRAFFIC_PROC_STATUS_OFFSET(channel), value, mask);
    }

    return retval;
}











uint32_t Ethernet::GetTxTrafficProcessorStatus(uint32_t channel, TxTrafficProcessorStatus* pStatus)
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
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_STATUS_TX_TRAFFIC_PROC_STATUS_OFFSET(channel), &value);
    }

    if (retval == XLNX_OK)
    {
        pStatus->bFIFOFullLive = (bool)((value >> 0) & 0x01);

        pStatus->bFIFOFullLatched = (bool)((value >> 16) & 0x01);

    }

    return retval;
}






uint32_t Ethernet::ClearTxTrafficProcessorStatusLatches(uint32_t channel)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 16;


    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        value = mask; //W1C

        value = value << shift;
        mask = mask << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_CHANNEL_STATUS_TX_TRAFFIC_PROC_STATUS_OFFSET(channel), value, mask);
    }

    return retval;
}






uint32_t Ethernet::GetEyescanStatus(uint32_t channel, EyescanStatus* pStatus)
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
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_STATUS_RX_TRAFFIC_PROC_STATUS_OFFSET(channel), &value);
    }

    if (retval == XLNX_OK)
    {
        pStatus->bEyescanDataErrorLatchedHigh = (bool)((value >> 16) & 0x01);

    }

    return retval;
}



uint32_t Ethernet::ClearEyescanStatusLatches(uint32_t channel)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 16;


    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        value = mask; //W1C

        value = value << shift;
        mask = mask << shift;

        retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_RX_MARGIN_ANALYSIS_OFFSET(channel), value, mask);
    }

    return retval;
}










uint32_t Ethernet::GetStatusFlags(uint32_t channel, StatusFlags* pStatusFlags)
{
    uint32_t retval = XLNX_OK;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = GetRxBlockLock(channel, &(pStatusFlags->rxBlockLock));
    }

    if (retval == XLNX_OK)
    {
        retval = GetRxBufStatus(channel, &(pStatusFlags->rxBufStatus));
    }

    if (retval == XLNX_OK)
    {
        retval = GetTxBufStatus(channel, &(pStatusFlags->txBufStatus));
    }

    if (retval == XLNX_OK)
    {
        retval = GetGTPowerGoodStatus(channel, &(pStatusFlags->gtPowerGood));
    }

    if (retval == XLNX_OK)
    {
        retval = GetRxTrafficProcessorStatus(channel, &(pStatusFlags->rxTrafficProcStatus));
    }

    if (retval == XLNX_OK)
    {
        retval = GetTxTrafficProcessorStatus(channel, &(pStatusFlags->txTrafficProcStatus));
    }

    if (retval == XLNX_OK)
    {
        retval = GetEyescanStatus(channel, &(pStatusFlags->eyescanStatus));
    }

    return retval;
}







uint32_t Ethernet::ClearStatusLatches(uint32_t channel)
{
    uint32_t retval = XLNX_OK;

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = ClearRxBlockLockLatches(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = ClearRxBufStatusLatches(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = ClearTxBufStatusLatches(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = ClearGTPowerGoodLatches(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = ClearRxTrafficProcessorStatusLatches(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = ClearTxTrafficProcessorStatusLatches(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = ClearEyescanStatusLatches(channel);
    }

    return retval;
}










uint32_t Ethernet::GetStats(uint32_t channel, Stats* pStats)
{
    uint32_t retval = XLNX_OK;

    memset(pStats, 0, sizeof(Stats));

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckChannel(channel);
    }

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_STATUS_RX_OVERFLOW_COUNT_OFFSET(channel), &(pStats->numFIFOOverflowDroppedFrames));
    }

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_STATUS_RX_UNICAST_DROP_COUNT_OFFSET(channel), &(pStats->numUnicastFilterDroppedFrames));
    }

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_STATUS_RX_MULTICAST_DROP_COUNT_OFFSET(channel), &(pStats->numMulticastFilterDroppedFrames));
    }

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_STATUS_RX_OVERSIZED_DROP_COUNT_OFFSET(channel), &(pStats->numOversizedDroppedFrames));
    }

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ETHERNET_CHANNEL_STATUS_TX_FIFO_UNDERFLOW_COUNT_OFFSET(channel), &(pStats->txFIFOUnderflowCount));
    }

    return retval;
}


