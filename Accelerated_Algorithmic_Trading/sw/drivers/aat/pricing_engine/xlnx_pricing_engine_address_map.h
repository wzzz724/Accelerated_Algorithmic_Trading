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


#ifndef XLNX_PRICING_ENGINE_ADDRESS_MAP_H
#define XLNX_PRICING_ENGINE_ADDRESS_MAP_H




 /* The following are the standard kernel control registers */
#define XLNX_PRICING_ENGINE_KERNEL_CONTROL_OFFSET						(0x00000000)
#define XLNX_PRICING_ENGINE_KERNEL_GLOBAL_INTERRUPT_ENABLE_OFFSET		(0x00000004)
#define XLNX_PRICING_ENGINE_KERNEL_IP_INTERRUPT_ENABLE_OFFSET			(0x00000008)
#define XLNX_PRICING_ENGINE_KERNEL_IP_INTERRUPT_STATUS_OFFSET			(0x0000000C)


#define XLNX_PRICING_ENGINE_RESET_CONTROL_OFFSET                        (0x00000010)

#define XLNX_PRICING_ENGINE_CAPTURE_CONTROL_OFFSET                      (0x00000020)

#define XLNX_PRICING_ENGINE_GLOBAL_STRATEGY_CONTOL_OFFSET				(0x00000028)

#define XLNX_PRICING_ENGINE_STATUS_FLAGS_OFFSET                         (0x00000050)

#define XLNX_PRICING_ENGINE_STATS_RX_RESPONSE_COUNT_OFFSET              (0x00000058)
#define XLNX_PRICING_ENGINE_STATS_PROCESSED_RESPONSES_COUNT_OFFSET      (0x00000068)
#define XLNX_PRICING_ENGINE_STATS_TX_OPERATIONS_COUNT_OFFSET            (0x00000078)
#define XLNX_PRICING_ENGINE_STATS_STRATEGY_NONE_COUNT_OFFSET            (0x00000088)
#define XLNX_PRICING_ENGINE_STATS_STRATEGY_PEG_COUNT_OFFSET             (0x00000098)
#define XLNX_PRICING_ENGINE_STATS_STRATEGY_LIMIT_COUNT_OFFSET           (0x000000A8)
#define XLNX_PRICING_ENGINE_STATS_STRATEGY_UNKNOWN_COUNT_OFFSET         (0x000000B8)
#define XLNX_PRICING_ENGINE_STATS_CLOCK_TICK_EVENTS_COUNT_OFFSET        (0x000000C8)


#define XLNX_PRICING_ENGINE_CAPTURE_OFFSET					            (0x00000110)
#define XLNX_PRICING_ENGINE_NUM_CAPTURE_REGISTERS                       (6)





















#endif

