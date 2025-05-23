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


#ifndef XLNX_ORDER_ENTRY_H
#define XLNX_ORDER_ENTRY_H


#include <cstdint>

#include "xlnx_device_interface.h"

#include "xlnx_order_entry_error_codes.h"


namespace XLNX
{

class OrderEntry
{


public:
    OrderEntry();
    virtual ~OrderEntry();


public:
    uint32_t Initialise(DeviceInterface* pDeviceInterface, const char* cuName);



public:
    typedef struct
    {
        uint32_t numRxOperations;
        uint32_t numProcessedOperations;
        uint32_t numTxDataFrames;
        uint32_t numTxMetaFrames;
        uint32_t numTxMessages;
        uint32_t numRxDataFrames;
        uint32_t numRxMetaFrames;

        uint32_t numClockTickEvents;
        
        uint32_t numTxDroppedMessages; //Message dropped due to connection being unavailable

        uint32_t numNotificationsReceived;  //from TCP kernel
        uint32_t numReadRequestsSent;       //to TCP kernel
      
    } Stats;

    uint32_t GetStats(Stats* pStats);
    uint32_t ResetStats(void);






public:

    static const uint32_t MAX_MESSAGE_LENGTH = 128;

    typedef struct
    {
        char message[MAX_MESSAGE_LENGTH+1];    //+1 to terminate with NULL character

    }OrderEntryMessage;

    uint32_t ReadMessage(OrderEntryMessage* pMessage);





public:
    static const uint32_t MAX_NUM_SYMBOLS = 256;

    uint32_t SetCaptureFilter(uint32_t symbolIndex);
    uint32_t GetCaptureFilter(uint32_t* pSymbolIndex);




public: //Connection Control
    uint32_t Connect(uint8_t ipAddrA, uint8_t ipAddrB, uint8_t ipAddrC, uint8_t ipAddrD, uint16_t port);
    uint32_t Disconnect(void);
    uint32_t Reconnect(void); //aka "disconnect" followed by "connect" using same details as previously

    uint32_t IsConnectionRequested(bool* pbConnectionRequested);
    uint32_t GetConnectionDetails(uint8_t* ipAddrA, uint8_t* ipAddrB, uint8_t* ipAddrC, uint8_t* ipAddrD, uint16_t* port);


   


public: //Connection Status

    enum class ConnectionErrorCode
    {
        SUCCESS = 0,
        CLOSED = 1,
        OUT_OF_SPACE = 2,
    };

    typedef struct
    {
        bool bConnectionEstablished;
        ConnectionErrorCode errorCode;
        uint32_t sendBufferSpace;

    }TxStatus;


    uint32_t GetTxStatus(TxStatus* pTxStatus);




public: //Partial Checksum Calculation And Forwarding

    //To help to reduce latency, the order entry block can calculate a partial checkum for the outgoing message
    //and forward this to the TCP block.  The following functions control this behaviour:
    uint32_t SetChecksumGeneration(bool bEnabled);
    uint32_t GetChecksumGeneration(bool* pbEnabled);



public:
    void IsInitialised(bool* pbIsInitialised);

    uint32_t GetCUIndex(uint32_t* pCUIndex);
    uint32_t GetCUAddress(uint64_t* pCUAddress);

    uint32_t IsRunning(bool* pbIsRunning);





protected:
    uint32_t CheckIsInitialised(void);

    // the follow functions provide a form of mutual exclusion to the order book data.
    // i.e they enable/disable HW updates to the data registers.
    uint32_t FreezeData(void);
    uint32_t UnfreezeData(void);

    uint32_t FreezeStats(void);
    uint32_t UnfreezeStats(void);



protected:
    uint32_t ReadReg32(uint64_t offset, uint32_t* value);
    uint32_t WriteReg32(uint64_t offset, uint32_t value);
    uint32_t WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask);
    uint32_t BlockReadReg32(uint64_t offset, uint32_t* buffer, uint32_t numWords);



protected:
    uint32_t SetConnectionRequestedState(bool bConnect);

protected:
    uint32_t m_initialisedMagicNumber;
    uint64_t m_cuAddress;
    uint32_t m_cuIndex;
    DeviceInterface* m_pDeviceInterface;

};



} //end namespace XLNX




#endif //XLNX_ORDER_ENTRY_H

