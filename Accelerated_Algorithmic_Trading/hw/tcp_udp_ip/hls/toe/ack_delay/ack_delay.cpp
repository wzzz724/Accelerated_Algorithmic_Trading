/************************************************
 * Copyright (c) 2016, 2020 Xilinx, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ************************************************/

#include "ack_delay.hpp"
/**
 * @ingroup ack_delay
 * 
 * This block sits between the @ref event_engine and the 
 * @ref tx_engine, and provides delayed ACK functionality
 * by filtering out ACK events and posting them some time
 * later.
 *
 * @param[in]  input          Stream of events from the @ref event_engine
 * @param[out] output         Stream of events to the @ref tx_engine
 * @param[out] readCountFifo  One item per read sent back to @ref event_engine
 * @param[out] writeCountFifo One item per write sent back to @ref event_engine
 *
 */
void ack_delay(hls::stream<extendedEvent>& input,
               hls::stream<extendedEvent>& output,
               hls::stream<ap_uint<16> > &releaseSession,
               hls::stream<ap_uint<1> >&   readCountFifo,
               hls::stream<ap_uint<1> >&   writeCountFifo) {
#pragma HLS PIPELINE II = 2

    // Counters for each session. the counter value varies with MAX_SESSIONS. 
	// A single counter value will be decremented ~every MAX_SESSIONS clock cycles.
	// The counter width should be the maximum width of TIME_64us.
	// For 32 MAX_SESSIONS this value is approx 600
	// Until constexpr supported set width to 10
    static ap_uint<10> ack_table[MAX_SESSIONS];
	
	// Hold any requests to clear a counter in the table
	// This is serviced in the decrement loop as if we zero a pending ack_delay
	// and then in the next clock cycle decrement the counter (1 clock cycle latency in RAM)
	// The decremented value will overwrite the zeroed value.
	static bool clear_table[MAX_SESSIONS];
    // clang-format off
    #pragma HLS RESOURCE variable=ack_table core=RAM_2P_BRAM
    // clang-format on
    static ap_uint<16> ad_pointer = 0;

    extendedEvent ev;

    if (!input.empty() || !releaseSession.empty()) {
        ap_uint<10> time = 0;
        if (!input.empty()) {
            input.read(ev);
            readCountFifo.write(1);
            // Check if there is a delayed ACK
            // If event is an ACK and there is no delayed ACK in progress
            if (ev.type == ACK && ack_table[ev.sessionID] == 0) {
                // Set the timer for the ACK to be sent out
                // We operate on an II = 2 because of RAM dependency - so use half the counter value
                time = TIME_64us / 2;
            } else {
                // Assumption no SYN/RST
                // turn off the timer for the session
                time = 0;
                // Pass event straight through to the output
                output.write(ev);
                writeCountFifo.write(1);
            }
			ack_table[ev.sessionID] = time;			
        } else {
            releaseSession.read(ev.sessionID);
			clear_table[ev.sessionID] = true;
        }
    } else {
		ap_uint<10> value = 0;
        // "idle" loop - service the timers
		if (clear_table[ad_pointer]) {
			clear_table[ad_pointer] = false;
		} else {
			if (ack_table[ad_pointer] > 0 && !output.full()) {
				if (ack_table[ad_pointer] == 1) {
					// ad_pointer here is proxy for original
					// event sessionID
					output.write(event(ACK, ad_pointer));
					writeCountFifo.write(1);
				}
				// Decrease value				
				value = ack_table[ad_pointer] - 1;
			}
        }
		ack_table[ad_pointer] = value;

        // move the index round the buffer
        ad_pointer++;
        if (ad_pointer == MAX_SESSIONS) {
            ad_pointer = 0;
        }
    }
}
