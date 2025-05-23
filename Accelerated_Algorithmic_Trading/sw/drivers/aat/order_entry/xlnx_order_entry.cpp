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


#include "xlnx_order_entry.h"
#include "xlnx_order_entry_address_map.h"
using namespace XLNX;


static const uint32_t IS_INITIALISED_MAGIC_NUMBER = 0xC672CBFE;




OrderEntry::OrderEntry()
{
    m_pDeviceInterface = nullptr;
    m_cuAddress = 0;
    m_cuIndex = 0;
    m_initialisedMagicNumber = 0;
}



OrderEntry::~OrderEntry()
{


}


uint32_t OrderEntry::Initialise(DeviceInterface* pDeviceInterface, const char* cuName)
{
    uint32_t retval = XLNX_OK;

    m_initialisedMagicNumber = 0;	//will be set to magic number if we successfully initialise...

    m_pDeviceInterface = pDeviceInterface;


    retval = m_pDeviceInterface->GetCUAddress(cuName, &m_cuAddress);  

    if (retval != XLNX_OK)
    {
        retval = XLNX_ORDER_ENTRY_ERROR_CU_NAME_NOT_FOUND;
    }



    if (retval == XLNX_OK)
    {
        
        retval = m_pDeviceInterface->GetCUIndex(cuName, &m_cuIndex);

        if (retval != XLNX_OK)
        {
            retval = XLNX_ORDER_ENTRY_ERROR_CU_NAME_NOT_FOUND;
        }
    }



    if (retval == XLNX_OK)
    {
        m_initialisedMagicNumber = IS_INITIALISED_MAGIC_NUMBER;
    }

    return retval;
}






uint32_t OrderEntry::CheckIsInitialised(void)
{
    uint32_t retval = XLNX_OK;

    if (m_initialisedMagicNumber != IS_INITIALISED_MAGIC_NUMBER)
    {
        retval = XLNX_ORDER_ENTRY_ERROR_NOT_INITIALISED;
    }

    return retval;
}







void OrderEntry::IsInitialised(bool* pbIsInitialised)
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







uint32_t OrderEntry::GetCUIndex(uint32_t* pCUIndex)
{
    uint32_t retval = XLNX_OK;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        *pCUIndex = m_cuIndex;
    }

    return retval;
}




uint32_t OrderEntry::GetCUAddress(uint64_t* pCUAddress)
{
    uint32_t retval = XLNX_OK;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        *pCUAddress = m_cuAddress;
    }

    return retval;
}













uint32_t OrderEntry::GetStats(OrderEntry::Stats* pStats)
{
    uint32_t retval = XLNX_OK;



    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = FreezeStats();
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_ENTRY_STATS_RX_OPERATIONS_COUNT_OFFSET, &pStats->numRxOperations);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_ENTRY_STATS_PROCESSED_OPERATIONS_COUNT_OFFSET, &pStats->numProcessedOperations);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_ENTRY_STATS_TX_DATA_FRAMES_COUNT_OFFSET, &pStats->numTxDataFrames);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_ENTRY_STATS_TX_META_FRAMES_COUNT_OFFSET, &pStats->numTxMetaFrames);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_ENTRY_STATS_TX_MESSAGES_COUNT_OFFSET, &pStats->numTxMessages);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_ENTRY_STATS_RX_DATA_FRAMES_COUNT_OFFSET, &pStats->numRxDataFrames);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_ENTRY_STATS_RX_META_FRAMES_COUNT_OFFSET, &pStats->numRxMetaFrames);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_ENTRY_STATS_CLOCK_TICK_EVENTS_COUNT_OFFSET, &pStats->numClockTickEvents);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_ENTRY_STATS_TX_DROPPED_MSG_COUNT_OFFSET, &pStats->numTxDroppedMessages);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_ENTRY_STATS_NOTIFICATIONS_RECEIVED_COUNT_OFFSET, &pStats->numNotificationsReceived);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_ORDER_ENTRY_STATS_READ_REQUESTS_SENT_COUNT_OFFSET, &pStats->numReadRequestsSent);
    }

    if (retval == XLNX_OK)
    {
        retval = UnfreezeStats();
    }

  
    return retval;
}












uint32_t OrderEntry::ResetStats(void)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;
    uint32_t shift = 2;
    uint32_t mask = 0x01;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        offset = XLNX_ORDER_ENTRY_CONTROL_OFFSET;
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









uint32_t OrderEntry::SetCaptureFilter(uint32_t symbolIndex)
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
            retval = XLNX_ORDER_ENTRY_ERROR_SYMBOL_INDEX_OUT_OF_RANGE;
        }
    }


    if (retval == XLNX_OK)
    {
        offset = XLNX_ORDER_ENTRY_CAPTURE_CONTROL_OFFSET;
        value = symbolIndex;
        mask = 0x000000FF;

        retval = WriteRegWithMask32(offset, value, mask);
    }

    return retval;
}








uint32_t OrderEntry::GetCaptureFilter(uint32_t* pSymbolIndex)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        if (pSymbolIndex == nullptr)
        {
            retval = XLNX_ORDER_ENTRY_ERROR_INVALID_PARAMETER;
        }
    }


    if (retval == XLNX_OK)
    {
        offset = XLNX_ORDER_ENTRY_CAPTURE_CONTROL_OFFSET;

        retval = ReadReg32(offset, &value);
    }



    if (retval == XLNX_OK)
    {
        *pSymbolIndex = value & 0x000000FF;
    }

    return retval;
}



















uint32_t OrderEntry::FreezeData(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 31;

    //set the capture-freeze bit
    value = 1;

    value = value << shift;
    mask = mask << shift;

    retval = WriteRegWithMask32(XLNX_ORDER_ENTRY_CAPTURE_CONTROL_OFFSET, value, mask);


    return retval;
}






uint32_t OrderEntry::UnfreezeData(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 31;

    //clear the capture-freeze bit
    value = 0;

    value = value << shift;
    mask = mask << shift;

    retval = WriteRegWithMask32(XLNX_ORDER_ENTRY_CAPTURE_CONTROL_OFFSET, value, mask);


    return retval;
}












uint32_t OrderEntry::FreezeStats(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 31;

    //set the stats-freeze bit
    value = 1;

    value = value << shift;
    mask = mask << shift;

    retval = WriteRegWithMask32(XLNX_ORDER_ENTRY_CONTROL_OFFSET, value, mask);


    return retval;
}






uint32_t OrderEntry::UnfreezeStats(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 31;

    //clear the stats-freeze bit
    value = 0;

    value = value << shift;
    mask = mask << shift;

    retval = WriteRegWithMask32(XLNX_ORDER_ENTRY_CONTROL_OFFSET, value, mask);


    return retval;
}












uint32_t OrderEntry::ReadReg32(uint64_t offset, uint32_t* value)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->ReadReg32(address, value);




    if (retval != XLNX_OK)
    {
        retval = XLNX_ORDER_ENTRY_ERROR_IO_FAILED;
    }

    return retval;
}






uint32_t OrderEntry::WriteReg32(uint64_t offset, uint32_t value)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->WriteReg32(address, value);


    if (retval != XLNX_OK)
    {
        retval = XLNX_ORDER_ENTRY_ERROR_IO_FAILED;
    }

    return retval;
}




uint32_t OrderEntry::WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->WriteRegWithMask32(address, value, mask);

    if (retval != XLNX_OK)
    {
        retval = XLNX_ORDER_ENTRY_ERROR_IO_FAILED;
    }

    return retval;
}






uint32_t OrderEntry::BlockReadReg32(uint64_t offset, uint32_t* buffer, uint32_t numWords)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->BlockReadReg32(address, buffer, numWords);


    if (retval != XLNX_OK)
    {
        retval = XLNX_ORDER_ENTRY_ERROR_IO_FAILED;
    }

    return retval;
}










uint32_t OrderEntry::ReadMessage(OrderEntryMessage* pMessage)
{
    uint32_t retval = XLNX_OK;
    static const uint32_t MESSAGE_LENGTH_32BIT_WORDS = (MAX_MESSAGE_LENGTH + (sizeof(uint32_t) - 1)) / sizeof(uint32_t); //round up to next nearest uint32_t
    static const uint32_t MESSAGE_LENGTH_64BIT_WORDS = MESSAGE_LENGTH_32BIT_WORDS / 2;
    uint32_t messageBuffer[MESSAGE_LENGTH_32BIT_WORDS];
    uint64_t offset;
    uint32_t i;
    uint32_t j;
    uint32_t shift;
    char c;
    uint32_t charOffset;

    uint64_t* p64BitBuffer;


    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        retval = FreezeData();
    }



    if (retval == XLNX_OK)
    {
        offset = XLNX_ORDER_ENTRY_CAPTURE_OFFSET;
        retval = BlockReadReg32(offset, messageBuffer, MESSAGE_LENGTH_32BIT_WORDS);
    }



    if (retval == XLNX_OK)
    {

        //The HW pushes the message data out in 64-bit words, but SW reads in 32-bit words...there is a little bit
        //of byte re-ordering to be done...

        p64BitBuffer = (uint64_t*)messageBuffer;

        charOffset = 0;

        for (i = 0; i < MESSAGE_LENGTH_64BIT_WORDS; i++)
        {
            for (j=0; j < sizeof(uint64_t); j++)
            {
                shift = 64 - ((j + 1) * 8);
                c = (char)((p64BitBuffer[i] >> shift) & 0xFF);


                //if the message contains any non-printable chars...replace them with something that is
                if (isprint(c) == 0)
                {
                    c = '.';
                }

                pMessage->message[charOffset] = c;
                charOffset++;
            }
        }

        pMessage->message[charOffset] = '\0'; //add string terminator...

    }



    if (retval == XLNX_OK)
    {
        retval = UnfreezeData();
    }


    return retval;
}









uint32_t OrderEntry::IsRunning(bool* pbIsRunning)
{
    uint32_t retval = XLNX_OK;
    uint32_t offset;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 0;

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        offset = XLNX_ORDER_ENTRY_CONTROL_OFFSET;

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





uint32_t OrderEntry::Connect(uint8_t ipAddrA, uint8_t ipAddrB, uint8_t ipAddrC, uint8_t ipAddrD, uint16_t port)
{
    uint32_t retval = XLNX_OK;
    bool bConnectionRequested = false;
    uint32_t value;


    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        //We will reject an attempt to reconnect (potentially with different IP/port) if we are already
        //connected - this means the user must call "disconnect" first before attempting to reconnect.

        retval = IsConnectionRequested(&bConnectionRequested);

        if (retval == XLNX_OK)
        {
            if (bConnectionRequested)
            {
                retval = XLNX_ORDER_ENTRY_ERROR_CONNECTION_ALREADY_REQUESTED;
            }
        }
    }


    if (retval == XLNX_OK)
    {
        value = 0;
        value |= ipAddrA << 24;
        value |= ipAddrB << 16;
        value |= ipAddrC << 8;
        value |= ipAddrD << 0;

        retval = WriteReg32(XLNX_ORDER_ENTRY_DESTINATION_IP_ADDRESS_OFFSET, value);
    }



    if (retval == XLNX_OK)
    {
        value = port;

        retval = WriteReg32(XLNX_ORDER_ENTRY_DESTINATION_PORT_OFFSET, value);
    }


    if (retval == XLNX_OK)
    {
        retval = SetConnectionRequestedState(true);
    }


    return retval;
}



uint32_t OrderEntry::Disconnect(void)
{
    uint32_t retval = XLNX_OK;
    bool bIsConnected = false;

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        retval = IsConnectionRequested(&bIsConnected);

        if (retval == XLNX_OK)
        {
            if (bIsConnected == false)
            {
                retval = XLNX_ORDER_ENTRY_ERROR_NOT_CONNECTED;
            }
        }

    }

    if (retval == XLNX_OK)
    {
        retval = SetConnectionRequestedState(false);
    }


    //On a disconnect, just to be tidy, we will also zero out the IP address and port registers...

    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_ORDER_ENTRY_DESTINATION_IP_ADDRESS_OFFSET, 0);
    }



    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_ORDER_ENTRY_DESTINATION_PORT_OFFSET, 0);
    }


    return retval;
}






uint32_t OrderEntry::Reconnect(void)
{
    uint32_t retval = XLNX_OK;
    bool bIsConnected = false;

    retval = CheckIsInitialised();


    if (retval == XLNX_OK)
    {
        //we want to only allow a "reconnect" to be performed when we are already connected
        //and therefore have a valid IP address and port number...

        retval = IsConnectionRequested(&bIsConnected);

        if (retval == XLNX_OK)
        {
            if (bIsConnected == false)
            {
                retval = XLNX_ORDER_ENTRY_ERROR_NOT_CONNECTED;
            }
        }
    }



    if (retval == XLNX_OK)
    {
        retval = SetConnectionRequestedState(false);
    }


    if (retval == XLNX_OK)
    {
        retval = SetConnectionRequestedState(true);
    }


    return retval;
}




uint32_t OrderEntry::GetConnectionDetails(uint8_t* ipAddrA, uint8_t* ipAddrB, uint8_t* ipAddrC, uint8_t* ipAddrD, uint16_t* port)
{
    uint32_t retval = XLNX_OK;
    bool bIsConnected = false;
    uint32_t value;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = IsConnectionRequested(&bIsConnected);

        if (retval == XLNX_OK)
        {
            if (bIsConnected == false)
            {
                retval = XLNX_ORDER_ENTRY_ERROR_NOT_CONNECTED;
            }
        }
    }


    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ORDER_ENTRY_DESTINATION_IP_ADDRESS_OFFSET, &value);
    }

    if (retval == XLNX_OK)
    {
        *ipAddrA = (value >> 24) & 0xFF;
        *ipAddrB = (value >> 16) & 0xFF;
        *ipAddrC = (value >> 8) & 0xFF;
        *ipAddrD = (value >> 0) & 0xFF;
    }


    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ORDER_ENTRY_DESTINATION_PORT_OFFSET, &value);
    }


    if (retval == XLNX_OK)
    {
        *port = value & 0x0000FFFF;
    }

    return retval;
}





uint32_t OrderEntry::IsConnectionRequested(bool* pbIsConnected)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 3;


    offset = XLNX_ORDER_ENTRY_CONTROL_OFFSET;

    retval = ReadReg32(offset, &value);

    if (retval == XLNX_OK)
    {
        value = (value >> shift) & mask;

        if (value != 0)
        {
            *pbIsConnected = true;
        }
        else
        {
            *pbIsConnected = false;
        }
    }

    return retval;
}





uint32_t OrderEntry::SetConnectionRequestedState(bool bConnect)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 3;


    offset = XLNX_ORDER_ENTRY_CONTROL_OFFSET;

    if (bConnect)
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

    return retval;
}







uint32_t OrderEntry::GetTxStatus(TxStatus* pTxStatus)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ORDER_ENTRY_TX_STATUS_OFFSET, &value);
    }

    if (retval == XLNX_OK)
    {
        pTxStatus->bConnectionEstablished   = (value >> 31) & 0x01;
        pTxStatus->errorCode                = (ConnectionErrorCode)((value >> 29) & 0x03);
        pTxStatus->sendBufferSpace          = (value & 0x1FFFFFFF);
    }

    return retval;
}





uint32_t OrderEntry::SetChecksumGeneration(bool bEnabled)
{
    uint32_t retval = XLNX_OK;
    uint32_t value = 0;
    uint32_t mask;
    uint32_t shift = 4;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        if (bEnabled)
        {
            value = 1 << shift;      
        }
      
        mask = 1 << shift;

        retval = WriteRegWithMask32(XLNX_ORDER_ENTRY_CONTROL_OFFSET, value, mask);
    }


    return retval;
}




uint32_t OrderEntry::GetChecksumGeneration(bool* pbEnabled)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t shift = 4;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_ORDER_ENTRY_CONTROL_OFFSET, &value);
    }

    if (retval == XLNX_OK)
    {
        *pbEnabled = (bool)((value >> shift) & 0x01);
    }


    return retval;
}