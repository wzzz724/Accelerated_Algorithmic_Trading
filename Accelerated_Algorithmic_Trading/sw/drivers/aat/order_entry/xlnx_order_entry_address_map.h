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


#ifndef XLNX_ORDER_ENTRY_ADDRESS_MAP_H
#define XLNX_ORDER_ENTRY_ADDRESS_MAP_H




 /* The following are the standard kernel control registers */
#define XLNX_ORDER_ENTRY_KERNEL_CONTROL_OFFSET						(0x00000000)
#define XLNX_ORDER_ENTRY_KERNEL_GLOBAL_INTERRUPT_ENABLE_OFFSET		(0x00000004)
#define XLNX_ORDER_ENTRY_KERNEL_IP_INTERRUPT_ENABLE_OFFSET			(0x00000008)
#define XLNX_ORDER_ENTRY_KERNEL_IP_INTERRUPT_STATUS_OFFSET			(0x0000000C)


#define XLNX_ORDER_ENTRY_CONTROL_OFFSET								(0x00000010)

#define XLNX_ORDER_ENTRY_CAPTURE_CONTROL_OFFSET                     (0x00000020)

#define XLNX_ORDER_ENTRY_DESTINATION_IP_ADDRESS_OFFSET				(0x00000028)
#define XLNX_ORDER_ENTRY_DESTINATION_PORT_OFFSET					(0x00000030)

#define XLNX_ORDER_ENTRY_STATUS_FLAGS_OFFSET                        (0x00000050)

#define XLNX_ORDER_ENTRY_STATS_RX_OPERATIONS_COUNT_OFFSET           (0x00000058)
#define XLNX_ORDER_ENTRY_STATS_PROCESSED_OPERATIONS_COUNT_OFFSET    (0x00000068)
#define XLNX_ORDER_ENTRY_STATS_TX_DATA_FRAMES_COUNT_OFFSET          (0x00000078)
#define XLNX_ORDER_ENTRY_STATS_TX_META_FRAMES_COUNT_OFFSET          (0x00000088)
#define XLNX_ORDER_ENTRY_STATS_TX_MESSAGES_COUNT_OFFSET             (0x00000098)
#define XLNX_ORDER_ENTRY_STATS_RX_DATA_FRAMES_COUNT_OFFSET          (0x000000A8)
#define XLNX_ORDER_ENTRY_STATS_RX_META_FRAMES_COUNT_OFFSET          (0x000000B8)
#define XLNX_ORDER_ENTRY_STATS_CLOCK_TICK_EVENTS_COUNT_OFFSET       (0x000000C8)
#define XLNX_ORDER_ENTRY_STATS_TX_DROPPED_MSG_COUNT_OFFSET          (0x000000D8)


#define XLNX_ORDER_ENTRY_TX_STATUS_OFFSET                           (0x000000E8)

#define XLNX_ORDER_ENTRY_STATS_NOTIFICATIONS_RECEIVED_COUNT_OFFSET  (0x000000F8)
#define XLNX_ORDER_ENTRY_STATS_READ_REQUESTS_SENT_COUNT_OFFSET      (0x00000108)

#define XLNX_ORDER_ENTRY_CAPTURE_OFFSET					            (0x00000138)




















#endif

