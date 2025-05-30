/************************************************
Copyright (c) 2019, Systems Group, ETH Zurich.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************/
#include "hash_table.hpp"

htUpdateReq<64, 16> axiu2htUpdateReq(ap_axiu<128, 0, 0, 0> inword) {
#pragma HLS inline
    htUpdateReq<64, 16> tmp_htUpdateReq;
    if (inword.data(0, 0) == 0) {
    	tmp_htUpdateReq.op = KV_INSERT;
    } else if (inword.data(0, 0) == 1) {
    	tmp_htUpdateReq.op = KV_DELETE;
    }
    tmp_htUpdateReq.key = inword.data(64, 1);
    tmp_htUpdateReq.value = inword.data(80, 65);
    tmp_htUpdateReq.source = inword.data(88, 81);
    return tmp_htUpdateReq;
}

ap_axiu<128, 0, 0, 0> htUpdateResp2axiu(htUpdateResp<64,16> inword) {
#pragma HLS inline
    ap_axiu<128, 0, 0, 0> tmp_axiu;
    tmp_axiu.data(0, 0) = inword.op;
    tmp_axiu.data(64, 1) = inword.key;
    tmp_axiu.data(80, 65) = inword.value;
    tmp_axiu.data(88, 81) = inword.success;
    tmp_axiu.data(96, 89) = inword.source;
    tmp_axiu.data(127, 97) = 0;
    tmp_axiu.last = 1;
    return tmp_axiu;
}

htLookupReq<64> axiu2htLookupReq(ap_axiu<128, 0, 0, 0> inword) {
#pragma HLS inline
    htLookupReq<64> tmp_htLookupReq;
    tmp_htLookupReq.key = inword.data(63, 0);
    tmp_htLookupReq.source = inword.data(71, 64);
    return tmp_htLookupReq;
}

ap_axiu<128, 0, 0, 0> htLookupResp2axiu(htLookupResp<64, 16> inword) {
#pragma HLS inline
    ap_axiu<128, 0, 0, 0> tmp_axiu;
    tmp_axiu.data(63, 0) = inword.key;
    tmp_axiu.data(79, 64) = inword.value;
    tmp_axiu.data(87, 80) = inword.hit;
    tmp_axiu.data(95, 88) = inword.source;
    tmp_axiu.data(127, 96) = 0;
    tmp_axiu.last = 1;
    return tmp_axiu;
}

const ap_uint<MAX_ADDRESS_BITS> tabulation_table[NUM_TABLES][2][MAX_KEY_SIZE] = {
#include "tabulation_table.txt"
};

static htEntry<KEY_SIZE, VALUE_SIZE> cuckooTables[NUM_TABLES][TABLE_SIZE];

void calculate_hashes(ap_uint<KEY_SIZE> key, ap_uint<TABLE_ADDRESS_BITS> hashes[NUM_TABLES]) {
#pragma HLS ARRAY_PARTITION variable = hashes complete dim = 1

    for (int i = 0; i < NUM_TABLES; i++) {
#pragma HLS UNROLL

        ap_uint<TABLE_ADDRESS_BITS> hash = 0;
        for (int k = 0; k < KEY_SIZE; k++) {
#pragma HLS UNROLL
            ap_uint<MAX_ADDRESS_BITS> randomValue = tabulation_table[i][key[k]][k];
            hash ^= randomValue(TABLE_ADDRESS_BITS - 1, 0);
        }
        hashes[i] = hash;
    }
}

template <int K, int V>
htLookupResp<K, V> lookup(htLookupReq<K> request) {
#pragma HLS INLINE

    htEntry<K, V> currentEntries[NUM_TABLES];
#pragma HLS ARRAY_PARTITION variable = currentEntries complete
    ap_uint<TABLE_ADDRESS_BITS> hashes[NUM_TABLES];
    htLookupResp<K, V> response;
    response.key = request.key;
    response.source = request.source;
    response.hit = false;

    calculate_hashes(request.key, hashes);
    // Look for matching key
    int slot = -1;
    for (int i = 0; i < NUM_TABLES; i++) {
#pragma HLS UNROLL
        currentEntries[i] = cuckooTables[i][hashes[i]];
        if (currentEntries[i].valid && currentEntries[i].key == request.key) {
            slot = i;
        }
    }
    // Check if key found
    if (slot != -1) {
        response.value = currentEntries[slot].value;
        response.hit = true;
    }

    return response;
}

template <int K, int V>
htUpdateResp<K, V> insert(htUpdateReq<K, V> request, ap_uint<16>& regInsertFailureCount) {
#pragma HLS INLINE

    htEntry<K, V> currentEntries[NUM_TABLES];
#pragma HLS ARRAY_PARTITION variable = currentEntries complete
    ap_uint<TABLE_ADDRESS_BITS> hashes[NUM_TABLES];
    static ap_uint<8> victimIdx = 0;
    static ap_uint<1> victimBit = 0;
    htUpdateResp<K, V> response;
    response.op = request.op;
    response.key = request.key;
    response.value = request.value;
    response.source = request.source;
    response.success = false;
    static uint16_t insertFailureCounter = 0;

    regInsertFailureCount = insertFailureCounter;

    htEntry<K, V> currentEntry(request.key, request.value);
    victimIdx = 0;
// Try multiple times
insertLoop:
    for (int j = 0; j < MAX_TRIALS; j++) {
        calculate_hashes(currentEntry.key, hashes);
        // Look for free slot
        int slot = -1;
        for (int i = 0; i < NUM_TABLES; i++) {
#pragma HLS UNROLL
            currentEntries[i] = cuckooTables[i][hashes[i]];
            if (!currentEntries[i].valid) {
                slot = i;
            }
        }

        // If free slot
        if (slot != -1) {
            currentEntries[slot] = currentEntry;
            response.success = true;
        } else {
            // Evict existing entry and try to re-insert
            int victimPos = (hashes[victimIdx] % (NUM_TABLES - 1)) + victimBit;
            htEntry<K, V> victimEntry = currentEntries[victimPos]; // cuckooTables[victimPos][hashes[victimPos]];
            currentEntries[victimPos] = currentEntry;
            currentEntry = victimEntry;
            victimIdx++;
            if (victimIdx == NUM_TABLES) victimIdx = 0;
        }
        // Write currentEntries back
        for (int i = 0; i < NUM_TABLES; i++) {
#pragma HLS UNROLL
            cuckooTables[i][hashes[i]] = currentEntries[i];
        }

        victimBit++;
        if (response.success) break;
    } // for
    if (!response.success) {
        std::cout << "REACHED MAX TRIALS: " << request.key << " " << currentEntry.key << std::endl;
        insertFailureCounter++;
    }
    return response;
}

template <int K, int V>
htUpdateResp<K, V> remove(htUpdateReq<K, V> request) {
#pragma HLS INLINE

    htEntry<K, V> currentEntries[NUM_TABLES];
#pragma HLS ARRAY_PARTITION variable = currentEntries complete
    ap_uint<TABLE_ADDRESS_BITS> hashes[NUM_TABLES];
    htUpdateResp<K, V> response;
    response.op = request.op;
    response.key = request.key;
    response.value = request.value;
    response.source = request.source;
    response.success = false;

    calculate_hashes(request.key, hashes);
    // Look for matching key
    for (int i = 0; i < NUM_TABLES; i++) {
#pragma HLS UNROLL
        currentEntries[i] = cuckooTables[i][hashes[i]];
        if (currentEntries[i].valid && currentEntries[i].key == request.key) {
            currentEntries[i].valid = false;
            response.success = true;
        }
        cuckooTables[i][hashes[i]] = currentEntries[i];
    }

    return response;
}

template <int K, int V>
void hash_table_body(hls::stream<ap_axiu<128, 0, 0, 0> >& s_axis_lup_req,
                     hls::stream<ap_axiu<128, 0, 0, 0> >& s_axis_upd_req,
					 hls::stream<ap_axiu<128, 0, 0, 0> >& m_axis_lup_rsp,
                     hls::stream<ap_axiu<128, 0, 0, 0> >& m_axis_upd_rsp,
                     ap_uint<16>& regInsertFailureCount)

{
#pragma HLS INLINE

// Global arrays
#pragma HLS ARRAY_PARTITION variable = tabulation_table complete dim = 1
#pragma HLS RESOURCE variable = cuckooTables core = RAM_2P_BRAM
#pragma HLS ARRAY_PARTITION variable = cuckooTables complete dim = 1

    if (!s_axis_lup_req.empty()) {
        htLookupReq<K> request = axiu2htLookupReq(s_axis_lup_req.read());
        htLookupResp<K, V> response = lookup<K, V>(request);
        m_axis_lup_rsp.write(htLookupResp2axiu(response));
    } else if (!s_axis_upd_req.empty()) {
        htUpdateReq<K, V> request = axiu2htUpdateReq(s_axis_upd_req.read());
        if (request.op == KV_INSERT) {
            htUpdateResp<K, V> response = insert<K, V>(request, regInsertFailureCount);
            m_axis_upd_rsp.write(htUpdateResp2axiu(response));
        } else // DELETE
        {
            htUpdateResp<K, V> response = remove<K, V>(request);
            m_axis_upd_rsp.write(htUpdateResp2axiu(response));
        }
    }
}

void hash_table(hls::stream<ap_axiu<128, 0, 0, 0> >& s_axis_lup_req,
                hls::stream<ap_axiu<128, 0, 0, 0> >& s_axis_upd_req,
				hls::stream<ap_axiu<128, 0, 0, 0> >& m_axis_lup_rsp,
                hls::stream<ap_axiu<128, 0, 0, 0> >& m_axis_upd_rsp,
                ap_uint<16>& regInsertFailureCount) {
#pragma HLS INTERFACE ap_ctrl_none port = return

#pragma HLS INTERFACE axis register port = s_axis_lup_req
#pragma HLS INTERFACE axis register port = s_axis_upd_req
#pragma HLS INTERFACE axis register port = m_axis_lup_rsp
#pragma HLS INTERFACE axis register port = m_axis_upd_rsp
#pragma HLS INTERFACE ap_stable port = regInsertFailureCount

    hash_table_body<KEY_SIZE, VALUE_SIZE>(s_axis_lup_req, s_axis_upd_req, m_axis_lup_rsp, m_axis_upd_rsp,
                                          regInsertFailureCount);
}
