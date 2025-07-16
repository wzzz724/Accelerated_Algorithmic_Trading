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

bool strEquals(const char* a, const char* b) {
#pragma HLS INLINE
    for (int i = 0; i < 16; ++i) {
#pragma HLS UNROLL
        if (a[i] != b[i]) return false;
        if (a[i] == '\0') return true;
    }
    return true;
}

TimeSeriesBuffer& getBuffer(pricingEngineCacheEntry_t& entry, const char* field, int level) {
    if (strEquals(field, "BID_PRICE")) return entry.bidPriceHistory;
    if (strEquals(field, "ASK_PRICE")) return entry.askPriceHistory;
    if (strEquals(field, "TRADE_PRICE")) return entry.tradePriceHistory;
    if (strEquals(field, "BID_SIZE")) return entry.bidSizeHistory[level];
    if (strEquals(field, "ASK_SIZE")) return entry.askSizeHistory[level];
    if (strEquals(field, "POSITION_SIZE")) return entry.positionSizeHistory;
    if (strEquals(field, "PNL_ESTIMATE")) return entry.pnlEstimateHistory;
    
    // 默认回退（返回成交价历史）
    return entry.tradePriceHistory;
}

// 滑动平均
ap_uint<32> PricingEngine::getMovingAvg(ap_uint<8> symbolIndex, const char* field, ap_uint<8> window = 8, int level = 0) {
#pragma HLS INLINE off
    return getBuffer(cache[symbolIndex], field, level).movingAvg(window);
}

// 指数加权平均
ap_uint<32> PricingEngine::getExpAvg(ap_uint<8> symbolIndex, const char* field, ap_uint<8> alpha = 32, ap_uint<8> window = 8, int level = 0) {
#pragma HLS INLINE off
    return getBuffer(cache[symbolIndex], field, level).expAvg(alpha, window);
}

// 最大值
ap_uint<32> PricingEngine::getMovingMax(ap_uint<8> symbolIndex, const char* field, ap_uint<8> window = 8, int level = 0) {
#pragma HLS INLINE off
    return getBuffer(cache[symbolIndex], field, level).movingMax(window);
}

// 最小值
ap_uint<32> PricingEngine::getMovingMin(ap_uint<8> symbolIndex, const char* field, ap_uint<8> window = 8, int level = 0) {
#pragma HLS INLINE off
    return getBuffer(cache[symbolIndex], field, level).movingMin(window);
}

// 求和
ap_uint<32> PricingEngine::getMovingSum(ap_uint<8> symbolIndex, const char* field, ap_uint<8> window = 8, int level = 0) {
#pragma HLS INLINE off
    return getBuffer(cache[symbolIndex], field, level).movingSum(window);
}

// 时间导数
ap_uint<32> PricingEngine::getDerivative(ap_uint<8> symbolIndex, const char* field, int level = 0) {
#pragma HLS INLINE off
    return getBuffer(cache[symbolIndex], field, level).derivative();
}

