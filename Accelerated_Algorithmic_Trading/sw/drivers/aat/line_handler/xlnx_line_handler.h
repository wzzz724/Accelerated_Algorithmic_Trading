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


#ifndef XLNX_LINE_HANDLER_H
#define XLNX_LINE_HANDLER_H


#include <cstdint>

#include "xlnx_device_interface.h"

#include "xlnx_line_handler_error_codes.h"


namespace XLNX
{

class LineHandler
{


public:
    LineHandler();
    virtual ~LineHandler();


public:
    uint32_t Initialise(DeviceInterface* pDeviceInterface, const char* cuName);

    
  

public: //Port Filters
    static const uint32_t NUM_SUPPORTED_INPUT_PORTS = 2;
    static const uint32_t NUM_SUPPORTED_FILTERS_PER_INPUT_PORT = 16;
    static const uint32_t NUM_SUPPORTED_SPLITS = 8;

    uint32_t Add(uint32_t inputPort, uint8_t ipAddrA, uint8_t ipAddrB, uint8_t ipAddrC, uint8_t ipAddrD, uint16_t port, uint32_t splitID);
    uint32_t Delete(uint32_t inputPort, uint8_t ipAddrA, uint8_t ipAddrB, uint8_t ipAddrC, uint8_t ipAddrD, uint16_t port, uint32_t splitID);
    uint32_t DeleteAll(uint32_t inputPort);






public:
    uint32_t SetSequenceResetTimer(uint32_t microseconds);
    uint32_t GetSequenceResetTimer(uint32_t* pMicroseconds);


    //The following function allows the user to manually reset the next expected sequence number back to 0.
    uint32_t ResetSequenceNumber(void);


public: //Stats
    
    typedef struct _InputPortStats
    {
        uint32_t numRxWords;
        uint32_t numRxMeta;
        uint32_t numDroppedWords;

        uint32_t numRxPackets;
        uint32_t numArbitratedTxPackets;
        uint32_t numDiscardedPackets;

    }InputPortStats;





    typedef struct
    {
        InputPortStats inputPortStats[NUM_SUPPORTED_INPUT_PORTS];

        uint32_t totalPacketsSent;
        uint32_t totalWordsSent;
        uint32_t totalPacketsMissed;

        uint32_t clockTickEvents;
    }Stats;



    uint32_t GetStats(Stats* pStats);







public: //Debug

    //To assist with debugging, the received and processed data can be echoed back via UDP to a specified destination IP address

    //The following function controls whether echoing is enabled
    uint32_t SetEchoEnabled(uint32_t inputPort, bool bEnabled);
    uint32_t GetEchoEnabled(uint32_t inputPort, bool* pbEnabled);

    //The following function sets the destination IP address and port where the echoed data will be sent
    uint32_t SetEchoDestination(uint32_t inputPort, uint8_t ipAddrA, uint8_t ipAddrB, uint8_t ipAddrC, uint8_t ipAddrD, uint16_t port);
    uint32_t GetEchoDestination(uint32_t inputPort, uint8_t* ipAddrA, uint8_t* ipAddrB, uint8_t* ipAddrC, uint8_t* ipAddrD, uint16_t* port);





public:
    void IsInitialised(bool* pbIsInitialised);

    uint32_t GetCUIndex(uint32_t* pCUIndex);
    uint32_t GetCUAddress(uint64_t* pCUAddress);




protected:
    uint32_t CheckIsInitialised(void);




protected:
    uint32_t ReadReg32(uint64_t offset, uint32_t* value);
    uint32_t WriteReg32(uint64_t offset, uint32_t value);
    uint32_t WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask);
    uint32_t BlockReadReg32(uint64_t offset, uint32_t* buffer, uint32_t numWords);




protected:
    uint32_t CheckInputPort(uint32_t inputPort);
    uint32_t CheckIPAddress(uint8_t ipAddrA, uint8_t ipAddrB, uint8_t ipAddrC, uint8_t ipAddrD);
    uint32_t CheckPort(uint16_t port);
    uint32_t CheckSplitID(uint32_t splitID);



public:
    uint32_t FilterIndexIsUsed(uint32_t inputPort, uint32_t index, bool* pbIsUsed);
    uint32_t GetFilterDetails(uint32_t inputPort, uint32_t index, uint8_t* pIPAddrA, uint8_t* pIPAddrB, uint8_t* pIPAddrC, uint8_t* pIPAddrD, uint16_t* pPort, uint32_t* pSplitID);

protected:
    uint32_t FindFreeFilterIndex(uint32_t inputPort, uint32_t* pFreeIndex);
    uint32_t FindFilterIndex(uint32_t inputPort, uint8_t ipAddrA, uint8_t ipAddrB, uint8_t ipAddrC, uint8_t ipAddrD, uint16_t port, uint32_t splitID, uint32_t* pIndex);
   
    uint32_t SetFilterDetails(uint32_t inputPort, uint32_t index, uint8_t ipAddrA, uint8_t ipAddrB, uint8_t ipAddrC, uint8_t ipAddrD, uint16_t port, uint32_t splitID);
   

protected:
    uint32_t ConvertMicrosecondsToClockCycles(uint32_t microseconds, uint32_t* pClockCycles);
    uint32_t ConvertClockCyclesToMicroseconds(uint32_t clockCycles, uint32_t* pMicroseconds);


protected:
    uint32_t m_initialisedMagicNumber;
    uint64_t m_cuAddress;
    uint32_t m_cuIndex;
    DeviceInterface* m_pDeviceInterface;

    uint32_t m_clockFrequencyMHz;
};



} //end namespace XLNX




#endif //XLNX_LINE_HANDLER_H

