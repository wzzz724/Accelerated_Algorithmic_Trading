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


#ifndef XLNX_ORDER_BOOK_DATA_MOVER_ADDRESS_MAP_H
#define XLNX_ORDER_BOOK_DATA_MOVER_ADDRESS_MAP_H


 /* The following are the standard kernel control registers */
#define XLNX_ORDER_BOOK_DATA_MOVER_KERNEL_CONTROL_OFFSET						(0x00000000)
#define XLNX_ORDER_BOOK_DATA_MOVER_KERNEL_GLOBAL_INTERRUPT_ENABLE_OFFSET		(0x00000004)
#define XLNX_ORDER_BOOK_DATA_MOVER_KERNEL_IP_INTERRUPT_ENABLE_OFFSET			(0x00000008)
#define XLNX_ORDER_BOOK_DATA_MOVER_KERNEL_IP_INTERRUPT_STATUS_OFFSET			(0x0000000C)

#define XLNX_ORDER_BOOK_DATA_MOVER_CTRL_OFFSET                                  (0x00000010)
#define XLNX_ORDER_BOOK_DATA_MOVER_RING_READ_BUFFER_HEAD_INDEX_OFFSET           (0x00000018)
#define XLNX_ORDER_BOOK_DATA_MOVER_RING_WRITE_BUFFER_TAIL_INDEX_OFFSET          (0x00000020)

#define XLNX_ORDER_BOOK_DATA_MOVER_THROTTLE_RATE_OFFSET                         (0x00000028)


#define XLNX_ORDER_BOOK_DATA_MOVER_STATUS_OFFSET                                (0x00000050)
#define XLNX_ORDER_BOOK_DATA_MOVER_RING_READ_BUFFER_TAIL_INDEX_OFFSET           (0x00000058)
#define XLNX_ORDER_BOOK_DATA_MOVER_TX_RESPONSE_INDEX_OFFSET                     (0x00000068)
#define XLNX_ORDER_BOOK_DATA_MOVER_RING_WRITE_BUFFER_HEAD_INDEX_OFFSET          (0x00000078)
#define XLNX_ORDER_BOOK_DATA_MOVER_NUM_RX_OP_OFFSET                             (0x00000088)

/* latency */
#define XLNX_ORDER_BOOK_DATA_MOVER_LATENCY_MIN_OFFSET                           (0x00000098)
#define XLNX_ORDER_BOOK_DATA_MOVER_LATENCY_MAX_OFFSET                           (0x000000A8)
#define XLNX_ORDER_BOOK_DATA_MOVER_LATENCY_SUM_OFFSET                           (0x000000B8)
#define XLNX_ORDER_BOOK_DATA_MOVER_LATENCY_CNT_OFFSET                           (0x000000C8)
#define XLNX_ORDER_BOOK_DATA_MOVER_CYCLES_PRE_OFFSET                            (0x000000D8)
#define XLNX_ORDER_BOOK_DATA_MOVER_CYCLES_POST_OFFSET                           (0x000000E8)

#define XLNX_ORDER_BOOK_DATA_MOVER_THROTTLE_COUNT_OFFSET                        (0x000000F8)
#define XLNX_ORDER_BOOK_DATA_MOVER_TROTTLE_EVENT_OFFSET                         (0x00000108)


#define XLNX_ORDER_BOOK_DATA_MOVER_READ_BUFFER_ADDRESS_LOWER_WORD_OFFSET        (0x00000130)
#define XLNX_ORDER_BOOK_DATA_MOVER_READ_BUFFER_ADDRESS_UPPER_WORD_OFFSET        (0x00000134)
#define XLNX_ORDER_BOOK_DATA_MOVER_WRITE_BUFFER_ADDRESS_LOWER_WORD_OFFSET       (0x0000013C)
#define XLNX_ORDER_BOOK_DATA_MOVER_WRITE_BUFFER_ADDRESS_UPPER_WORD_OFFSET       (0x00000140)


#endif

