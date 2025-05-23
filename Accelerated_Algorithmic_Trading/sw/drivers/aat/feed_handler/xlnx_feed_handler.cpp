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

#include "xlnx_feed_handler.h"
#include "xlnx_feed_handler_address_map.h"
using namespace XLNX;




static const uint32_t IS_INITIALISED_MAGIC_NUMBER = 0xD284583A;

static const uint32_t EMPTY_SECURITY_ID = 0x00000000;


static const uint32_t NUM_BYTES_PER_HW_WORD = 8;


FeedHandler::FeedHandler()
{
	m_pDeviceInterface = nullptr;
	m_cuAddress = 0;
	m_cuIndex = 0;
	m_initialisedMagicNumber = 0;


    m_numSecurites = 0;
    for (uint32_t i = 0; i < MAX_NUM_SECURITIES; i++)
    {
        m_securityIDLookup[i] = EMPTY_SECURITY_ID;
    }
}






FeedHandler::~FeedHandler()
{

}







uint32_t FeedHandler::Initialise(DeviceInterface* pDeviceInterface, const char* cuName)
{
	uint32_t retval = XLNX_OK;

    m_initialisedMagicNumber = 0;	//will be set to magic number if we successfully initialise...

	m_pDeviceInterface = pDeviceInterface;


    retval = m_pDeviceInterface->GetCUAddress(cuName, &m_cuAddress);

    if (retval != XLNX_OK)
    {
        retval = XLNX_FEED_HANDLER_ERROR_CU_NAME_NOT_FOUND;
    }



    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->GetCUIndex(cuName, &m_cuIndex);

        if (retval != XLNX_OK)
        {
            retval = XLNX_FEED_HANDLER_ERROR_CU_NAME_NOT_FOUND;
        }
    }


    if (retval == XLNX_OK)
    {
       retval = InternalPopulateCache();
    }



	if (retval == XLNX_OK)
	{
		m_initialisedMagicNumber = IS_INITIALISED_MAGIC_NUMBER;
	}



	return retval;
}









uint32_t FeedHandler::ReadReg32(uint64_t offset, uint32_t* value)
{
	uint32_t retval = XLNX_OK;
	uint32_t address = m_cuAddress + offset;

	retval = m_pDeviceInterface->ReadReg32(address, value);

	if (retval != XLNX_OK)
	{
		retval = XLNX_FEED_HANDLER_ERROR_IO_FAILED;
	}

	return retval;
}






uint32_t FeedHandler::WriteReg32(uint64_t offset, uint32_t value)
{
	uint32_t retval = XLNX_OK;
	uint64_t address = m_cuAddress + offset;

	retval = m_pDeviceInterface->WriteReg32(address, value);

	if (retval != XLNX_OK)
	{
		retval = XLNX_FEED_HANDLER_ERROR_IO_FAILED;
	}

	return retval;
}




uint32_t FeedHandler::WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask)
{
	uint32_t retval = XLNX_OK;
	uint64_t address = m_cuAddress + offset;

	retval = m_pDeviceInterface->WriteRegWithMask32(address, value, mask);

	if (retval != XLNX_OK)
	{
		retval = XLNX_FEED_HANDLER_ERROR_IO_FAILED;
	}

	return retval;
}



uint32_t FeedHandler::BlockReadReg32(uint64_t offset, uint32_t* buffer, uint32_t numWords)
{
	uint32_t retval = XLNX_OK;
	uint64_t address = m_cuAddress + offset;

	retval = m_pDeviceInterface->BlockReadReg32(address, buffer, numWords);

	if (retval != XLNX_OK)
	{
		retval = XLNX_FEED_HANDLER_ERROR_IO_FAILED;
	}

	return retval;
}











uint32_t FeedHandler::AddSecurity(uint32_t securityID, uint32_t* pIndex)
{
	uint32_t retval = XLNX_OK;
    uint32_t index;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckSecurityID(securityID);
    }

    if (retval == XLNX_OK)
    {
        //we need to check the specified securityID has NOT already been added...

        //first do a lookup on the specified security ID
        retval = GetIndexForSecurityID(securityID, &index);


        //if the symbol does not already exist...we are OK to add it...
        if (retval == XLNX_FEED_HANDLER_ERROR_SECURITY_ID_DOES_NOT_EXIST)
        {
            retval = XLNX_OK;
        }
        else if(retval == XLNX_OK)  //if the lookup was SUCCESSFUL...the security ID already exists...
        {
            retval = XLNX_FEED_HANDLER_ERROR_SECURITY_ID_ALREADY_EXISTS;
        }

    }



    if (retval == XLNX_OK)
    {
        //find a free index.....
        retval = InternalGetFreeSecurityIndex(&index);
    }

    if (retval == XLNX_OK)
    {
        //if we got a free index...write the security ID down to HW at the correct index...
        retval = InternalWriteSecurityIDToHW(index, securityID);
    }

    if (retval == XLNX_OK)
    {
        //update the cache...
        m_securityIDLookup[index] = securityID;
        m_numSecurites++;


        //give the user back the index...
        *pIndex = index;
    }



	return retval;
}





uint32_t FeedHandler::AddSecurityAtIndex(uint32_t securityID, uint32_t index)
{
    uint32_t retval = XLNX_OK;
    uint32_t existingSecID;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckSecurityID(securityID);
    }

    if (retval == XLNX_OK)
    {
        //we need to check the specified securityID has NOT already been added...

        //first do a lookup on the specified security ID
        retval = GetIndexForSecurityID(securityID, &index);


        //if the symbol does not already exist...we are OK to add it...
        if (retval == XLNX_FEED_HANDLER_ERROR_SECURITY_ID_DOES_NOT_EXIST)
        {
            retval = XLNX_OK;
        }
        else if (retval == XLNX_OK)  //if the lookup was SUCCESSFUL...the security ID already exists...
        {
            retval = XLNX_FEED_HANDLER_ERROR_SECURITY_ID_ALREADY_EXISTS;
        }

    }


    if (retval == XLNX_OK)
    {
        //need to check as well that the specified index is free...
        retval = GetSecurityIDAtIndex(index, &existingSecID);
        if (retval == XLNX_FEED_HANDLER_ERROR_NO_SECURITY_AT_SPECIFIED_INDEX)
        {
            //if the index was free, we are OK to use it to add the specified security ID
            retval = XLNX_OK;
        }
    }





    if (retval == XLNX_OK)
    {
        //if we get to here, we are good to add the security ID to the specified index...
        retval = InternalWriteSecurityIDToHW(index, securityID);
    }


    if (retval == XLNX_OK)
    {
        //update the cache...
        m_securityIDLookup[index] = securityID;
        m_numSecurites++;
    }

    return retval;
}





uint32_t FeedHandler::RemoveSecurity(uint32_t securityID)
{
	uint32_t retval = XLNX_OK;
    uint32_t index;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckSecurityID(securityID);
    }

    if (retval == XLNX_OK)
    {
        retval = GetIndexForSecurityID(securityID, &index);
    }


    if (retval == XLNX_OK)
    {
        retval = InternalWriteSecurityIDToHW(index, EMPTY_SECURITY_ID);
    }

    if (retval == XLNX_OK)
    {
        m_securityIDLookup[index] = EMPTY_SECURITY_ID;

        if (m_numSecurites > 0)
        {
            m_numSecurites--;
        }
    }


	return retval;
}






uint32_t FeedHandler::RemoveAllSecurities(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t i;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        for (i = 0; i < MAX_NUM_SECURITIES; i++)
        {
            retval = InternalWriteSecurityIDToHW(i, EMPTY_SECURITY_ID);

            if (retval == XLNX_OK)
            {
                m_securityIDLookup[i] = EMPTY_SECURITY_ID;
            }
        }

        if (retval == XLNX_OK)
        {
            m_numSecurites = 0;
        }
    }

    return retval;
}






uint32_t  FeedHandler::GetStats(FeedHandler::Stats* pStats)
{
	uint32_t retval = XLNX_OK;
    uint32_t numProcessedWords;

	retval = CheckIsInitialised();


	

    if (retval == XLNX_OK)
    {
        //NOTE - for the following, the HW reports number of processed WORDS.
        //       Each word is 8 bytes in HW.  We do a conversion here so we 
        //       report the number of processed BYTES to the end-user
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_FEED_HANDLER_STATS_PROCESSED_WORDS_COUNT_OFFSET, &numProcessedWords);

        if (retval == XLNX_OK)
        {
            pStats->numProcessedBytes = ((uint64_t)numProcessedWords) * NUM_BYTES_PER_HW_WORD;
        }
    }


    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_FEED_HANDLER_STATS_PROCESSED_PACKETS_COUNT_OFFSET, &pStats->numProcessedPackets);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_FEED_HANDLER_STATS_PROCESSED_BINARY_MSG_COUNT_OFFSET, &pStats->numProcessedBinaryMessages);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_FEED_HANDLER_STATS_PROCESSED_FIX_MSG_COUNT_OFFSET, &pStats->numProcessedFIXMessages);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_FEED_HANDLER_STATS_TX_OPERATION_COUNT_OFFSET, &pStats->numTxOrderBookOperations);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_FEED_HANDLER_STATS_CLOCK_TICK_COUNT_OFFSET, &pStats->numClockTickEvents);
    }


	return retval;
}








uint32_t  FeedHandler::ResetStats(void)
{
	uint32_t retval = XLNX_OK;
	uint64_t offset;
	uint32_t value;
	uint32_t shift = 2;
	uint32_t mask = 0x01;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		offset = XLNX_FEED_HANDLER_RESET_CONTROL_OFFSET;
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













uint32_t FeedHandler::GetSecurityAtContiguousIndex(uint32_t contiguousIndex, uint32_t* pSecurityIndex, uint32_t* pSecurityID)
{
    uint32_t retval = XLNX_OK;
    uint32_t rc;
    uint32_t numSymbols = 0;
    uint32_t count;
    uint32_t i;
    uint32_t securityID;
    bool bFound = false;



    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        GetNumSecurities(&numSymbols);

        if (contiguousIndex >= numSymbols)
        {
            retval = XLNX_FEED_HANDLER_ERROR_SECURITY_INDEX_OUT_OF_RANGE;
        }
    }



    if (retval == XLNX_OK)
    {

        count = 0;

        for (i = 0; i < MAX_NUM_SECURITIES; i++)
        {
            rc = GetSecurityIDAtIndex(i, &securityID);

            if (rc == XLNX_OK)
            {
                if (count == contiguousIndex)
                {
                    *pSecurityIndex = i;
                    *pSecurityID = securityID;
                    bFound = true;
                    break;
                }

                count++;
            }
        }


        if (bFound == false)
        {
            retval = XLNX_FEED_HANDLER_ERROR_NO_SECURITY_AT_SPECIFIED_INDEX;
        }
    }

    return retval;
}







uint32_t FeedHandler::CheckIsInitialised(void)
{
	uint32_t retval = XLNX_OK;

	if (m_initialisedMagicNumber != IS_INITIALISED_MAGIC_NUMBER)
	{
		retval = XLNX_FEED_HANDLER_ERROR_NOT_INITIALISED;
	}

	return retval;
}







void FeedHandler::IsInitialised(bool* pbIsInitialised)
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






uint32_t FeedHandler::GetCUAddress(uint64_t* pCUAddress)
{
	uint32_t retval = XLNX_OK;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		*pCUAddress = m_cuAddress;
	}

	return retval;
}




uint32_t FeedHandler::GetCUIndex(uint32_t* pCUIndex)
{
	uint32_t retval = XLNX_OK;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		*pCUIndex = m_cuIndex;
	}

	return retval;
}






uint32_t FeedHandler::Start(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t offset;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 0;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        offset = XLNX_FEED_HANDLER_RESET_CONTROL_OFFSET;
        mask = mask << shift;

        value = 0;
        value = value << shift;

        retval = WriteRegWithMask32(offset, value, mask);
    }

    return retval;
}






uint32_t FeedHandler::Stop(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t offset;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 0;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        offset = XLNX_FEED_HANDLER_RESET_CONTROL_OFFSET;
        mask = mask << shift;

        value = 1;
        value = value << shift;

        retval = WriteRegWithMask32(offset, value, mask);
    }

    return retval;
}






uint32_t FeedHandler::IsRunning(bool* pbIsRunning)
{
    uint32_t retval = XLNX_OK;
    uint32_t offset;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 0;

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        offset = XLNX_FEED_HANDLER_RESET_CONTROL_OFFSET;

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
















uint32_t FeedHandler::CheckSecurityIndex(uint32_t index)
{
    uint32_t retval = XLNX_OK;

    if (index >= MAX_NUM_SECURITIES)
    {
        retval = XLNX_FEED_HANDLER_ERROR_SECURITY_INDEX_OUT_OF_RANGE;
    }

    return retval;
}



uint32_t FeedHandler::CheckSecurityID(uint32_t securityID)
{
    uint32_t retval = XLNX_OK;

    if (securityID == EMPTY_SECURITY_ID)
    {
        retval = XLNX_FEED_HANDLER_ERROR_INVALID_SECURITY_ID;
    }

    return retval;
}








uint32_t FeedHandler::InternalWriteSecurityIDToHW(uint32_t index, uint32_t securityID)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;

    offset = XLNX_FEED_HANDLER_SYMBOL_MAP_START_OFFSET + (index * XLNX_FEED_HANDLER_SYMBOL_INDEX_MULTIPLIER);

    retval = WriteReg32(offset, securityID);

    return retval;
}




uint32_t FeedHandler::InternalReadSecurityIDFromHW(uint32_t index, uint32_t* pSecurityID)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;

    offset = XLNX_FEED_HANDLER_SYMBOL_MAP_START_OFFSET + (index * XLNX_FEED_HANDLER_SYMBOL_INDEX_MULTIPLIER);

    retval = ReadReg32(offset, &value);

    if (retval == XLNX_OK)
    {
        *pSecurityID = value;
    }


    return retval;
}



uint32_t FeedHandler::InternalPopulateCache(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t securityID;

    //Start by zeroing our count of securities.
    //It will be incremented for each non-zero security ID we find...
    m_numSecurites = 0;


    for (uint32_t i= 0; i < MAX_NUM_SECURITIES; i++)
    {
        retval = InternalReadSecurityIDFromHW(i, &securityID);

        if (retval == XLNX_OK)
        {
            if (securityID != EMPTY_SECURITY_ID)
            {
                m_numSecurites++;
            }

            m_securityIDLookup[i] = securityID;
        }
        else
        {
            break; //out of loop
        }

    }

    return retval;
}





uint32_t FeedHandler::RefreshCache(void)
{
    return InternalPopulateCache();
}




void FeedHandler::GetNumSecurities(uint32_t* pNumSecurities)
{
    *pNumSecurities =  m_numSecurites;
}




uint32_t FeedHandler::GetSecurityIDAtIndex(uint32_t index, uint32_t* pSecurityID)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckSecurityIndex(index);
    }

    if (retval == XLNX_OK)
    {
        //just read from the cache...
       value = m_securityIDLookup[index];

       if (value != EMPTY_SECURITY_ID)
       {
           *pSecurityID = value;
       }
       else
       {
           retval = XLNX_FEED_HANDLER_ERROR_NO_SECURITY_AT_SPECIFIED_INDEX;
       }
    }

    return retval;
}






uint32_t FeedHandler::GetIndexForSecurityID(uint32_t securityID, uint32_t* pIndex)
{
    uint32_t retval = XLNX_OK;
    bool bFound = false;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckSecurityID(securityID);
    }


    if (retval == XLNX_OK)
    {

        for (uint32_t i = 0; i < MAX_NUM_SECURITIES; i++)
        {
            if (m_securityIDLookup[i] == securityID)
            {
                bFound = true;
                *pIndex = i;
                break; //out of loop
            }
        }


        if (bFound == false)
        {
            retval = XLNX_FEED_HANDLER_ERROR_SECURITY_ID_DOES_NOT_EXIST;
        }
    }



    return retval;
}




uint32_t FeedHandler::InternalGetFreeSecurityIndex(uint32_t* pIndex)
{
    uint32_t retval = XLNX_OK;
    bool bFound = false;

    for (uint32_t i = 0; i < MAX_NUM_SECURITIES; i++)
    {
        if (m_securityIDLookup[i] == EMPTY_SECURITY_ID)
        {
            bFound = true;
            *pIndex = i;
            break; //out of loop
        }
    }

    if (bFound == false)
    {
        retval = XLNX_FEED_HANDLER_ERROR_NO_FREE_SECURITY_INDEX;
    }

    return retval;
}




uint32_t FeedHandler::FreezeData(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 31;

    //set the capture-freeze bit
    value = 1;

    value = value << shift;
    mask = mask << shift;

    retval = WriteRegWithMask32(XLNX_FEED_HANDLER_CAPTURE_FREEZE_OFFSET, value, mask);

    return retval;
}




uint32_t FeedHandler::UnfreezeData(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 31;

    //clear the capture-freeze bit
    value = 0;

    value = value << shift;
    mask = mask << shift;

    retval = WriteRegWithMask32(XLNX_FEED_HANDLER_CAPTURE_FREEZE_OFFSET, value, mask);


    return retval;
}



uint32_t FeedHandler::ReadData(FeedData* pData)
{
    uint32_t retval = XLNX_OK;
    uint32_t buffer[XLNX_FEED_HANDLER_NUM_CAPTURE_REGISTERS];

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        //Stop HW updates...
        retval = FreezeData();
    }

    if (retval == XLNX_OK)
    {
        //Take a snapshot of the data....
        retval = BlockReadReg32(XLNX_FEED_HANDLER_CAPTURE_DATA_REGISTER, buffer, XLNX_FEED_HANDLER_NUM_CAPTURE_REGISTERS);
    }



    if (retval == XLNX_OK)
    {
        //The following bitshift operations are due to several things:
        //
        //(1) The HW pushes its data out basically as a PACKED STRUCT.
        //(2) That struct has some 8-bit fields, which moves the next field onto a byte boundary (rather than a nice word boundary).
        //(3) the SW performs 32-bit reads on the AXI bus at even word boundaries (+0x00, +0x04, +0x08 and +0x0C)

        pData->level        = (buffer[0] & 0x000000FF);
        pData->side         = (OrderSide)((buffer[0] >> 8) & 0x000000FF);
        pData->price        = ((buffer[0] >> 16) & 0x0000FFFF) | ((buffer[1] << 16) & 0xFFFF0000);
        pData->quantity     = ((buffer[1] >> 16) & 0x0000FFFF) | ((buffer[2] << 16) & 0xFFFF0000);
        pData->count        = ((buffer[2] >> 16) & 0x0000FFFF) | ((buffer[3] << 16) & 0xFFFF0000);
        pData->orderID      = ((buffer[3] >> 16) & 0x0000FFFF) | ((buffer[4] << 16) & 0xFFFF0000);
        pData->symbolIndex  = ((buffer[4] >> 16) & 0x000000FF);
        pData->operation    = (OrderOperation)((buffer[4] >> 24) & 0x000000FF); 
        pData->timestamp    = (buffer[5] | (((uint64_t)buffer[6]) << 32));
    }


    if (retval == XLNX_OK)
    {
        //Restart HW updates...
        retval = UnfreezeData();
    }

    return retval;
}