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


#ifndef XLNX_ORDER_BOOK_H
#define XLNX_ORDER_BOOK_H


#include <cstdint>

#include "xlnx_device_interface.h"

#include "xlnx_order_book_error_codes.h"




namespace XLNX
{




class OrderBook
{






public:
	OrderBook();
	virtual ~OrderBook();


public:
	uint32_t Initialise(DeviceInterface* pDeviceInterface, const char* cuName);



public:
	static const uint32_t NUM_LEVELS = 5;
	static const uint32_t MAX_NUM_SYMBOLS = 256;

	typedef struct
	{
		uint32_t askQuantity[NUM_LEVELS];
		uint32_t askPrice[NUM_LEVELS];
		uint32_t askCount[NUM_LEVELS];

		uint32_t bidQuantity[NUM_LEVELS];
		uint32_t bidPrice[NUM_LEVELS];
		uint32_t bidCount[NUM_LEVELS];

		uint8_t symbolIndex;

        uint64_t timestamp; 

	} OrderBookData;

    //NOTE - currently HW represents the timestamp field as a ap_uint<56>.  In SW, we will hold this as a uint64_t
    //       This means there is subtle differece in the size of the HW data struct compared to the SW data struct
    static const uint32_t HW_DATA_SIZE_BYTES = 1024 / 8; 


public:
    uint32_t SetCaptureFilter(uint32_t symbolIndex);
    uint32_t GetCaptureFilter(uint32_t* pSymbolIndex);

	uint32_t ReadData(OrderBookData* pData);





public:
    typedef struct
    {
        uint32_t numRxOperations;
        uint32_t numProcessedOperations;
        uint32_t numInvalidOperations;
        uint32_t numResponsesGenerated;
        uint32_t numResponsesSent;              //after filter applied
        uint32_t numAddOperations;
        uint32_t numModifyOperations;
        uint32_t numDeleteOperations;
        uint32_t numTransactOperations;
        uint32_t numHaltOperations;
        uint32_t numTimestampErrors;            //out of sequence check
        uint32_t numUnhandledOpCodes;
        uint32_t numSymbolErrors;               //symbol index outsode of supported range
        uint32_t numDirectionErrors;            //not BID or ASK
        uint32_t numLevelErrors;                //level outside of supported range
        uint32_t numClockTickGeneratorEvents;

    } Stats;

    uint32_t GetStats(Stats* pStats);
    uint32_t ResetStats(void);




public:
    uint32_t Start(void);
    uint32_t Stop(void);
    uint32_t IsRunning(bool* pbIsRunning);



public: //Orderbook Data Mover Output Stream Control
    uint32_t SetDataMoverOutput(bool bEnabled);
    uint32_t GetDataMoverOutput(bool* pbEnabled);


public:
    void IsInitialised(bool* pbIsInitialised);
	uint32_t GetCUIndex(uint32_t* pCUIndex);
	uint32_t GetCUAddress(uint64_t* pCUAddress);




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

	uint32_t BlockReadMem32(uint64_t address, uint32_t* buffer, uint32_t numWords);




protected:
	uint32_t m_initialisedMagicNumber;

	uint32_t m_cuIndex;
	uint64_t m_cuAddress;

	DeviceInterface* m_pDeviceInterface;


}; //class OrderBook



} //namespace XLNX



#endif //XLNX_ORDER_BOOK_H

