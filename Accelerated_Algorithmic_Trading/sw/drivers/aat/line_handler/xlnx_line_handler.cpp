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

#include <string.h> //for memset

#include "xlnx_line_handler.h"
#include "xlnx_line_handler_address_map.h"
using namespace XLNX;


static const uint32_t IS_INITIALISED_MAGIC_NUMBER = 0x19CAA610;




LineHandler::LineHandler()
{
    m_pDeviceInterface = nullptr;
    m_cuAddress = 0;
    m_cuIndex = 0;
    m_initialisedMagicNumber = 0;
    m_clockFrequencyMHz = 0;
}



LineHandler::~LineHandler()
{


}


uint32_t LineHandler::Initialise(DeviceInterface* pDeviceInterface, const char* cuName)
{
    uint32_t retval = XLNX_OK;
    uint32_t clockFrequencyArray[DeviceInterface::MAX_SUPPORTED_CLOCKS];
    uint32_t numClocks;

    m_initialisedMagicNumber = 0;	//will be set to magic number if we successfully initialise...

    m_pDeviceInterface = pDeviceInterface;

   
    retval = m_pDeviceInterface->GetCUAddress(cuName, &m_cuAddress);

    if (retval != XLNX_OK)
    {
        retval = XLNX_LINE_HANDLER_ERROR_CU_NAME_NOT_FOUND;
    }



    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->GetCUIndex(cuName, &m_cuIndex);

        if (retval != XLNX_OK)
        {
            retval = XLNX_LINE_HANDLER_ERROR_CU_NAME_NOT_FOUND;
        }
    }


    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->GetClocks(clockFrequencyArray, &numClocks);
        if (retval == XLNX_OK)
        {
            m_clockFrequencyMHz = clockFrequencyArray[0]; //NOTE - assuming our clock is the first one in the list
        }
    }


    if (retval == XLNX_OK)
    {
        m_initialisedMagicNumber = IS_INITIALISED_MAGIC_NUMBER;
    }

    return retval;
}






uint32_t LineHandler::CheckIsInitialised(void)
{
    uint32_t retval = XLNX_OK;

    if (m_initialisedMagicNumber != IS_INITIALISED_MAGIC_NUMBER)
    {
        retval = XLNX_LINE_HANDLER_ERROR_NOT_INITIALISED;
    }

    return retval;
}







void LineHandler::IsInitialised(bool* pbIsInitialised)
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







uint32_t LineHandler::GetCUIndex(uint32_t* pCUIndex)
{
    uint32_t retval = XLNX_OK;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        *pCUIndex = m_cuIndex;
    }

    return retval;
}




uint32_t LineHandler::GetCUAddress(uint64_t* pCUAddress)
{
    uint32_t retval = XLNX_OK;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        *pCUAddress = m_cuAddress;
    }

    return retval;
}



uint32_t LineHandler::ReadReg32(uint64_t offset, uint32_t* value)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->ReadReg32(address, value);




    if (retval != XLNX_OK)
    {
        retval = XLNX_LINE_HANDLER_ERROR_IO_FAILED;
    }

    return retval;
}






uint32_t LineHandler::WriteReg32(uint64_t offset, uint32_t value)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->WriteReg32(address, value);


    if (retval != XLNX_OK)
    {
        retval = XLNX_LINE_HANDLER_ERROR_IO_FAILED;
    }

    return retval;
}




uint32_t LineHandler::WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->WriteRegWithMask32(address, value, mask);

    if (retval != XLNX_OK)
    {
        retval = XLNX_LINE_HANDLER_ERROR_IO_FAILED;
    }

    return retval;
}






uint32_t LineHandler::BlockReadReg32(uint64_t offset, uint32_t* buffer, uint32_t numWords)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->BlockReadReg32(address, buffer, numWords);


    if (retval != XLNX_OK)
    {
        retval = XLNX_LINE_HANDLER_ERROR_IO_FAILED;
    }

    return retval;
}





uint32_t LineHandler::CheckInputPort(uint32_t inputPort)
{
    uint32_t retval = XLNX_OK;

    if (inputPort >= NUM_SUPPORTED_INPUT_PORTS)
    {
        retval = XLNX_LINE_HANDLER_ERROR_INVALID_INPUT_PORT;
    }

    return retval;
}





uint32_t LineHandler::CheckIPAddress(uint8_t ipAddrA, uint8_t ipAddrB, uint8_t ipAddrC, uint8_t ipAddrD)
{
    uint32_t retval = XLNX_OK;

    if ((ipAddrA == 0) && (ipAddrB == 0) && (ipAddrC == 0) && (ipAddrD == 0))
    {
        retval = XLNX_LINE_HANDLER_ERROR_INVALID_IP_ADDRESS;
    }

    return retval;
}




uint32_t LineHandler::CheckPort(uint16_t port)
{
    uint32_t retval = XLNX_OK;

    if (port == 0)
    {
        retval = XLNX_LINE_HANDLER_ERROR_INVALID_PORT;
    }

    return retval;
}





uint32_t LineHandler::CheckSplitID(uint32_t splitID)
{
    uint32_t retval = XLNX_OK;

    if (splitID >= NUM_SUPPORTED_SPLITS)
    {
        retval = XLNX_LINE_HANDLER_ERROR_INVALID_SPLIT_ID;
    }

    return retval;
}







uint32_t LineHandler::SetEchoEnabled(uint32_t inputPort, bool bEnabled)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 3;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckInputPort(inputPort);
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

        retval = WriteRegWithMask32(XLNX_LINE_HANDLER_RESET_CONTROL_OFFSET(inputPort), value, mask);

    }

    return retval;
}







uint32_t LineHandler::GetEchoEnabled(uint32_t inputPort, bool* pbEnabled)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask = 0x01;
    uint32_t shift = 3;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckInputPort(inputPort);
    }

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_LINE_HANDLER_RESET_CONTROL_OFFSET(inputPort), &value);
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







uint32_t LineHandler::SetEchoDestination(uint32_t inputPort, uint8_t ipAddrA, uint8_t ipAddrB, uint8_t ipAddrC, uint8_t ipAddrD, uint16_t port)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckInputPort(inputPort);
    }

    if (retval == XLNX_OK)
    {
        value = 0;
        value |= ipAddrA << 24;
        value |= ipAddrB << 16;
        value |= ipAddrC << 8;
        value |= ipAddrD << 0;

        retval = WriteReg32(XLNX_LINE_HANDLER_ECHO_DESTINATION_IP_ADDRESS_OFFSET(inputPort), value);
    }



    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_LINE_HANDLER_ECHO_DESTINATION_PORT_OFFSET(inputPort), port);
    }


    return retval;
}










uint32_t LineHandler::GetEchoDestination(uint32_t inputPort, uint8_t* ipAddrA, uint8_t* ipAddrB, uint8_t* ipAddrC, uint8_t* ipAddrD, uint16_t* port)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckInputPort(inputPort);
    }

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_LINE_HANDLER_ECHO_DESTINATION_IP_ADDRESS_OFFSET(inputPort), &value);
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
        retval = ReadReg32(XLNX_LINE_HANDLER_ECHO_DESTINATION_PORT_OFFSET(inputPort), &value);
    }

    if (retval == XLNX_OK)
    {
        *port = value & 0x0000FFFF;
    }

    return retval;
}






uint32_t LineHandler::GetStats(Stats* pStats)
{
    uint32_t retval = XLNX_OK;

    memset(pStats, 0, sizeof(*pStats));

    retval = CheckIsInitialised();




    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_LINE_HANDLER_STATS_RX_WORDS_COUNT_0_OFFSET, &pStats->inputPortStats[0].numRxWords);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_LINE_HANDLER_STATS_RX_META_COUNT_0_OFFSET, &pStats->inputPortStats[0].numRxMeta);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_LINE_HANDLER_STATS_DROPPED_WORDS_COUNT_0_OFFSET, &pStats->inputPortStats[0].numDroppedWords);
    }






    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_LINE_HANDLER_STATS_RX_WORDS_COUNT_1_OFFSET, &pStats->inputPortStats[1].numRxWords);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_LINE_HANDLER_STATS_RX_META_COUNT_1_OFFSET, &pStats->inputPortStats[1].numRxMeta);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_LINE_HANDLER_STATS_DROPPED_WORDS_COUNT_1_OFFSET, &pStats->inputPortStats[1].numDroppedWords);
    }



    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_LINE_HANDLER_STATS_TOTAL_PACKETS_SENT_COUNT_OFFSET, &pStats->totalPacketsSent);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_LINE_HANDLER_STATS_TOTAL_WORDS_SENT_COUNT_OFFSET, &pStats->totalWordsSent);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_LINE_HANDLER_STATS_TOTAL_PACKETS_MISSED_COUNT_OFFSET, &pStats->totalPacketsMissed);
    }







    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_LINE_HANDLER_STATS_RX_PACKETS_COUNT_0_OFFSET, &pStats->inputPortStats[0].numRxPackets);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_LINE_HANDLER_STATS_RX_PACKETS_COUNT_1_OFFSET, &pStats->inputPortStats[1].numRxPackets);
    }





    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_LINE_HANDLER_STATS_ARBITRATED_TX_PACKETS_COUNT_0_OFFSET, &pStats->inputPortStats[0].numArbitratedTxPackets);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_LINE_HANDLER_STATS_ARBITRATED_TX_PACKETS_COUNT_1_OFFSET, &pStats->inputPortStats[1].numArbitratedTxPackets);
    }





    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_LINE_HANDLER_STATS_DISCARDED_PACKETS_COUNT_0_OFFSET, &pStats->inputPortStats[0].numDiscardedPackets);
    }

    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_LINE_HANDLER_STATS_DISCARDED_PACKETS_COUNT_1_OFFSET, &pStats->inputPortStats[1].numDiscardedPackets);
    }





    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->ReadReg32(m_cuAddress + XLNX_LINE_HANDLER_STATS_CLOCK_TICK_EVENTS_COUNT_OFFSET, &pStats->clockTickEvents);
    }



    return retval;
    
}




uint32_t LineHandler::SetSequenceResetTimer(uint32_t microseconds)
{
    uint32_t retval = XLNX_OK;
    uint32_t clockCycles = 0;
    
    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = ConvertMicrosecondsToClockCycles(microseconds, &clockCycles);
    }

    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_LINE_HANDLER_SEQUENCE_RESET_TIMER_OFFSET, clockCycles);
    }

    return retval;
}





uint32_t LineHandler::GetSequenceResetTimer(uint32_t* pMicroseconds)
{
    uint32_t retval = XLNX_OK;
    uint32_t clockCycles = 0;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_LINE_HANDLER_SEQUENCE_RESET_TIMER_OFFSET, &clockCycles);
    }

    if (retval == XLNX_OK)
    {
        retval = ConvertClockCyclesToMicroseconds(clockCycles, pMicroseconds);
    }


    return retval;
}








uint32_t LineHandler::ResetSequenceNumber(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t mask = 0x00000001;

    retval = CheckIsInitialised();

    //Here we will toggle the bit (write it to 1, then write it to 0)

    if (retval == XLNX_OK)
    {
        retval = WriteRegWithMask32(XLNX_LINE_HANDLER_RESET_SEQUENCE_NUMBER_OFFSET, 1, mask);
    }

    if (retval == XLNX_OK)
    {
        retval = WriteRegWithMask32(XLNX_LINE_HANDLER_RESET_SEQUENCE_NUMBER_OFFSET, 0, mask);
    }

    return retval;
}





uint32_t LineHandler::ConvertMicrosecondsToClockCycles(uint32_t microseconds, uint32_t* pClockCycles)
{
    uint32_t retval = XLNX_OK;
    uint64_t cyclesPerMicrosecond;
    uint64_t clockCycles;

    cyclesPerMicrosecond = m_clockFrequencyMHz;

    clockCycles = cyclesPerMicrosecond * microseconds;


    //The HW register is a 32-bit register - we need to check that our calculated number of clock cycles doesn't
    //exceed the 32-bit range.
    if (clockCycles > 0xFFFFFFFF)
    {
        retval = XLNX_LINE_HANDLER_ERROR_TIMER_INTERVAL_TOO_LARGE;
    }

    if (retval == XLNX_OK)
    {
        *pClockCycles = (uint32_t)clockCycles;
    }

    return retval;
}



uint32_t LineHandler::ConvertClockCyclesToMicroseconds(uint32_t clockCycles, uint32_t* pMicroseconds)
{
    uint32_t retval = XLNX_OK;
    uint32_t cyclesPerMicrosecond;

    cyclesPerMicrosecond = m_clockFrequencyMHz ;

    //Here we are rounding up or down to the nearest whole number of microseconds...
    *pMicroseconds = (uint32_t)(((double)clockCycles / (double)cyclesPerMicrosecond) + 0.5);

    return retval;
}






uint32_t LineHandler::Add(uint32_t inputPort, uint8_t ipAddrA, uint8_t ipAddrB, uint8_t ipAddrC, uint8_t ipAddrD, uint16_t port, uint32_t splitID)
{
    uint32_t retval = XLNX_OK;
    uint32_t slot = 0;


    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckInputPort(inputPort);
    }

    if (retval == XLNX_OK)
    {
        retval = CheckIPAddress(ipAddrA, ipAddrB, ipAddrC, ipAddrD);
    }

    if (retval == XLNX_OK)
    {
        retval = CheckPort(port);
    }

    if (retval == XLNX_OK)
    {
        retval = CheckSplitID(splitID);
    }


    if (retval == XLNX_OK)
    {
        //Need to check to see if filter details have already been previously added...
        retval = FindFilterIndex(inputPort, ipAddrA, ipAddrB, ipAddrC, ipAddrD, port, splitID, &slot);
        if (retval == XLNX_OK)
        {
            //Found a matching filter...but we don't allow duplicates
            retval = XLNX_LINE_HANDLER_ERROR_FILTER_ALREADY_EXISTS;
        }
        else if (retval == XLNX_LINE_HANDLER_ERROR_FILTER_DOES_NOT_EXIST)
        {
            //No matching existing filter...we're OK to add a new one.
            retval = XLNX_OK;
        }
    }


    if (retval == XLNX_OK)
    {
        retval = FindFreeFilterIndex(inputPort, &slot);
    }


    if (retval == XLNX_OK)
    {
        retval = SetFilterDetails(inputPort, slot, ipAddrA, ipAddrB, ipAddrC, ipAddrD, port, splitID);
    }

    return retval;
}










uint32_t LineHandler::Delete(uint32_t inputPort, uint8_t ipAddrA, uint8_t ipAddrB, uint8_t ipAddrC, uint8_t ipAddrD, uint16_t port, uint32_t splitID)
{
    uint32_t retval = XLNX_OK;
    uint32_t slot = 0;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckInputPort(inputPort);
    }

    if (retval == XLNX_OK)
    {
        retval = CheckIPAddress(ipAddrA, ipAddrB, ipAddrC, ipAddrD);
    }

    if (retval == XLNX_OK)
    {
        retval = CheckPort(port);
    }

    if (retval == XLNX_OK)
    {
        retval = CheckSplitID(splitID);
    }


    if (retval == XLNX_OK)
    {
        retval = FindFilterIndex(inputPort, ipAddrA, ipAddrB, ipAddrC, ipAddrD, port, splitID, &slot);
    }

    if (retval == XLNX_OK)
    {
        //zero-out the slot details...
        retval = SetFilterDetails(inputPort, slot, 0, 0, 0, 0, 0, 0);
    }


    return retval;
}






uint32_t LineHandler::DeleteAll(uint32_t inputPort)
{
    uint32_t retval = XLNX_OK;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        for (uint32_t i = 0; i < NUM_SUPPORTED_FILTERS_PER_INPUT_PORT; i++)
        {
            retval = SetFilterDetails(inputPort, i, 0, 0, 0, 0, 0, 0);

            if (retval != XLNX_OK)
            {
                break; //out of loop
            }
        }
    }

    return retval;
}







uint32_t LineHandler::FindFreeFilterIndex(uint32_t inputPort, uint32_t* pFreeIndex)
{
    uint32_t retval = XLNX_OK;
    bool bSlotUsed;
    bool bFoundFreeIndex = false;

    for (uint32_t i = 0; i < NUM_SUPPORTED_FILTERS_PER_INPUT_PORT; i++)
    {
        retval = FilterIndexIsUsed(inputPort, i, &bSlotUsed);
        if (retval == XLNX_OK)
        {
            if (bSlotUsed == false)
            {
                bFoundFreeIndex = true;
                *pFreeIndex = i;
                break; //out of loop
            }
        }
        else
        {
            break; //out of loop
        }
    }



    if (bFoundFreeIndex == false)
    {
        retval = XLNX_LINE_HANDLER_ERROR_NO_FREE_FILTER_SLOTS;
    }




    return retval;
}





uint32_t LineHandler::FindFilterIndex(uint32_t inputPort, uint8_t ipAddrA, uint8_t ipAddrB, uint8_t ipAddrC, uint8_t ipAddrD, uint16_t port, uint32_t splitID, uint32_t* pIndex)
{
    uint32_t retval = XLNX_OK;
    bool bSlotUsed;
    bool bFoundMatch = false;
    uint8_t slotIPAddrA;
    uint8_t slotIPAddrB;
    uint8_t slotIPAddrC;
    uint8_t slotIPAddrD;
    uint16_t slotPort;
    uint32_t slotSplitID;

    for (uint32_t i = 0; i < NUM_SUPPORTED_FILTERS_PER_INPUT_PORT; i++)
    {
        retval = FilterIndexIsUsed(inputPort, i, &bSlotUsed);

        if (retval == XLNX_OK)
        {
            if (bSlotUsed)
            {
                retval = GetFilterDetails(inputPort, i, &slotIPAddrA, &slotIPAddrB, &slotIPAddrC, &slotIPAddrD, &slotPort, &slotSplitID);

                if (retval == XLNX_OK)
                {
                    if ((slotIPAddrA == ipAddrA) &&
                        (slotIPAddrB == ipAddrB) &&
                        (slotIPAddrC == ipAddrC) &&
                        (slotIPAddrD == ipAddrD) &&
                        (slotPort == port)       &&
                        (slotSplitID == splitID))
                    {                   
                        bFoundMatch = true;
                        *pIndex = i;
                    }
                }
            }
        }
    }


    if (bFoundMatch == false)
    {
        retval = XLNX_LINE_HANDLER_ERROR_FILTER_DOES_NOT_EXIST;
    }

    return retval;
}



uint32_t LineHandler::FilterIndexIsUsed(uint32_t inputPort, uint32_t index, bool* pbIsUsed)
{
    uint32_t retval = XLNX_OK;
    uint64_t offset;
    uint32_t value;


    //To determine whether or not a partiular slot is used, we will look at the ADDRESS field
    //If it is NON-ZERO, then we will assume the slot is in use...

    offset = XLNX_LINE_HANDLER_FILTER_SLOT_ADDRESS_OFFSET(inputPort, index);

    retval = ReadReg32(offset, &value);

    if (retval == XLNX_OK)
    {
        if (value != 0)
        {
            *pbIsUsed = true;
        }
        else
        {
            *pbIsUsed = false;
        }
    }



    return retval;
}




uint32_t LineHandler::SetFilterDetails(uint32_t inputPort, uint32_t index, uint8_t ipAddrA, uint8_t ipAddrB, uint8_t ipAddrC, uint8_t ipAddrD, uint16_t port, uint32_t splitID)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;


    value = 0;
    value |= ipAddrA << 24;
    value |= ipAddrB << 16;
    value |= ipAddrC << 8;
    value |= ipAddrD << 0;

    retval = WriteReg32(XLNX_LINE_HANDLER_FILTER_SLOT_ADDRESS_OFFSET(inputPort, index), value);
    


    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_LINE_HANDLER_FILTER_SLOT_PORT_OFFSET(inputPort, index), (uint32_t)port);
    }


    if (retval == XLNX_OK)
    {
        retval = WriteReg32(XLNX_LINE_HANDLER_FILTER_SLOT_SPLIT_ID_OFFSET(inputPort, index), splitID);
    }

    return retval;
}







uint32_t LineHandler::GetFilterDetails(uint32_t inputPort, uint32_t index, uint8_t* pIPAddrA, uint8_t* pIPAddrB, uint8_t* pIPAddrC, uint8_t* pIPAddrD, uint16_t* pPort, uint32_t* pSplitID)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;


    retval = ReadReg32(XLNX_LINE_HANDLER_FILTER_SLOT_ADDRESS_OFFSET(inputPort, index), &value);
    if (retval == XLNX_OK)
    {
        *pIPAddrA = (value >> 24) & 0xFF;
        *pIPAddrB = (value >> 16) & 0xFF;
        *pIPAddrC = (value >> 8) & 0xFF;
        *pIPAddrD = (value >> 0) & 0xFF;
    }
    


    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_LINE_HANDLER_FILTER_SLOT_PORT_OFFSET(inputPort, index), &value);
        if (retval == XLNX_OK)
        {
            *pPort = (uint16_t)(value & 0xFFFF);
        }
    }


    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_LINE_HANDLER_FILTER_SLOT_SPLIT_ID_OFFSET(inputPort, index), &value);
        if (retval == XLNX_OK)
        {
            *pSplitID = value;
        }
    }

    return retval;
}


