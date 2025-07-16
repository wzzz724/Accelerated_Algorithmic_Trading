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

#include <iostream>
#include "pricingengine.hpp"

/**
 * PricingEngine Core
 */

void PricingEngine::responsePull(ap_uint<32> &regRxResponse,
                                 hls::stream<orderBookResponsePack_t> &responseStreamPack,
                                 hls::stream<orderBookResponse_t> &responseStream)
{
#pragma HLS PIPELINE II=1 style=flp

    mmInterface intf;
    orderBookResponsePack_t responsePack;
    orderBookResponse_t response;

    static ap_uint<32> countRxResponse=0;

    if(!responseStreamPack.empty())
    {
        responsePack = responseStreamPack.read();
        intf.orderBookResponseUnpack(&responsePack, &response);
        responseStream.write(response);
        ++countRxResponse;
    }

    regRxResponse = countRxResponse;

    return;
}

void PricingEngine::pricingProcess(ap_uint<32> &regStrategyControl,
                                   ap_uint<32> &regProcessResponse,
                                   ap_uint<32> &regStrategyNone,
                                   ap_uint<32> &regStrategyPeg,
                                   ap_uint<32> &regStrategyLimit,
                                   ap_uint<32> &regStrategyUnknown,
                                   pricingEngineRegStrategy_t *regStrategies,
                                   hls::stream<orderBookResponse_t> &responseStream,
                                   hls::stream<orderEntryOperation_t> &operationStream)
{
#pragma HLS PIPELINE II=1 style=flp

    mmInterface intf;
    orderBookResponse_t response;
    orderEntryOperation_t operation;

    ap_uint<8> symbolIndex = 0;
    ap_uint<8> strategySelect = 0;
    ap_uint<8> thresholdEnable = 0;
    ap_uint<8> thresholdPosition = 0;
    bool orderExecute = false;

    static ap_uint<32> orderId = 0;
    static ap_uint<32> countProcessResponse = 0;
    static ap_uint<32> countStrategyNone = 0;
    static ap_uint<32> countStrategyPeg = 0;
    static ap_uint<32> countStrategyLimit = 0;
    static ap_uint<32> countStrategyUnknown = 0;

    if (!responseStream.empty())
    {
        response = responseStream.read();
        ++countProcessResponse;

        symbolIndex = response.symbolIndex;
        pricingEngineCacheEntry_t &entry = cache[symbolIndex];

        thresholdEnable = regStrategies[symbolIndex].enable.range(7, 0);

        // 选策略
        if (PE_GLOBAL_STRATEGY & regStrategyControl)
            strategySelect = regStrategyControl.range(7, 0);
        else
            strategySelect = regStrategies[symbolIndex].select.range(7, 0);

        // ==== 状态缓存更新 ====

        // 时间状态
        entry.tickIndex += 1;
        entry.clockUS = response.timestamp.to_uint();
        entry.valid = 1;

        // 价格状态
        ap_uint<32> rawBid = response.bidPrice.range(31, 0);
        ap_uint<32> rawAsk = response.askPrice.range(31, 0);

        // 成交方向推断
        if (entry.bidPrice != rawBid)
            entry.lastTradeSide = 1; // BUY
        else if (entry.askPrice != rawAsk)
            entry.lastTradeSide = 0; // SELL

        // 拆解 bid/ask 量（10档，每档16bit）
        for (int i = 0; i < LEVELS; i++) {
#pragma HLS UNROLL
            entry.bidSize[i] = response.bidQuantity.range((i + 1) * 32 - 1, i * 32);
            entry.askSize[i] = response.askQuantity.range((i + 1) * 32 - 1, i * 32);
        }

        // 更新价格缓存
        entry.bidPrice = rawBid;
        entry.askPrice = rawAsk;
        entry.tradePrice = (entry.bidPrice + entry.askPrice) / 2;

        // 设置系统状态
        entry.systemState = 1; // STATE_RUNNING

        // ==== 策略执行 ====

        switch (strategySelect)
        {
            case (STRATEGY_NONE):
                ++countStrategyNone;
                orderExecute = false;
                break;

            case (STRATEGY_PEG):
                ++countStrategyPeg;
                orderExecute = pricingStrategyPeg(thresholdEnable,
                                                  thresholdPosition,
                                                  response,
                                                  operation);
                break;

            case (STRATEGY_LIMIT):
                ++countStrategyLimit;
                orderExecute = pricingStrategyLimit(thresholdEnable,
                                                    thresholdPosition,
                                                    response,
                                                    operation);
                break;

            default:
                ++countStrategyUnknown;
                orderExecute = false;
                break;
        }

        // ==== 下单逻辑 ====

        if (orderExecute)
        {
            operation.orderId = ++orderId;
            entry.lastOrderId = orderId;

            if (operation.direction == ORDER_BID)
                entry.positionSize += operation.quantity;
            else
                entry.positionSize -= operation.quantity;

            // 粗略 PnL = 仓位 × (成交价格 - 当前买价)
            entry.pnlEstimate = entry.positionSize * (entry.tradePrice - entry.bidPrice);

            operationStream.write(operation);
        }
    }

    // 写回寄存器
    regProcessResponse = countProcessResponse;
    regStrategyNone = countStrategyNone;
    regStrategyPeg = countStrategyPeg;
    regStrategyLimit = countStrategyLimit;
    regStrategyUnknown = countStrategyUnknown;

    return;
}


bool PricingEngine::pricingStrategyPeg(ap_uint<8> thresholdEnable,
                                       ap_uint<32> thresholdPosition,
                                       orderBookResponse_t &response,
                                       orderEntryOperation_t &operation)
{
#pragma HLS PIPELINE II=1 style=flp

    ap_uint<8> symbolIndex=0;
    bool executeOrder=false;

    symbolIndex = response.symbolIndex;

    // TODO: restore valid check when test data updated to trigger top of book update
    //if(cache[symbolIndex].valid)
    {
        if(cache[symbolIndex].bidPrice != response.bidPrice.range(31,0))
        {
            // create an order, current best bid +100
            operation.timestamp = response.timestamp;
            operation.opCode = ORDERENTRY_ADD;
            operation.symbolIndex = symbolIndex;
            operation.quantity = 800;
            operation.price = (response.bidPrice.range(31,0)+100);
            operation.direction = ORDER_BID;
            executeOrder = true;
        }
    }

    // cache top of book prices (used as trigger on next delta if change detected)
    cache[symbolIndex].bidPrice = response.bidPrice.range(31,0);
    cache[symbolIndex].askPrice = response.askPrice.range(31,0);
    cache[symbolIndex].valid = true;

    return executeOrder;
}

bool PricingEngine::pricingStrategyLimit(ap_uint<8> thresholdEnable,
                                         ap_uint<32> thresholdPosition,
                                         orderBookResponse_t &response,
                                         orderEntryOperation_t &operation)
{
#pragma HLS PIPELINE II=1 style=flp

    ap_uint<8> symbolIndex=0;
    bool executeOrder=false;

    symbolIndex = response.symbolIndex;

    // TODO: restore valid check when test data updated to trigger top of book update
    //if(cache[symbolIndex].valid)
    {
        if(cache[symbolIndex].bidPrice != response.bidPrice.range(31,0))
        {
            // create an order, current best bid +50
            operation.timestamp = response.timestamp;
            operation.opCode = ORDERENTRY_ADD;
            operation.symbolIndex = symbolIndex;
            operation.quantity = 800;
            operation.price = (response.bidPrice.range(31,0)+50);
            operation.direction = ORDER_BID;
            executeOrder = true;
        }
    }

    // cache top of book prices (used as trigger on next delta if change detected)
    cache[symbolIndex].bidPrice = response.bidPrice.range(31,0);
    cache[symbolIndex].askPrice = response.askPrice.range(31,0);
    cache[symbolIndex].valid = true;

    return executeOrder;
}

void PricingEngine::operationPush(ap_uint<32> &regCaptureControl,
                                  ap_uint<32> &regTxOperation,
                                  ap_uint<1024> &regCaptureBuffer,
                                  hls::stream<orderEntryOperation_t> &operationStream,
                                  hls::stream<orderEntryOperationPack_t> &operationStreamPack)
{
#pragma HLS PIPELINE II=1 style=flp

    mmInterface intf;
    orderEntryOperation_t operation;
    orderEntryOperationPack_t operationPack;

    static ap_uint<32> countTxOperation=0;

    if(!operationStream.empty())
    {
        operation = operationStream.read();

        intf.orderEntryOperationPack(&operation, &operationPack);
        operationStreamPack.write(operationPack);
        ++countTxOperation;

        // check if host has capture freeze control enabled before updating
        // TODO: filter capture by user supplied symbol
        if(0 == (PE_CAPTURE_FREEZE & regCaptureControl))
        {
            regCaptureBuffer = operationPack.data;
        }
    }

    regTxOperation = countTxOperation;

    return;
}

void PricingEngine::eventHandler(ap_uint<32> &regRxEvent,
                                 hls::stream<clockTickGeneratorEvent_t> &eventStream)
{
#pragma HLS PIPELINE II=1 style=flp

    clockTickGeneratorEvent_t tickEvent;

    static ap_uint<32> countRxEvent=0;

    if(!eventStream.empty())
    {
        eventStream.read(tickEvent);
        ++countRxEvent;

        // event notification has been received from programmable clock tick
        // generator, handling currently limited to incrementing a counter,
        // placeholder for user to extend with custom event handling code
    }

    regRxEvent = countRxEvent;

    return;
}
