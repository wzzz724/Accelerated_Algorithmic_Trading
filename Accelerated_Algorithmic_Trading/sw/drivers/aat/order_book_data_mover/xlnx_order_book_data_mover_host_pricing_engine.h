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

#ifndef XLNX_ORDER_BOOK_DATA_MOVER_HOST_PRICING_ENGINE_H
#define XLNX_ORDER_BOOK_DATA_MOVER_HOST_PRICING_ENGINE_H

#include "xlnx_order_book_data_mover_host_pricing_interface.h"

namespace XLNX
{

class HostPricingEngine : public HostPricingInterface
{

public:
    HostPricingEngine();
    virtual ~HostPricingEngine();



public: //HostPricingInterface
    bool PricingProcess(orderBookResponse_t* response, orderEntryOperation_t* operation);
    void SetVerboseTracing(bool bEnabled);


   

protected:
    static const uint32_t NUM_SYMBOL = 256;

    typedef struct hostPricingEngineCacheEntry_t
    {
        uint32_t bidPrice;
        uint32_t askPrice;
        uint32_t valid;
    } hostPricingEngineCacheEntry_t;


    hostPricingEngineCacheEntry_t   m_cache[NUM_SYMBOL];

    uint32_t m_orderId = 1; //default to 1 to match the HW pricing engine

    bool m_bVerboseTracing = false;
    
}; //end class HostPricingEngine


} //end namespace XLNX


#endif //XLNX_ORDER_BOOK_DATA_MOVER_HOST_PRICING_ENGINE_H


