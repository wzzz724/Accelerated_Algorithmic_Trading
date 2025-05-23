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


#ifndef XLNX_FEED_HANDLER_H
#define XLNX_FEED_HANDLER_H


#include <cstdint>

#include "xlnx_device_interface.h"


#include "xlnx_feed_handler_error_codes.h"




namespace XLNX
{

class FeedHandler
{


public:
	FeedHandler();
	virtual ~FeedHandler();


public:
	uint32_t Initialise(DeviceInterface* pDeviceInterface, const char* cuName);



public: //Security ID Control
	static const uint32_t MAX_NUM_SECURITIES = 256;

	//The following function can be used to add a security ID at the FIRST AVAILABLE INDEX
	//If successfully added, the index at which the security was added in placed into the locaton pointed to by pIndex.
	//This index is then used by the other blocks (e.g. order book) to refer to this security.
	uint32_t AddSecurity(uint32_t securityID, uint32_t* pIndex);

	//The following function can be used to add a securityID at a SPECIFIC INDEX
	//NOTE - will fail if a security already exists at that index
	uint32_t AddSecurityAtIndex(uint32_t securityID, uint32_t index);


	//The following functions can be used to remove securities...
	uint32_t RemoveSecurity(uint32_t securityID);
    uint32_t RemoveAllSecurities(void);


	uint32_t RefreshCache(void);




public:
    void GetNumSecurities(uint32_t* pNumSecurities);

    uint32_t GetSecurityIDAtIndex(uint32_t index, uint32_t* pSecurityID);
    uint32_t GetIndexForSecurityID(uint32_t securityID, uint32_t* pIndex);


public: //the following function is  for use by the GUI...
    uint32_t GetSecurityAtContiguousIndex(uint32_t contiguousIndex, uint32_t* pSecurityIndex, uint32_t* pSecurityID);





public: //Capture Data

	typedef enum
	{
		ORDER_SIDE_BID = 0,
		ORDER_SIDE_ASK = 1

	}OrderSide;



	typedef enum
	{
		ORDER_OPERATION_ADD = 0,
		ORDER_OPERATION_MODIFY = 1,
		ORDER_OPERATION_DELETE = 2

	}OrderOperation;

	typedef struct
	{
		uint8_t level;    
		OrderSide side; 
		uint32_t price;
		uint32_t quantity;
		uint32_t count; 
		uint32_t orderID;
		uint32_t symbolIndex;
		OrderOperation operation; 
		uint64_t timestamp;
	}FeedData;

	uint32_t ReadData(FeedData* pData);






public: //Stats

	typedef struct
	{
		uint64_t numProcessedBytes;				// Number of UDP payload bytes forwarded.
		uint32_t numProcessedPackets;			// Number of UDP packets processed.
		uint32_t numProcessedBinaryMessages;	// Number of binary messages processed
		uint32_t numProcessedFIXMessages;		// Number of FIX messages processed.
		uint32_t numTxOrderBookOperations;		// Number of Order Book operations sent.
		uint32_t numClockTickEvents;			// Number of events received from clock tick generator block

	} Stats;

	uint32_t GetStats(Stats* pStats);
	uint32_t ResetStats(void);






public:
    uint32_t Start(void);
    uint32_t Stop(void);
    uint32_t IsRunning(bool* pbIsRunning);



public:
    void IsInitialised(bool* pbIsInitialised);
	uint32_t GetCUAddress(uint64_t* pCUAddress);
	uint32_t GetCUIndex(uint32_t* pCUIndex);










protected:
	uint32_t CheckIsInitialised(void);
	uint32_t CheckSecurityIndex(uint32_t index);
	uint32_t CheckSecurityID(uint32_t securityID);


protected:
	uint32_t InternalWriteSecurityIDToHW(uint32_t index, uint32_t securityID);
	uint32_t InternalReadSecurityIDFromHW(uint32_t index, uint32_t* pSecurityID);
	uint32_t InternalPopulateCache(void);

protected:
	uint32_t InternalGetFreeSecurityIndex(uint32_t* pIndex);



protected:
	// the follow functions provide a form of mutual exclusion to the capture data.
	// i.e they enable/disable HW updates to the capture.
	uint32_t FreezeData(void);
	uint32_t UnfreezeData(void);

protected:
	uint32_t ReadReg32(uint64_t offset, uint32_t* value);
	uint32_t WriteReg32(uint64_t offset, uint32_t value);
	uint32_t WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask);
	uint32_t BlockReadReg32(uint64_t offset, uint32_t* buffer, uint32_t numWords);




protected:
	uint32_t m_initialisedMagicNumber;
	uint64_t m_cuAddress;
	uint32_t m_cuIndex;
	DeviceInterface* m_pDeviceInterface;


protected: //cache
	uint32_t m_numSecurites;
	uint32_t m_securityIDLookup[MAX_NUM_SECURITIES]; //NOTE - sparse array


}; //class FeedHandler



} //namespace XLNX



#endif //XLNX_FEED_HANDLER_H

