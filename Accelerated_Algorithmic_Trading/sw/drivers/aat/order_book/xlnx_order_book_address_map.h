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


#ifndef XLNX_ORDER_BOOK_ADDRESS_MAP_H
#define XLNX_ORDER_BOOK_ADDRESS_MAP_H


 /* The following are the standard kernel control registers */
#define XLNX_ORDER_BOOK_KERNEL_CONTROL_OFFSET						(0x00000000)
#define XLNX_ORDER_BOOK_KERNEL_GLOBAL_INTERRUPT_ENABLE_OFFSET		(0x00000004)
#define XLNX_ORDER_BOOK_KERNEL_IP_INTERRUPT_ENABLE_OFFSET			(0x00000008)
#define XLNX_ORDER_BOOK_KERNEL_IP_INTERRUPT_STATUS_OFFSET			(0x0000000C)


#define XLNX_ORDER_BOOK_RESET_CONTROL_OFFSET                        (0x00000010)
#define XLNX_ORDER_BOOK_SIZE_CONTROL_OFFSET                         (0x00000018)
#define XLNX_ORDER_BOOK_CAPTURE_CONTROL_OFFSET                      (0x00000020)


#define XLNX_ORDER_BOOK_STATUS_FLAGS_OFFSET                         (0x00000050)


#define XLNX_ORDER_BOOK_STATS_RX_OPERATIONS_COUNT_OFFSET            (0x00000058)
#define XLNX_ORDER_BOOK_STATS_PROCESS_OPERATIONS_COUNT_OFFSET       (0x00000068)
#define XLNX_ORDER_BOOK_STATS_INVALID_OPERATIONS_COUNT_OFFSET       (0x00000078)
#define XLNX_ORDER_BOOK_RESPONSES_GENERATED_COUNT_OFFSET            (0x00000088)
#define XLNX_ORDER_BOOK_RESPONSE_SENT_COUNT_OFFSET                  (0x00000098)
#define XLNX_ORDER_BOOK_ADD_OPERATIONS_COUNT_OFFSET                 (0x000000A8)
#define XLNX_ORDER_BOOK_MODIFY_OPERATIONS_COUNT_OFFSET              (0x000000B8)
#define XLNX_ORDER_BOOK_DELETE_OPERATIONS_COUNT_OFFSET              (0x000000C8)
#define XLNX_ORDER_BOOK_TRANSACT_OPERATIONS_COUNT_OFFSET            (0x000000D8)
#define XLNX_ORDER_BOOK_HALT_OPERATIONS_COUNT_OFFSET                (0x000000E8)
#define XLNX_ORDER_BOOK_TIMESTAMP_ERRORS_COUNT_OFFSET               (0x000000F8)
#define XLNX_ORDER_BOOK_UNHANDLED_OP_CODES_COUNT_OFFSET             (0x00000108)
#define XLNX_ORDER_BOOK_SYMBOL_ERRORS_COUNT_OFFSET                  (0x00000118)
#define XLNX_ORDER_BOOK_DIRECTION_ERRORS_COUNT_OFFSET               (0x00000128)
#define XLNX_ORDER_BOOK_LEVEL_ERRORS_COUNT_OFFSET                   (0x00000138)
#define XLNX_ORDER_BOOK_CLOCK_TICK_GEN_EVENTS_COUNT_OFFSET          (0x00000148)


#define XLNX_ORDER_BOOK_DATA_OFFSET                                 (0x00000190)












#endif
