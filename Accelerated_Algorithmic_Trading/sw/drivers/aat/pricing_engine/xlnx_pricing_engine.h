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


#ifndef XLNX_PRICING_ENGINE_H
#define XLNX_PRICING_ENGINE_H


#include <cstdint>

#include "xlnx_device_interface.h"

#include "xlnx_pricing_engine_error_codes.h"


namespace XLNX
{

class PricingEngine
{


public:
    PricingEngine();
    virtual ~PricingEngine();


public:
    uint32_t Initialise(DeviceInterface* pDeviceInterface, const char* cuName);






public:
    typedef struct
    {
        uint32_t numRxResponses;
        uint32_t numProcessedResponses;
        uint32_t numTxOperations;

        uint32_t numStrategyNone;       //number of executions for strategy = NONE
        uint32_t numStrategyPeg;        //number of executions for strategy = PEG
        uint32_t numStrategyLimit;      //number of executions for strategy = LIMIT
        uint32_t numStrategyUnknown;    //number of executions for strategy = UNKNOWN

        uint32_t numClockTickEvents;
    } Stats;

    uint32_t GetStats(Stats* pStats);
    uint32_t ResetStats(void);





public:
    static const uint32_t MAX_NUM_SYMBOLS = 256;

    typedef enum
    {
        ORDER_SIDE_BID = 0,
        ORDER_SIDE_ASK = 1

    }OrderSide;



    typedef enum
    {
        ORDER_OPERATION_ADD     = 0,
        ORDER_OPERATION_MODIFY  = 1,
        ORDER_OPERATION_DELETE  = 2

    }OrderOperation;



    typedef struct
    {
        uint32_t       symbolIndex;
        OrderSide      orderSide;
        OrderOperation orderOperation;
        uint32_t       orderID;
        uint32_t       orderPrice;
        uint32_t       orderQuantity;

    } PricingEngineData;




public:
    uint32_t SetCaptureFilter(uint32_t symbolIndex);
    uint32_t GetCaptureFilter(uint32_t* pSymbolIndex);

    uint32_t ReadData(PricingEngineData* pData);






public: //Pricing Strategy

    typedef enum
    {
        STRATEGY_NONE   = 0,
        STRATEGY_PEG    = 1,
        STRATEGY_LIMIT  = 2

    }PricingStrategy;


    //Global Strategy - when global strategy is enabled, ALL symbols will
    //                  use the SAME pricing strategy
    //                  when global strategy is disabled, each symbol can
    //                  use its own strategy
    uint32_t SetGlobalStrategyMode(bool bEnabled);
    uint32_t GetGlobalStrategyMode(bool* pbEnabled);

    uint32_t SetGlobalStrategy(PricingStrategy strategy);
    uint32_t GetGlobalStrategy(PricingStrategy* pStrategy);





public:
    uint32_t Start(void);
    uint32_t Stop(void);
    uint32_t IsRunning(bool* pbIsRunning);







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





protected:
    uint32_t m_initialisedMagicNumber;
    uint64_t m_cuAddress;
    uint32_t m_cuIndex;
    DeviceInterface* m_pDeviceInterface;

    char m_cuName[DeviceInterface::MAX_CU_NAME_LENGTH + 1];

    static const uint32_t STREAM_ARG_INDEX = 6;
    DeviceInterface::StreamHandleType m_streamHandle;

};



} //end namespace XLNX



#endif //XLNX_PRICING_ENGINE_H
