/*
 * Copyright (c) 2020, Xilinx, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE#
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once
#include <stdint.h>

//Copied from hlslib by Johannes de Fine Licht https://github.com/definelicht/hlslib/blob/master/include/hlslib/xilinx/Utility.h
/*constexpr unsigned long ConstLog2(unsigned long val) {
  return val == 1 ? 0 : 1 + ConstLog2(val >> 1);
}*/

//const uint16_t MSS = ${TCP_STACK_MSS};
const uint16_t MSS = 1460;
//const uint16_t MAX_SESSIONS = ${TCP_STACK_MAX_SESSIONS};
const uint16_t MAX_SESSIONS = 32;
//const unsigned DATA_WIDTH = ${DATA_WIDTH} * 8;
const unsigned DATA_WIDTH = 8 * 8;
//const unsigned DATA_WIDTH_BITS = ConstLog2(DATA_WIDTH);
//const unsigned DATA_KEEP_BITS = ConstLog2(DATA_WIDTH/8);
const unsigned DATA_WIDTH_BITS = 6;
const unsigned DATA_KEEP_BITS = 3;

// TCP_NODELAY flag, to disable Nagle's Algorithm
//#define TCP_NODELAY ${TCP_STACK_NODELAY_EN}
#define TCP_NODELAY 1

// RX_DDR_BYPASS flag, to enable DDR bypass on RX path
//#define RX_DDR_BYPASS ${TCP_STACK_RX_DDR_BYPASS_EN}
#ifndef RX_DDR_BYPASS
#define RX_DDR_BYPASS 1
#endif

//TCP fast recovery/fast retransmit
//#define FAST_RETRANSMIT ${TCP_STACK_FAST_RETRANSMIT_EN}
#define FAST_RETRANSMIT 1

//TCP window scaling option
//#define WINDOW_SCALE ${TCP_STACK_WINDOW_SCALING_EN}
#define WINDOW_SCALE 0
const unsigned WINDOW_SCALE_BITS = 2;
#if (WINDOW_SCALE)
const unsigned WINDOW_BITS = 16 + WINDOW_SCALE_BITS;
#else
const unsigned WINDOW_BITS = 16;
#endif


const unsigned BUFFER_SIZE = (1 << WINDOW_BITS);
const unsigned CONGESTION_WINDOW_MAX = (BUFFER_SIZE - 2048);
