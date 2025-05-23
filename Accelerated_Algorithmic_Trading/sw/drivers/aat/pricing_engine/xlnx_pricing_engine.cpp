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


#include <string.h>

#include "xlnx_pricing_engine.h"
#include "xlnx_pricing_engine_address_map.h"
using namespace XLNX;




static const uint32_t IS_INITIALISED_MAGIC_NUMBER = 0x674217FB;


PricingEngine::PricingEngine()
{
    m_pDeviceInterface = nullptr;
    m_cuAddress = 0;
    m_cuIndex = 0;
    m_initialisedMagicNumber = 0;

    m_streamHandle = 0;

}



PricingEngine::~PricingEngine()
{


}





uint32_t PricingEngine::Initialise(DeviceInterface* pDeviceInterface, const char* cuName)
{
    uint32_t retval = XLNX_OK;

    m_initialisedMagicNumber = 0;	//will be set to magic number if we successfully initialise...

    m_pDeviceInterface = pDeviceInterface;

    strncpy(m_cuName, cuName, DeviceInterface::MAX_CU_NAME_LENGTH);
    m_cuName[DeviceInterface::MAX_CU_NAME_LENGTH] = '\0'; //always terminate the string...


    retval = m_pDeviceInterface->GetCUAddress(cuName, &m_cuAddress);

    if (retval != XLNX_OK)
    {
        retval = XLNX_PRICING_ENGINE_ERROR_CU_NAME_NOT_FOUND;
    }



    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->GetCUIndex(cuName, &m_cuIndex);

        if (retval != XLNX_OK)
        {
            retval = XLNX_PRICING_ENGINE_ERROR_CU_NAME_NOT_FOUND;
        }
    }





    if (retval == XLNX_OK)
    {
        m_initialisedMagicNumber = IS_INITIALISED_MAGIC_NUMBER;
    }

    return retval;
}










uint32_t PricingEngine::GetCUIndex(uint32_t* pCUIndex)
{
    uint32_t retval = XLNX_OK;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        *pCUIndex = m_cuIndex;
    }

    return retval;
}




uint32_t PricingEngine::GetCUAddress(uint64_t* pCUAddress)
{
    uint32_t retval = XLNX_OK;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        *pCUAddress = m_cuAddress;
    }

    return retval;
}













uint32_t PricingEngine::GetStats(PricingEngine::Stats* pStats)
{
    uint32_t retval = XLNX_OK;
  

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = FreezeStats();
    }

  
    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_PRICING_ENGINE_STATS_RX_RESPONSE_COUNT_OFFSET, &pStats->numRxResponses);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_PRICING_ENGINE_STATS_PROCESSED_RESPONSES_COUNT_OFFSET, &pStats->numProcessedResponses);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_PRICING_ENGINE_STATS_TX_OPERATIONS_COUNT_OFFSET, &pStats->numTxOperations);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_PRICING_ENGINE_STATS_STRATEGY_NONE_COUNT_OFFSET, &pStats->numStrategyNone);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_PRICING_ENGINE_STATS_STRATEGY_PEG_COUNT_OFFSET, &pStats->numStrategyPeg);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_PRICING_ENGINE_STATS_STRATEGY_LIMIT_COUNT_OFFSET, &pStats->numStrategyLimit);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_PRICING_ENGINE_STATS_STRATEGY_UNKNOWN_COUNT_OFFSET, &pStats->numStrategyUnknown);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_PRICING_ENGINE_STATS_CLOCK_TICK_EVENTS_COUNT_OFFSET, &pStats->numClockTickEvents);
    }

    if (retval == XLNX_OK)
    {
        retval = UnfreezeStats();
    }

    return retval;
}












uint32_t PricingEngine::ResetStats(void)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;
    uint32_t shift = 2;
    uint32_t mask = 0x01;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        offset = XLNX_PRICING_ENGINE_RESET_CONTROL_OFFSET;
        mask = mask << shift;

        value = 1;
        value = value << shift;

        retval = WriteRegWithMask32(offset, value, mask);
    }



    if (retval == XLNX_OK)
    {
        value = 0;
        value = value << shift;
        retval = WriteRegWithMask32(offset, value, mask);
    }

    return retval;

}









uint32_t PricingEngine::SetCaptureFilter(uint32_t symbolIndex)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;
    uint32_t mask;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        if (symbolIndex >= MAX_NUM_SYMBOLS)
        {
            retval = XLNX_PRICING_ENGINE_ERROR_SYMBOL_INDEX_OUT_OF_RANGE;
        }
    }


    if (retval == XLNX_OK)
    {
        offset = XLNX_PRICING_ENGINE_CAPTURE_CONTROL_OFFSET;
        value = symbolIndex;
        mask = 0x000000FF;

        retval = WriteRegWithMask32(offset, value, mask);
    }

    return retval;
}








uint32_t PricingEngine::GetCaptureFilter(uint32_t* pSymbolIndex)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        if (pSymbolIndex == nullptr)
        {
            retval = XLNX_PRICING_ENGINE_ERROR_INVALID_PARAMETER;
        }
    }


    if (retval == XLNX_OK)
    {
        offset = XLNX_PRICING_ENGINE_CAPTURE_CONTROL_OFFSET;

        retval = ReadReg32(offset, &value);
    }



    if (retval == XLNX_OK)
    {
        *pSymbolIndex = value & 0x000000FF;
    }

    return retval;
}












uint32_t PricingEngine::ReadData(PricingEngineData* pData)
{
    uint32_t retval = XLNX_OK;
    uint32_t buffer[XLNX_PRICING_ENGINE_NUM_CAPTURE_REGISTERS];

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        //Stop HW updates...
        retval = FreezeData();
    }

    if (retval == XLNX_OK)
    {
        //Take a snapshot of the data....
        retval = BlockReadReg32(XLNX_PRICING_ENGINE_CAPTURE_OFFSET, buffer, XLNX_PRICING_ENGINE_NUM_CAPTURE_REGISTERS);
    }



    if (retval == XLNX_OK)
    {
        //The following bitshift operations are due to several things:
        //
        //(1) The HW pushes its data out basically as a PACKED STRUCT.
        //(2) That struct has some 8-bit fields, which moves the next field onto a byte boundary (rather than a nice word boundary).
        //(3) the SW performs 32-bit reads on the AXI bus at even word boundaries (+0x00, +0x04, +0x08 and +0x0C)

        pData->orderSide        = (OrderSide) (buffer[0] & 0x000000FF);
        pData->orderPrice       = ((buffer[0] >> 8) & 0x00FFFFFF) | ((buffer[1] << 24) & 0xFF000000);
        pData->orderQuantity    = ((buffer[1] >> 8) & 0x00FFFFFF) | ((buffer[2] << 24) & 0xFF000000);
        pData->orderID          = ((buffer[2] >> 8) & 0x00FFFFFF) | ((buffer[3] << 24) & 0xFF000000);
        pData->symbolIndex      = ((buffer[3] >> 8) & 0x000000FF);
        pData->orderOperation   = (OrderOperation) ((buffer[3] >> 16) & 0x000000FF);
    }


    if (retval == XLNX_OK)
    {
        //Restart HW updates...
        retval = UnfreezeData();
    }

    return retval;
}
















uint32_t PricingEngine::Start(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t offset;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 0;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        offset = XLNX_PRICING_ENGINE_RESET_CONTROL_OFFSET;
        mask = mask << shift;

        value = 0;
        value = value << shift;

        retval = WriteRegWithMask32(offset, value, mask);
    }

    return retval;
}






uint32_t PricingEngine::Stop(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t offset;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 0;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        offset = XLNX_PRICING_ENGINE_RESET_CONTROL_OFFSET;
        mask = mask << shift;

        value = 1;
        value = value << shift;

        retval = WriteRegWithMask32(offset, value, mask);
    }

    return retval;
}






uint32_t PricingEngine::IsRunning(bool* pbIsRunning)
{
    uint32_t retval = XLNX_OK;
    uint32_t offset;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 0;

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        offset = XLNX_PRICING_ENGINE_RESET_CONTROL_OFFSET;

        retval = ReadReg32(offset, &value);
    }


    if (retval == XLNX_OK)
    {
        value = (value >> shift)& mask;

        if (value == 0)
        {
            *pbIsRunning = true;
        }
        else
        {
            *pbIsRunning = false;
        }
    }

    return retval;
}














uint32_t PricingEngine::CheckIsInitialised(void)
{
    uint32_t retval = XLNX_OK;

    if (m_initialisedMagicNumber != IS_INITIALISED_MAGIC_NUMBER)
    {
        retval = XLNX_PRICING_ENGINE_ERROR_NOT_INITIALISED;
    }

    return retval;
}








void PricingEngine::IsInitialised(bool* pbIsInitialised)
{
    if (CheckIsInitialised() == XLNX_OK)
    {
        *pbIsInitialised = true;
    }
    else
    {
        *pbIsInitialised = false;
    }
}











uint32_t PricingEngine::FreezeData(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 31;

    //set the capture-freeze bit
    value = 1;

    value = value << shift;
    mask = mask << shift;

    retval = WriteRegWithMask32(XLNX_PRICING_ENGINE_CAPTURE_CONTROL_OFFSET, value, mask);


    return retval;
}






uint32_t PricingEngine::UnfreezeData(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 31;

    //clear the capture-freeze bit
    value = 0;

    value = value << shift;
    mask = mask << shift;

    retval = WriteRegWithMask32(XLNX_PRICING_ENGINE_CAPTURE_CONTROL_OFFSET, value, mask);


    return retval;
}












uint32_t PricingEngine::FreezeStats(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 31;

    //set the stats-freeze bit
    value = 1;

    value = value << shift;
    mask = mask << shift;

    retval = WriteRegWithMask32(XLNX_PRICING_ENGINE_RESET_CONTROL_OFFSET, value, mask);


    return retval;
}






uint32_t PricingEngine::UnfreezeStats(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 31;

    //clear the stats-freeze bit
    value = 0;

    value = value << shift;
    mask = mask << shift;

    retval = WriteRegWithMask32(XLNX_PRICING_ENGINE_RESET_CONTROL_OFFSET, value, mask);


    return retval;
}










uint32_t PricingEngine::ReadReg32(uint64_t offset, uint32_t* value)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->ReadReg32(address, value);




    if (retval != XLNX_OK)
    {
        retval = XLNX_PRICING_ENGINE_ERROR_IO_FAILED;
    }

    return retval;
}






uint32_t PricingEngine::WriteReg32(uint64_t offset, uint32_t value)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->WriteReg32(address, value);


    if (retval != XLNX_OK)
    {
        retval = XLNX_PRICING_ENGINE_ERROR_IO_FAILED;
    }

    return retval;
}




uint32_t PricingEngine::WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->WriteRegWithMask32(address, value, mask);

    if (retval != XLNX_OK)
    {
        retval = XLNX_PRICING_ENGINE_ERROR_IO_FAILED;
    }

    return retval;
}






uint32_t PricingEngine::BlockReadReg32(uint64_t offset, uint32_t* buffer, uint32_t numWords)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->BlockReadReg32(address, buffer, numWords);


    if (retval != XLNX_OK)
    {
        retval = XLNX_PRICING_ENGINE_ERROR_IO_FAILED;
    }

    return retval;
}





uint32_t PricingEngine::SetGlobalStrategyMode(bool bEnabled)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 31;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        offset = XLNX_PRICING_ENGINE_GLOBAL_STRATEGY_CONTOL_OFFSET;

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

        retval = WriteRegWithMask32(offset, value, mask);
    }


    return retval;
}






uint32_t PricingEngine::GetGlobalStrategyMode(bool* pbEnabled)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 31;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        offset = XLNX_PRICING_ENGINE_GLOBAL_STRATEGY_CONTOL_OFFSET;
        retval = ReadReg32(offset, &value);
    }

    if (retval == XLNX_OK)
    {
        value = (value >> shift) & mask;

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






uint32_t PricingEngine::SetGlobalStrategy(PricingStrategy strategy)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;
    uint32_t mask = 0xFF;
    uint32_t shift = 0;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        offset = XLNX_PRICING_ENGINE_GLOBAL_STRATEGY_CONTOL_OFFSET;
        value = (uint32_t)strategy;

        value = value << shift;
        mask = mask << shift;

        retval = WriteRegWithMask32(offset, value, mask);
    }

    return retval;
}






uint32_t PricingEngine::GetGlobalStrategy(PricingStrategy* pStrategy)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;
    uint32_t mask = 0xFF;
    uint32_t shift = 0;

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        offset = XLNX_PRICING_ENGINE_GLOBAL_STRATEGY_CONTOL_OFFSET;

        retval = ReadReg32(offset, &value);
    }


    if (retval == XLNX_OK)
    {
        value = (value >> shift) & mask;

        *pStrategy = (PricingStrategy)value;
    }



    return retval;
}



