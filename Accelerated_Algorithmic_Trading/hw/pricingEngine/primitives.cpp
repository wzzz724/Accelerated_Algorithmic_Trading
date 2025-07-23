#include "pricingengine.hpp"
#include <string>

// Data Input & Streaming Primitives

BookSnapshot PricingEngine::getBookSnapshot(ap_uint<8> symbolIndex,
                                            ap_uint<8> depth) {
#pragma HLS INLINE off
#pragma HLS PIPELINE II=1

    pricingEngineCacheEntry_t &entry = cache[symbolIndex];

    BookSnapshot result;

    ap_uint<8> actualDepth = (depth > LEVELS) ? LEVELS : depth;

    for (int i = 0; i < LEVELS; ++i) {
#pragma HLS UNROLL
        if (i < actualDepth) {
            result.levels[i].bidPrice = entry.bidPrice[i];
            result.levels[i].bidSize  = entry.bidSize[i];
            result.levels[i].askPrice = entry.askPrice[i];
            result.levels[i].askSize  = entry.askSize[i];
        } else {
            result.levels[i].bidPrice = 0;
            result.levels[i].bidSize  = 0;
            result.levels[i].askPrice = 0;
            result.levels[i].askSize  = 0;
        }
    }

    return result;
}

ap_int<32> PricingEngine::getOrderDelta(ap_uint<8> symbolIndex,
                                        ap_uint<3> level,
                                        bool isBidSide)
{
#pragma HLS INLINE off
#pragma HLS PIPELINE II=1

    pricingEngineCacheEntry_t &entry = cache[symbolIndex];
    if (level >= LEVELS) return 0;

    return isBidSide ? entry.bidDelta[level] : entry.askDelta[level];
}

ap_uint<56> PricingEngine::getTimeSinceLastUpdate(ap_uint<8> symbolIndex,
                                                  ap_uint<3> level,
                                                  bool isBidSide,
                                                  ap_uint<56> nowTimestamp)
{
#pragma HLS INLINE off
#pragma HLS PIPELINE II=1

    if (level >= LEVELS) return 0;

    pricingEngineCacheEntry_t &entry = cache[symbolIndex];
    ap_uint<56> lastUpdate = isBidSide ? entry.lastUpdateTimestampBid[level]
                                       : entry.lastUpdateTimestampAsk[level];

    return nowTimestamp - lastUpdate;  // 返回时间差（单位 = response.timestamp 单位）
}

// Data Transformation Primitives

TimeSeriesBuffer& getBuffer(pricingEngineCacheEntry_t& entry, ap_uint<8> field, int level) {
    if (field == BID_PRICE) return entry.bidPriceHistory;
    if (field == ASK_PRICE) return entry.askPriceHistory;
    if (field == TRADE_PRICE) return entry.tradePriceHistory;
    if (field == BID_SIZE) return entry.bidSizeHistory[level];
    if (field == ASK_SIZE) return entry.askSizeHistory[level];
    if (field == POSITION_SIZE) return entry.positionSizeHistory;
    if (field == PNL_ESTIMATE) return entry.pnlEstimateHistory;
    
    // 默认回退（返回成交价历史）
    return entry.tradePriceHistory;
}

// 滑动平均
ap_uint<32> PricingEngine::getMovingAvg(ap_uint<8> symbolIndex, ap_uint<8> field, ap_uint<8> window = 8, int level = 0) {
#pragma HLS INLINE off
    return getBuffer(cache[symbolIndex], field, level).movingAvg(window);
}

// 指数加权平均
ap_uint<32> PricingEngine::getExpAvg(ap_uint<8> symbolIndex, ap_uint<8> field, ap_uint<8> alpha = 32, ap_uint<8> window = 8, int level = 0) {
#pragma HLS INLINE off
    return getBuffer(cache[symbolIndex], field, level).expAvg(alpha, window);
}

// 最大值
ap_uint<32> PricingEngine::getMovingMax(ap_uint<8> symbolIndex, ap_uint<8> field, ap_uint<8> window = 8, int level = 0) {
#pragma HLS INLINE off
    return getBuffer(cache[symbolIndex], field, level).movingMax(window);
}

// 最小值
ap_uint<32> PricingEngine::getMovingMin(ap_uint<8> symbolIndex, ap_uint<8> field, ap_uint<8> window = 8, int level = 0) {
#pragma HLS INLINE off
    return getBuffer(cache[symbolIndex], field, level).movingMin(window);
}

// 求和
ap_uint<32> PricingEngine::getMovingSum(ap_uint<8> symbolIndex, ap_uint<8> field, ap_uint<8> window = 8, int level = 0) {
#pragma HLS INLINE off
    return getBuffer(cache[symbolIndex], field, level).movingSum(window);
}

// 时间导数
ap_uint<32> PricingEngine::getDerivative(ap_uint<8> symbolIndex, ap_uint<8> field, int level = 0) {
#pragma HLS INLINE off
    return getBuffer(cache[symbolIndex], field, level).derivative();
}

ap_int<32> PricingEngine::getCrossover(const TimeSeriesBuffer& signal1, const TimeSeriesBuffer& signal2) {
#pragma HLS INLINE

    if (signal1.size < 2 || signal2.size < 2)
        return 0; // 数据不足

    ap_uint<32> prev1 = signal1.get(signal1.size - 2);
    ap_uint<32> curr1 = signal1.get(signal1.size - 1);
    ap_uint<32> prev2 = signal2.get(signal2.size - 2);
    ap_uint<32> curr2 = signal2.get(signal2.size - 1);

    if (prev1 <= prev2 && curr1 > curr2)
        return 1;   // 上穿
    else if (prev1 >= prev2 && curr1 < curr2)
        return -1;  // 下穿
    else
        return 0;   // 无交叉
}

ap_fixed<16, 2> PricingEngine::getImbalance(ap_uint<32> bid_vol, ap_uint<32> ask_vol) {
#pragma HLS INLINE

    ap_uint<32> total = bid_vol + ask_vol;

    if (total == 0)
        return 0;

    ap_fixed<32, 4> bid = bid_vol;
    ap_fixed<32, 4> ask = ask_vol;
    ap_fixed<32, 4> diff = bid - ask;
    ap_fixed<32, 4> sum = bid + ask;

    return (ap_fixed<16, 2>)(diff / sum);
}

bool PricingEngine::priceJump(ap_uint<8> symbolIndex, ap_uint<32> threshold) {
#pragma HLS INLINE

    // 最近两个成交价
    ap_uint<32> curr = cache[symbolIndex].tradePriceHistory.getLatest();
    ap_uint<32> prev = cache[symbolIndex].tradePriceHistory.getPrev(1);

    ap_uint<32> diff = (curr > prev) ? (curr - prev) : (prev - curr);

    return (diff > threshold);
}

bool PricingEngine::SPIKE(const TimeSeriesBuffer &tsBuf, ap_fixed<32, 8> std_dev_thresh) {
#pragma HLS INLINE

    if (tsBuf.count < 2) return false;

    ap_fixed<32, 8> mean = 0;
    ap_fixed<32, 8> variance = 0;

    // Step 1: 均值计算
    for (int i = 0; i < tsBuf.count; ++i) {
#pragma HLS UNROLL
        mean += ap_fixed<32, 8>(tsBuf.getPrev(i));
    }
    mean /= ap_fixed<32, 8>(tsBuf.count);

    // Step 2: 方差计算（无开方）
    for (int i = 0; i < tsBuf.count; ++i) {
#pragma HLS UNROLL
        ap_fixed<32, 8> diff = ap_fixed<32, 8>(tsBuf.getPrev(i)) - mean;
        variance += diff * diff;
    }
    variance /= ap_fixed<32, 8>(tsBuf.count);

    // Step 3: 当前值与均值的差异平方
    ap_fixed<32, 8> current = ap_fixed<32, 8>(tsBuf.getLatest());
    ap_fixed<32, 8> delta = current - mean;
    ap_fixed<32, 8> delta_sq = delta * delta;

    // Step 4: 与阈值平方 × 方差 比较
    ap_fixed<32, 8> thresh_sq = std_dev_thresh * std_dev_thresh;
    return delta_sq > thresh_sq * variance;
}

ap_int<32> PricingEngine::BOOK_PRESSURE(ap_uint<8> symbolIndex, const ap_uint<4> levels[], int num_levels) {
#pragma HLS INLINE

    ap_uint<32> bid_sum = 0;
    ap_uint<32> ask_sum = 0;

    for (int i = 0; i < num_levels; ++i) {
#pragma HLS UNROLL
        ap_uint<4> level = levels[i];
        if (level < LEVELS) {
            bid_sum += cache[symbolIndex].bidSize[level];
            ask_sum += cache[symbolIndex].askSize[level];
        }
    }

    return ap_int<32>(bid_sum) - ap_int<32>(ask_sum);
}

// STATEFUL_IF函数，条件成立时切换状态，返回当前状态
ap_uint<8> PricingEngine::STATEFUL_IF(ap_uint<8> symbolIndex, bool condition, ap_uint<8> state) {
#pragma HLS INLINE
    if (condition) {
        currentState = state;
    }
    return currentState;
}

ap_uint<32> PricingEngine::LATENCY_GATE(const TimeSeriesBuffer &tsBuf, ap_uint<8> delay) {
#pragma HLS INLINE

    if (tsBuf.count < delay)
        return 0; // 数据不足

    return tsBuf.getPrev(delay);
}

bool PricingEngine::sendOrder(ap_uint<8> symbolIndex,
                              ap_uint<32> quantity,
                              ap_uint<32> price,
                              ap_uint<1> direction,
                            orderEntryOperation_t &operation)
{
#pragma HLS INLINE

    operation.timestamp = cache[symbolIndex].clockUS;
    operation.opCode = ORDERENTRY_ADD;
    operation.symbolIndex = symbolIndex;
    operation.quantity = quantity;
    operation.price = price;
    operation.direction = direction;

    // 发送订单到操作流
    return true;
}