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

#include <cstdio>

#include "xlnx_order_book_data_mover_host_pricing_engine.h"
using namespace XLNX;




HostPricingEngine::HostPricingEngine()
{

}




HostPricingEngine::~HostPricingEngine()
{

}






bool HostPricingEngine::PricingProcess(orderBookResponse_t* response, orderEntryOperation_t* operation)
{
    uint8_t symbolIndex = 0;
    bool executeOrder = false;

    symbolIndex = response->symbolIndex;

    // TODO: restore valid check when test data updated to trigger top of book update
    //if(cache[symbolIndex].valid)
    {
        if (m_bVerboseTracing)
        {
            printf("[HPE response] symbolIndex = %d\n", response->symbolIndex);
            printf("[HPE response] bidCount    = %d\n", response->bidCount[0]);
            printf("[HPE response] bidPrice    = %d\n", response->bidPrice[0]);
            printf("[HPE response] bidQuantity = %d\n", response->bidQuantity[0]);
            printf("[HPE response] askCount    = %d\n", response->askCount[0]);
            printf("[HPE response] askPrice    = %d\n", response->askPrice[0]);
            printf("[HPE response] askQuantity = %d\n", response->askQuantity[0]);
            printf("\n");
        }



        if (m_cache[symbolIndex].bidPrice != response->bidPrice[0])
        {
            // create an order, current best bid +100
            operation->opCode = ORDER_OPERATION_ADD;
            operation->symbolIndex = symbolIndex;
            operation->orderId = m_orderId++;
            operation->quantity = 800;
            operation->price = (response->bidPrice[0] + 100);
            operation->direction = ORDER_SIDE_BID;
            operation->timestamp = response->timestamp;
            executeOrder = true;

            if (m_bVerboseTracing)
            {
                printf("    [HPE operation] opCode      = %d\n", operation->opCode);
                printf("    [HPE operation] symbolIndex = %d\n", operation->symbolIndex);
                printf("    [HPE operation] orderId     = %d\n", operation->orderId);
                printf("    [HPE operation] quantity    = %d\n", operation->quantity);
                printf("    [HPE operation] price       = %d\n", operation->price);
                printf("    [HPE operation] direction   = %d\n", operation->direction);
                printf("\n");
            }
        }
    }

    // cache top of book prices (used as trigger on next delta if change detected)
    m_cache[symbolIndex].bidPrice = response->bidPrice[0];
    m_cache[symbolIndex].askPrice = response->askPrice[0];
    m_cache[symbolIndex].valid = true;

    return executeOrder;
}






void HostPricingEngine::SetVerboseTracing(bool bEnabled)
{
    m_bVerboseTracing = bEnabled;
}


