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

#ifndef XLNX_SHELL_AAT_OBJECTS_H
#define XLNX_SHELL_AAT_OBJECTS_H

#include <cinttypes>

#include "xlnx_shell.h"

#include "xlnx_shell_ethernet.h"
#include "xlnx_shell_feed_handler.h"
#include "xlnx_shell_order_book.h"
#include "xlnx_shell_order_book_data_mover.h"
#include "xlnx_shell_order_entry.h"
#include "xlnx_shell_pricing_engine.h"
#include "xlnx_shell_tcp_udp_ip.h"
#include "xlnx_shell_clock_tick_generator.h"
#include "xlnx_shell_line_handler.h"

using namespace XLNX;





extern CommandTableElement XLNX_AAT_COMMAND_TABLE[];
extern const uint32_t XLNX_AAT_COMMAND_TABLE_LENGTH;





#endif

