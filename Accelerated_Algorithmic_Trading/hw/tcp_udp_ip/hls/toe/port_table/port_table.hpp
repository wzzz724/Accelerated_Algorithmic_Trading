/************************************************
 * Copyright (c) 2016, 2020 Xilinx, Inc.
 * All rights reserved.
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

#include "../toe_internals.hpp"

/** @defgroup port_table Port Table
 *  @ingroup tcp_module
 *
 * The Port Table module maintains a list of available and open ports
 * both for active and passive connections. See port_table().
 *
 * For active port opening, there is a fixed list of ports. These are
 * written to the @ref tx_app_if. When a port is closed, the port table
 * puts it back in the list. The @ref tx_app_if takes ports off the head
 * of the list as it needs them and notifies the port table when it closes
 * a port through the path from the @ref session_lookup_controller.
 */
void port_table(hls::stream<ap_uint<16> >& rxEng2portTable_check_req,
                hls::stream<ap_uint<16> >& rxApp2portTable_listen_req,
                // stream<ap_uint<1> >&		txApp2portTable_port_req,
                hls::stream<ap_uint<16> >& sLookup2portTable_releasePort,
                hls::stream<bool>&         portTable2rxEng_check_rsp,
                hls::stream<bool>&         portTable2rxApp_listen_rsp,
                hls::stream<ap_uint<16> >& portTable2txApp_port_rsp);
