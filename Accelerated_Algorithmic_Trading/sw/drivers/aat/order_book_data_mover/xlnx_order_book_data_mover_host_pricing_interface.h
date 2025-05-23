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

#ifndef XLNX_ORDER_BOOK_DATA_MOVER_HOST_PRICING_INTERFACE_H
#define XLNX_ORDER_BOOK_DATA_MOVER_HOST_PRICING_INTERFACE_H

#include <cstdint>


namespace XLNX
{
    /*-------------------------------------------- HW data interface ----------------------------------------*/





typedef struct orderBookResponse_t
{
    uint32_t askQuantity[5];
    uint32_t askPrice[5];
    uint32_t askCount[5];
    uint32_t bidQuantity[5];
    uint32_t bidPrice[5];
    uint32_t bidCount[5];
    uint8_t  symbolIndex;
    uint64_t timestamp;
} orderBookResponse_t;






typedef struct orderEntryOperation_t
{
    uint64_t timestamp;
    uint8_t  opCode;
    uint8_t  symbolIndex;
    uint32_t orderId;
    uint32_t quantity;
    uint32_t price;
    uint8_t  direction;
} orderEntryOperation_t;












class HostPricingInterface
{
public:
    virtual bool PricingProcess(orderBookResponse_t* response, orderEntryOperation_t* operation) = 0;

    virtual void SetVerboseTracing(bool bEnabled) = 0;


protected:
    static const uint32_t ORDER_OPERATION_ADD       = 0;
    static const uint32_t ORDER_OPERATION_MODIFY    = 1;
    static const uint32_t ORDER_OPERATION_DELETE    = 2;
    
    static const uint32_t ORDER_SIDE_BID            = 0;
    static const uint32_t ORDER_SIDE_ASK            = 1;

};




} //end namespace XLNX







#endif //XLNX_ORDER_BOOK_DATA_MOVER_HOST_PRICING_INTERFACE_H
