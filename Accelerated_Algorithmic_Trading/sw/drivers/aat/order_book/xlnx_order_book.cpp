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

#include <string.h> //for memset,memcpy

#include "xlnx_order_book.h"
#include "xlnx_order_book_address_map.h"
using namespace XLNX;




static const uint32_t IS_INITIALISED_MAGIC_NUMBER = 0x4362AA5B;


OrderBook::OrderBook()
{
	m_pDeviceInterface = nullptr;

	m_cuIndex = 0xFFFFFFFF;
	m_cuAddress = 0;

	m_initialisedMagicNumber = 0;
}






OrderBook::~OrderBook()
{

}







uint32_t OrderBook::Initialise(DeviceInterface* pDeviceInterface, const char* cuName)
{
	uint32_t retval = XLNX_OK;

    m_initialisedMagicNumber = 0;	//will be set to magic number if we successfully initialise...

	m_pDeviceInterface = pDeviceInterface;

    retval = m_pDeviceInterface->GetCUAddress(cuName, &m_cuAddress);	
	if (retval != XLNX_OK)
	{
		retval = XLNX_ORDER_BOOK_ERROR_CU_NAME_NOT_FOUND;
	}


	if (retval == XLNX_OK)
	{
        retval = m_pDeviceInterface->GetCUIndex(cuName, &m_cuIndex);
		if (retval != XLNX_OK)
		{
			retval = XLNX_ORDER_BOOK_ERROR_CU_NAME_NOT_FOUND;
		}
	}



	if (retval == XLNX_OK)
	{
		m_initialisedMagicNumber = IS_INITIALISED_MAGIC_NUMBER;
	}





	return retval;
}














uint32_t OrderBook::CheckIsInitialised(void)
{
	uint32_t retval = XLNX_OK;

	if (m_initialisedMagicNumber != IS_INITIALISED_MAGIC_NUMBER)
	{
		retval = XLNX_ORDER_BOOK_ERROR_NOT_INITIALISED;
	}

	return retval;
}







void OrderBook::IsInitialised(bool* pbIsInitialised)
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





uint32_t OrderBook::GetCUIndex(uint32_t* pCUIndex)
{
	uint32_t retval = XLNX_OK;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		*pCUIndex = m_cuIndex;
	}

	return retval;
}




uint32_t OrderBook::GetCUAddress(uint64_t* pCUAddress)
{
	uint32_t retval = XLNX_OK;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		*pCUAddress = m_cuAddress;
	}

	return retval;
}




uint32_t OrderBook::FreezeData(void)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 31;

	//set the capture-freeze bit
	value = 1;

	value = value << shift;
	mask = mask << shift;

	retval = WriteRegWithMask32(XLNX_ORDER_BOOK_CAPTURE_CONTROL_OFFSET, value, mask);


	return retval;
}






uint32_t OrderBook::UnfreezeData(void)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 31;

	//clear the capture-freeze bit
	value = 0;

	value = value << shift;
	mask = mask << shift;

	retval = WriteRegWithMask32(XLNX_ORDER_BOOK_CAPTURE_CONTROL_OFFSET, value, mask);


	return retval;
}


uint32_t OrderBook::FreezeStats(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 31;

    //set the stats-freeze bit
    value = 1;

    value = value << shift;
    mask = mask << shift;

    retval = WriteRegWithMask32(XLNX_ORDER_BOOK_RESET_CONTROL_OFFSET, value, mask);


    return retval;
}



uint32_t OrderBook::UnfreezeStats(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 31;

    //clear the stats-freeze bit
    value = 0;

    value = value << shift;
    mask = mask << shift;

    retval = WriteRegWithMask32(XLNX_ORDER_BOOK_RESET_CONTROL_OFFSET, value, mask);


    return retval;
}




uint32_t OrderBook::SetCaptureFilter(uint32_t symbolIndex)
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
            retval = XLNX_ORDER_BOOK_ERROR_SYMBOL_INDEX_OUT_OF_RANGE;
        }
    }


    if(retval == XLNX_OK)
    {
        offset = XLNX_ORDER_BOOK_CAPTURE_CONTROL_OFFSET;
        value = symbolIndex;
        mask = 0x000000FF;

        retval = WriteRegWithMask32(offset, value, mask);
    }

    return retval;
}








uint32_t OrderBook::GetCaptureFilter(uint32_t* pSymbolIndex)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        if (pSymbolIndex == nullptr)
        {
            retval = XLNX_ORDER_BOOK_ERROR_INVALID_PARAMETER;
        }
    }


    if (retval == XLNX_OK)
    {
        offset = XLNX_ORDER_BOOK_CAPTURE_CONTROL_OFFSET;

        retval = ReadReg32(offset, &value);
    }



    if (retval == XLNX_OK)
    {
        *pSymbolIndex = value & 0x000000FF;
    }

    return retval;
}











uint32_t OrderBook::ReadData(OrderBookData* pData)
{
	uint32_t retval = XLNX_OK;
    uint8_t buffer[HW_DATA_SIZE_BYTES];

	retval = CheckIsInitialised();


	if (retval == XLNX_OK)
	{
		//Stop HW updates...
		retval = FreezeData();
	}

	if (retval == XLNX_OK)
	{
		//Take a snapshot of the data....
		retval = BlockReadReg32(XLNX_ORDER_BOOK_DATA_OFFSET, (uint32_t*)buffer, HW_DATA_SIZE_BYTES / sizeof(uint32_t));
	}

    if (retval == XLNX_OK)
    {
        memset(pData, 0, sizeof(*pData));

        memcpy(&pData->timestamp,         &buffer[121], 7); //NOTE - HW timestamp is currently only 56-bits wide...
        memcpy(&pData->symbolIndex,       &buffer[120], 1);
        memcpy(&pData->bidCount[0],       &buffer[100], 20);
        memcpy(&pData->bidPrice[0],       &buffer[80],  20);
        memcpy(&pData->bidQuantity[0],    &buffer[60],  20);
        memcpy(&pData->askCount[0],       &buffer[40],  20);
        memcpy(&pData->askPrice[0],       &buffer[20],  20);
        memcpy(&pData->askQuantity[0],    &buffer[0],   20);
    }


	if (retval == XLNX_OK)
	{
		//Restart HW updates...
		retval = UnfreezeData();
	}

	return retval;
}









uint32_t OrderBook::ReadReg32(uint64_t offset, uint32_t* value)
{
	uint32_t retval = XLNX_OK;
	uint64_t address;


	address = m_cuAddress + offset;

	retval = m_pDeviceInterface->ReadReg32(address, value);




	if (retval != XLNX_OK)
	{
		retval = XLNX_ORDER_BOOK_ERROR_IO_FAILED;
	}

	return retval;
}






uint32_t OrderBook::WriteReg32(uint64_t offset, uint32_t value)
{
	uint32_t retval = XLNX_OK;
	uint64_t address;


	address = m_cuAddress + offset;

	retval = m_pDeviceInterface->WriteReg32(address, value);


	if (retval != XLNX_OK)
	{
		retval = XLNX_ORDER_BOOK_ERROR_IO_FAILED;
	}

	return retval;
}




uint32_t OrderBook::WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask)
{
	uint32_t retval = XLNX_OK;
	uint64_t address;


	address = m_cuAddress + offset;

	retval = m_pDeviceInterface->WriteRegWithMask32(address, value, mask);

	if (retval != XLNX_OK)
	{
		retval = XLNX_ORDER_BOOK_ERROR_IO_FAILED;
	}

	return retval;
}



uint32_t OrderBook::BlockReadReg32(uint64_t offset, uint32_t* buffer, uint32_t numWords)
{
	uint32_t retval = XLNX_OK;
	uint64_t address;


	address = m_cuAddress + offset;

	retval = m_pDeviceInterface->BlockReadReg32(address, buffer, numWords);


	if (retval != XLNX_OK)
	{
		retval = XLNX_ORDER_BOOK_ERROR_IO_FAILED;
	}

	return retval;
}





uint32_t OrderBook::BlockReadMem32(uint64_t address, uint32_t* buffer, uint32_t numWords)
{
	uint32_t retval = XLNX_OK;

	retval = m_pDeviceInterface->BlockReadMem32(address, buffer, numWords);

	if (retval != XLNX_OK)
	{
		retval = XLNX_ORDER_BOOK_ERROR_IO_FAILED;
	}

	return retval;
}






uint32_t OrderBook::GetStats(OrderBook::Stats* pStats)
{
    uint32_t retval = XLNX_OK;
 
    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = FreezeStats();
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_BOOK_STATS_RX_OPERATIONS_COUNT_OFFSET, &pStats->numRxOperations);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_BOOK_STATS_PROCESS_OPERATIONS_COUNT_OFFSET, &pStats->numProcessedOperations);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_BOOK_STATS_INVALID_OPERATIONS_COUNT_OFFSET, &pStats->numInvalidOperations);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_BOOK_RESPONSES_GENERATED_COUNT_OFFSET, &pStats->numResponsesGenerated);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_BOOK_RESPONSE_SENT_COUNT_OFFSET, &pStats->numResponsesSent);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_BOOK_ADD_OPERATIONS_COUNT_OFFSET, &pStats->numAddOperations);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_BOOK_MODIFY_OPERATIONS_COUNT_OFFSET, &pStats->numModifyOperations);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_BOOK_DELETE_OPERATIONS_COUNT_OFFSET, &pStats->numDeleteOperations);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_BOOK_TRANSACT_OPERATIONS_COUNT_OFFSET, &pStats->numTransactOperations);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_BOOK_HALT_OPERATIONS_COUNT_OFFSET, &pStats->numHaltOperations);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_BOOK_TIMESTAMP_ERRORS_COUNT_OFFSET, &pStats->numTimestampErrors);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_BOOK_UNHANDLED_OP_CODES_COUNT_OFFSET, &pStats->numUnhandledOpCodes);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_BOOK_SYMBOL_ERRORS_COUNT_OFFSET, &pStats->numSymbolErrors);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_BOOK_DIRECTION_ERRORS_COUNT_OFFSET, &pStats->numDirectionErrors);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_BOOK_LEVEL_ERRORS_COUNT_OFFSET, &pStats->numLevelErrors);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_BOOK_CLOCK_TICK_GEN_EVENTS_COUNT_OFFSET, &pStats->numClockTickGeneratorEvents);
    }

   
    if (retval == XLNX_OK)
    {
        retval = UnfreezeStats();
    }

 
    return retval;
}







uint32_t OrderBook::ResetStats(void)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;
    uint32_t shift = 2;
    uint32_t mask = 0x01;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        offset = XLNX_ORDER_BOOK_RESET_CONTROL_OFFSET;
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










uint32_t OrderBook::Start(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t offset;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 0;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        offset = XLNX_ORDER_BOOK_RESET_CONTROL_OFFSET;
        mask = mask << shift;

        value = 0;
        value = value << shift;

        retval = WriteRegWithMask32(offset, value, mask);
    }

    return retval;
}



uint32_t OrderBook::Stop(void)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 0;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        offset = XLNX_ORDER_BOOK_RESET_CONTROL_OFFSET;
        mask = mask << shift;

        value = 1;
        value = value << shift;

        retval = WriteRegWithMask32(offset, value, mask);
    }

    return retval;
}






uint32_t OrderBook::IsRunning(bool* pbIsRunning)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 0;

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        offset = XLNX_ORDER_BOOK_RESET_CONTROL_OFFSET;

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






uint32_t OrderBook::SetDataMoverOutput(bool bEnabled)
{
    uint32_t retval = XLNX_OK;
    uint32_t offset;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 3;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        offset = XLNX_ORDER_BOOK_RESET_CONTROL_OFFSET;

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





uint32_t OrderBook::GetDataMoverOutput(bool* pbEnabled)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value = 0;
    uint32_t mask = 0x01;
    uint32_t shift = 3;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        offset = XLNX_ORDER_BOOK_RESET_CONTROL_OFFSET;

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

