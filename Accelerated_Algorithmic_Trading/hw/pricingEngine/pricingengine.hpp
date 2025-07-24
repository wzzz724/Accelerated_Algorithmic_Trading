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

#ifndef PRICINGENGINE_H
#define PRICINGENGINE_H

#include "hls_stream.h"
#include "ap_int.h"
#include "ap_fixed.h"
#include "aat_defines.hpp"
#include "aat_interfaces.hpp"

#define PE_GLOBAL_STRATEGY (1<<31)
#define PE_CAPTURE_FREEZE  (1<<31)

// 最多支持 5 档报价
#define LEVELS 5
#define MAX_WINDOW 8

typedef struct pricingEngineRegControl_t
{
    ap_uint<32> control;
    ap_uint<32> config;
    ap_uint<32> capture;
    ap_uint<32> strategy;
    ap_uint<32> reserved04;
    ap_uint<32> reserved05;
    ap_uint<32> reserved06;
    ap_uint<32> reserved07;
} pricingEngineRegControl_t;

typedef struct pricingEngineRegStatus_t
{
    ap_uint<32> status;
    ap_uint<32> rxResponse;
    ap_uint<32> processResponse;
    ap_uint<32> txOperation;
    ap_uint<32> strategyNone;
    ap_uint<32> strategyPeg;
    ap_uint<32> strategyLimit;
    ap_uint<32> strategyUnknown;
    ap_uint<32> rxEvent;
    ap_uint<32> debug;
    ap_uint<32> reserved10;
    ap_uint<32> reserved11;
    ap_uint<32> reserved12;
    ap_uint<32> reserved13;
    ap_uint<32> reserved14;
    ap_uint<32> reserved15;
} pricingEngineRegStatus_t;

typedef struct pricingEngineRegStrategy_t
{
    // 32b registers are wider than required here for some fields (e.g. select and enable) but not sure
    // what XRT does in terms of packing, potential to get messy in terms of register map decoding from host
    // TODO: width appropriate fields when XRT register map packing mechanism is understood
    ap_uint<32> select;  // 8b
    ap_uint<32> enable;  // 8b
    ap_uint<32> totalBid;
    ap_uint<32> totalAsk;
} pricingEngineRegStrategy_t;

typedef struct pricingEngineRegThresholds_t
{
    ap_uint<32> threshold0;
    ap_uint<32> threshold1;
    ap_uint<32> threshold2;
    ap_uint<32> threshold3;
    ap_uint<32> threshold4;
    ap_uint<32> threshold5;
    ap_uint<32> threshold6;
    ap_uint<32> threshold7;
} pricingEngineRegThresholds_t;

typedef struct TimeSeriesEntry {
    ap_uint<32> value;
    ap_uint<56> timestamp;
} TimeSeriesEntry;

typedef struct TimeSeriesBuffer {
    TimeSeriesEntry buffer[MAX_WINDOW];
    ap_uint<8> index = 0;
    ap_uint<8> count = 0;

    

    // 插入新数据（值 + 时间戳）
    void insert(ap_uint<32> val, ap_uint<56> ts) {
#pragma HLS INLINE
        buffer[index].value = val;
        buffer[index].timestamp = ts;
        index = (index + 1) % MAX_WINDOW;
        if (count < MAX_WINDOW) ++count;
    }

    // 获取最近一个值（index-1）
    ap_uint<32> getLatest() const {
#pragma HLS INLINE
        if (count == 0) return 0;
        ap_uint<8> lastIdx = MAX_WINDOW - 1;
        if (index != 0) lastIdx = index - 1;
        return buffer[lastIdx].value;
    }

    // 获取前n个值（n=1代表上一个）
    ap_uint<32> getPrev(ap_uint<8> n) const {
#pragma HLS INLINE
        if (n > count) return 0;
        ap_uint<8> pos = (index + MAX_WINDOW - n) % MAX_WINDOW;
        return buffer[pos].value;
    }

    // 滑动平均
    ap_uint<32> movingAvg(ap_uint<8> window) const {
#pragma HLS INLINE
        ap_uint<64> sum = 0;
        ap_uint<8> actual = (window > count) ? count : window;
        for (int i = 0; i < MAX_WINDOW; ++i) {
#pragma HLS UNROLL
            if (i < actual) {
                int idx = (index + MAX_WINDOW - 1 - i) % MAX_WINDOW;
                sum += buffer[idx].value;
            }
        }
        ap_uint<32> result = 0;
        if (actual > 0) result = sum / actual;
        return result;
    }

    // 滑动求和
    ap_uint<32> movingSum(ap_uint<8> window) const {
#pragma HLS INLINE
        ap_uint<64> sum = 0;
        ap_uint<8> actual = (window > count) ? count : window;
        for (int i = 0; i < MAX_WINDOW; ++i) {
#pragma HLS UNROLL
            if (i < actual) {
                int idx = (index + MAX_WINDOW - 1 - i) % MAX_WINDOW;
                sum += buffer[idx].value;
            }
        }
        return sum;
    }

    // 最大值
    ap_uint<32> movingMax(ap_uint<8> window) const {
#pragma HLS INLINE
        ap_uint<32> maxVal = 0;
        ap_uint<8> actual = (window > count) ? count : window;
        for (int i = 0; i < MAX_WINDOW; ++i) {
#pragma HLS UNROLL
            if (i < actual) {
                int idx = (index + MAX_WINDOW - 1 - i) % MAX_WINDOW;
                if (buffer[idx].value > maxVal)
                    maxVal = buffer[idx].value;
            }
        }
        return maxVal;
    }

    // 最小值
    ap_uint<32> movingMin(ap_uint<8> window) const {
#pragma HLS INLINE
        ap_uint<32> minVal = ~ap_uint<32>(0); // 全1，即最大uint32
        ap_uint<8> actual = (window > count) ? count : window;
        for (int i = 0; i < MAX_WINDOW; ++i) {
#pragma HLS UNROLL
            if (i < actual) {
                int idx = (index + MAX_WINDOW - 1 - i) % MAX_WINDOW;
                if (buffer[idx].value < minVal)
                    minVal = buffer[idx].value;
            }
        }
        return minVal;
    }

    // 指数加权平均（α ∈ [0,255]）
    ap_uint<32> expAvg(ap_uint<8> alpha, ap_uint<8> window) const {
#pragma HLS INLINE
        if (count == 0) return 0;

        ap_uint<8> actual = (window > count) ? count : window;

        ap_uint<32> ema = buffer[(index + MAX_WINDOW - 1) % MAX_WINDOW].value;

        for (int i = 1; i < MAX_WINDOW; ++i) {
#pragma HLS UNROLL
            if (i < actual) {
                int idx = (index + MAX_WINDOW - 1 - i) % MAX_WINDOW;
                ema = ((ap_uint<64>)alpha * buffer[idx].value +
                    (ap_uint<64>)(255 - alpha) * ema) >> 8;
            }
        }

        return ema;
    }

    // 时间导数（Δ值 / Δ时间）
    ap_uint<32> derivative() const {
#pragma HLS INLINE
        if (count < 2) return 0;

        int idx1 = (index + MAX_WINDOW - 1) % MAX_WINDOW;
        int idx0 = (index + MAX_WINDOW - 2) % MAX_WINDOW;

        ap_uint<32> dv = buffer[idx1].value - buffer[idx0].value;
        ap_uint<56> dt = buffer[idx1].timestamp - buffer[idx0].timestamp;

        ap_uint<32> result = 0;
        if (dt != 0) result = dv / dt;
        return result;
    }
} TimeSeriesBuffer;

enum PEState {
    STATE_IDLE = 0,
    STATE_ACTIVE = 1,
    STATE_ERROR = 2,
    // ...根据需要定义更多状态
};

typedef struct pricingEngineCacheEntry_t
{
    ap_uint<32> bidPrice[LEVELS];
    ap_uint<32> askPrice[LEVELS];

    ap_uint<32> tradePrice;
    ap_uint<32> bidSize[LEVELS];
    ap_uint<32> askSize[LEVELS];

    ap_int<32>  bidSizeDelta[LEVELS];        // 增量（可正负）
    ap_int<32>  askSizeDelta[LEVELS];

    ap_uint<32> positionSize;
    ap_uint<32> pnlEstimate;

    TimeSeriesBuffer bidPriceHistory;  // 买价历史
    TimeSeriesBuffer askPriceHistory;  // 卖价历史
    TimeSeriesBuffer tradePriceHistory; // 成交价历史
    TimeSeriesBuffer bidSizeHistory[LEVELS];   // 买量历史
    TimeSeriesBuffer askSizeHistory[LEVELS];   // 卖量历史
    TimeSeriesBuffer positionSizeHistory; // 仓位历史
    TimeSeriesBuffer pnlEstimateHistory; // PnL 估算历史

    ap_uint<1>  valid;
    ap_uint<56> lastUpdateTimestampBid[LEVELS]; // 买档上次更新时间
    ap_uint<56> lastUpdateTimestampAsk[LEVELS]; // 卖档上次更新时间
    ap_uint<1>  lastTradeSide;      // 0: SELL, 1: BUY
    ap_uint<32> tickIndex;
    ap_uint<32> clockUS;
    ap_uint<32> lastOrderId;
    ap_uint<8>  systemState = STATE_IDLE;
} pricingEngineCacheEntry_t;

// For primitives
typedef struct BookLevel
{
    ap_uint<32> bidPrice;
    ap_uint<32> bidSize;
    ap_uint<32> askPrice;
    ap_uint<32> askSize;
} BookLevel;

typedef struct BookSnapshot
{
    BookLevel levels[LEVELS];
} BookSnapshot;

enum VarField {
    BID_PRICE = 0,
    ASK_PRICE = 1,
    TRADE_PRICE = 2,
    BID_SIZE = 3,
    ASK_SIZE = 4,
    POSITION_SIZE = 5,
    PNL_ESTIMATE = 6,
    // ...根据需要定义更多状态
};


/**
 * PricingEngine Core
 */
class PricingEngine
{
public:

    void responsePull(ap_uint<32> &regRxResponse,
                      hls::stream<orderBookResponsePack_t> &responseStreamPack,
                      hls::stream<orderBookResponse_t> &responseStream);

    void pricingProcess(ap_uint<32> &regStrategyControl,
                        ap_uint<32> &regProcessResponse,
                        ap_uint<32> &regStrategyNone,
                        ap_uint<32> &regStrategyPeg,
                        ap_uint<32> &regStrategyLimit,
                        ap_uint<32> &regStrategyUnknown,
                        pricingEngineRegStrategy_t *regStrategies,
                        hls::stream<orderBookResponse_t> &responseStream,
                        hls::stream<orderEntryOperation_t> &operationStream);

    bool pricingStrategyPeg(ap_uint<8> thresholdEnable,
                            ap_uint<32> thresholdPosition,
                            orderBookResponse_t &response,
                            orderEntryOperation_t &operation);

    bool pricingStrategyLimit(ap_uint<8> thresholdEnable,
                              ap_uint<32> thresholdPosition,
                              orderBookResponse_t &response,
                              orderEntryOperation_t &operation);

    bool pricingStrategyCustom(orderBookResponse_t &response,
                               orderEntryOperation_t &operation);

    void operationPush(ap_uint<32> &regCaptureControl,
                       ap_uint<32> &regTxOperation,
                       ap_uint<1024> &regCaptureBuffer,
                       hls::stream<orderEntryOperation_t> &operationStream,
                       hls::stream<orderEntryOperationPack_t> &operationStreamPack);

    void eventHandler(ap_uint<32> &regRxEvent,
                      hls::stream<clockTickGeneratorEvent_t> &eventStream);

private:

    //pricingEngineRegThresholds_t thresholds[NUM_SYMBOL];
    pricingEngineCacheEntry_t cache[NUM_SYMBOL];

    // Primitives
    // 获取订单簿快照：最多返回 depth 档
    BookSnapshot getBookSnapshot(ap_uint<8> symbolIndex,
                                 ap_uint<8> depth);

    ap_int<32> getOrderDelta(ap_uint<8> symbolIndex,
                             ap_uint<3> level,
                             bool isBidSide);

    ap_uint<56> getTimeSinceLastUpdate(ap_uint<8> symbolIndex,
                                       ap_uint<3> level,
                                       bool isBidSide,
                                       ap_uint<56> nowTimestamp);

    // 滑动平均
    ap_uint<32> getMovingAvg(ap_uint<8> symbolIndex, ap_uint<8> field, ap_uint<8> window = 8, int level = 0);

    // 指数加权平均
    ap_uint<32> getExpAvg(ap_uint<8> symbolIndex, ap_uint<8> field, ap_uint<8> alpha = 32, ap_uint<8> window = 8, int level = 0);

    // 最大值
    ap_uint<32> getMovingMax(ap_uint<8> symbolIndex, ap_uint<8> field, ap_uint<8> window = 8, int level = 0);

    // 最小值
    ap_uint<32> getMovingMin(ap_uint<8> symbolIndex, ap_uint<8> field, ap_uint<8> window = 8, int level = 0);

    // 求和
    ap_uint<32> getMovingSum(ap_uint<8> symbolIndex, ap_uint<8> field, ap_uint<8> window = 8, int level = 0);

    // 时间导数
    ap_uint<32> getDerivative(ap_uint<8> symbolIndex, ap_uint<8> field, int level = 0);

    ap_int<32> getCrossover(const TimeSeriesBuffer& signal1, const TimeSeriesBuffer& signal2);

    ap_fixed<16, 2> getImbalance(ap_uint<32> bid_vol, ap_uint<32> ask_vol);

    bool PRICE_JUMP(ap_uint<8> symbolIndex, ap_uint<32> threshold);

    bool SPIKE(const TimeSeriesBuffer &tsBuf, ap_fixed<32, 8> std_dev_thresh);

    ap_int<32> BOOK_PRESSURE(ap_uint<8> symbolIndex, const ap_uint<4> levels[], int num_levels);

    ap_uint<8> STATEFUL_IF(ap_uint<8> symbolIndex, bool condition, ap_uint<8> state);

    // TODO: 直接在DSL解析器里实现
    // bool DEBOUNCE(bool condition, ap_uint<32> hold_time)

    ap_uint<32> LATENCY_GATE(const TimeSeriesBuffer &tsBuf, ap_uint<8> delay);

    bool sendOrder(ap_uint<8> symbolIndex,
                    ap_uint<32> quantity,
                    ap_uint<32> price,
                    ap_uint<1> direction,
                    orderEntryOperation_t &operation);
};

#endif
