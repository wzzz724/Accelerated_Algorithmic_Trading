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

#ifndef XLNX_ETHERNET_ADDRESS_MAP_H
#define XLNX_ETHERNET_ADDRESS_MAP_H





/* The following are the standard kernel control registers */
#define XLNX_ETHERNET_KERNEL_CONTROL_OFFSET						                (0x00000000)
#define XLNX_ETHERNET_KERNEL_GLOBAL_INTERRUPT_ENABLE_OFFSET		                (0x00000004)
#define XLNX_ETHERNET_KERNEL_IP_INTERRUPT_ENABLE_OFFSET			                (0x00000008)
#define XLNX_ETHERNET_KERNEL_IP_INTERRUPT_STATUS_OFFSET			                (0x0000000C)


#define XLNX_ETHERNET_KERNEL_RESET_CONTROL_OFFSET                               (0x00000010)


//NOTE - Originally this block only supported 2-channels. It was later increased to 4-channels.
//       However, there was not enough room in the register map to squeeze in all the new registers for the new channels within their appropriate groupings.
//       So to keep the register map backwardly compatible with the old 2-channel version, all the registers for the new channels start
//       AFTER all of the old registers but still follow the same layout at the old registers. 
//       See the following diagram:
//         
//      +===================================+
//      |   Channel 0 Control Registers     |
//      +-----------------------------------+
//      |   Channel 1 Control Registers     |
//      +-----------------------------------+
//      |   Channel 0 Status Registers      |
//      +-----------------------------------+
//      |   Channel 1 Status Registers      |
//      +-----------------------------------+
//      |   Channel 0 GT Control Registers  |
//      +-----------------------------------+
//      |   Channel 1 GT Control Registers  |
//      +===================================+
//      |   Channel 2 Control Registers     |
//      +-----------------------------------+
//      |   Channel 3 Control Registers     |
//      +-----------------------------------+
//      |   Channel 2 Status Registers      |
//      +-----------------------------------+
//      |   Channel 3 Status Registers      |
//      +-----------------------------------+
//      |   Channel 2 GT Control Registers  |
//      +-----------------------------------+
//      |   Channel 3 GT Control Registers  |
//      +===================================+
//
//       So to index via channel, we need to:
//       (1) Figure out which "half" of the register map to access based on the channel
//       (2) Adjust the channel number so that any further multipliers are correct when starting relative to the mid-point
//
//

#define XLNX_ETHERNET_CHANNEL_0_1_START                                         (0x00000020)
#define XLNX_ETHERNET_CHANNEL_2_3_START                                         (0x00000180)


#define XLNX_ETHERNET_CHANNEL_GROUP_START(CHANNEL)                              ((CHANNEL < 2) ? XLNX_ETHERNET_CHANNEL_0_1_START : XLNX_ETHERNET_CHANNEL_2_3_START)
#define XLNX_ETHERNET_CONVERT_CHANNEL_FOR_GROUP(CHANNEL)                        (CHANNEL & 0x01)


#define XLNX_ETHERNET_CHANNEL_CONTROL_START_OFFSET                              (0x00000000)
#define XLNX_ETHERNET_CHANNEL_CONTROL_MULTIPLIER                                (0x00000020)
#define XLNX_ETHERNET_CHANNEL_CONTROL_OFFSET(CHANNEL)                           (XLNX_ETHERNET_CHANNEL_GROUP_START(CHANNEL) + XLNX_ETHERNET_CHANNEL_CONTROL_START_OFFSET + (XLNX_ETHERNET_CHANNEL_CONTROL_MULTIPLIER * (XLNX_ETHERNET_CONVERT_CHANNEL_FOR_GROUP(CHANNEL))))


#define XLNX_ETHERNET_CHANNEL_STATUS_START_OFFSET                               (0x00000060)
#define XLNX_ETHERNET_CHANNEL_STATUS_MULTIPLIER                                 (0x00000040)
#define XLNX_ETHERNET_CHANNEL_STATUS_OFFSET(CHANNEL)                            (XLNX_ETHERNET_CHANNEL_GROUP_START(CHANNEL) + XLNX_ETHERNET_CHANNEL_STATUS_START_OFFSET + (XLNX_ETHERNET_CHANNEL_STATUS_MULTIPLIER * (XLNX_ETHERNET_CONVERT_CHANNEL_FOR_GROUP(CHANNEL))))


#define XLNX_ETHERNET_GT_CONTROL_START_OFFSET                                   (0x000000E0)
#define XLNX_ETHERNET_GT_CONTROL_MULTIPLIER                                     (0x00000040)
#define XLNX_ETHERNET_GT_CONTROL_OFFSET(CHANNEL)                                (XLNX_ETHERNET_CHANNEL_GROUP_START(CHANNEL) + XLNX_ETHERNET_GT_CONTROL_START_OFFSET + (XLNX_ETHERNET_GT_CONTROL_MULTIPLIER * (XLNX_ETHERNET_CONVERT_CHANNEL_FOR_GROUP(CHANNEL))))














//Channel Status Registers
#define XLNX_ETHERNET_CHANNEL_STATUS_BLOCK_LOCK_OFFSET(CHANNEL)                 (XLNX_ETHERNET_CHANNEL_STATUS_OFFSET(CHANNEL) + 0x00000000)
#define XLNX_ETHERNET_CHANNEL_STATUS_RX_TRAFFIC_PROC_STATUS_OFFSET(CHANNEL)     (XLNX_ETHERNET_CHANNEL_STATUS_OFFSET(CHANNEL) + 0x00000004)
#define XLNX_ETHERNET_CHANNEL_STATUS_TX_TRAFFIC_PROC_STATUS_OFFSET(CHANNEL)     (XLNX_ETHERNET_CHANNEL_STATUS_OFFSET(CHANNEL) + 0x00000008)

#define XLNX_ETHERNET_CHANNEL_STATUS_RX_OVERFLOW_COUNT_OFFSET(CHANNEL)          (XLNX_ETHERNET_CHANNEL_STATUS_OFFSET(CHANNEL) + 0x00000010)
#define XLNX_ETHERNET_CHANNEL_STATUS_RX_UNICAST_DROP_COUNT_OFFSET(CHANNEL)      (XLNX_ETHERNET_CHANNEL_STATUS_OFFSET(CHANNEL) + 0x00000014)
#define XLNX_ETHERNET_CHANNEL_STATUS_RX_MULTICAST_DROP_COUNT_OFFSET(CHANNEL)    (XLNX_ETHERNET_CHANNEL_STATUS_OFFSET(CHANNEL) + 0x00000018)
#define XLNX_ETHERNET_CHANNEL_STATUS_RX_OVERSIZED_DROP_COUNT_OFFSET(CHANNEL)    (XLNX_ETHERNET_CHANNEL_STATUS_OFFSET(CHANNEL) + 0x0000001C)
#define XLNX_ETHERNET_CHANNEL_STATUS_TX_FIFO_UNDERFLOW_COUNT_OFFSET(CHANNEL)    (XLNX_ETHERNET_CHANNEL_STATUS_OFFSET(CHANNEL) + 0x00000020)




//Channel Control Registers
#define XLNX_ETHERNET_CHANNEL_CONTROL_MAC_ADDRESS_LOWER_OFFSET(CHANNEL)         (XLNX_ETHERNET_CHANNEL_CONTROL_OFFSET(CHANNEL) + 0x00000000)
#define XLNX_ETHERNET_CHANNEL_CONTROL_MAC_ADDRESS_UPPER_OFFSET(CHANNEL)         (XLNX_ETHERNET_CHANNEL_CONTROL_OFFSET(CHANNEL) + 0x00000004)
#define XLNX_ETHERNET_CHANNEL_CONTROL_TRAFFIC_PROC_RESET_OFFSET(CHANNEL)        (XLNX_ETHERNET_CHANNEL_CONTROL_OFFSET(CHANNEL) + 0x00000008)
#define XLNX_ETHERNET_CHANNEL_CONTROL_TRAFFIC_PROC_CONFIG_OFFSET(CHANNEL)       (XLNX_ETHERNET_CHANNEL_CONTROL_OFFSET(CHANNEL) + 0x0000000C)
#define XLNX_ETHERNET_CHANNEL_CONTROL_TX_FIFO_THRESHOLD_OFFSET(CHANNEL)         (XLNX_ETHERNET_CHANNEL_CONTROL_OFFSET(CHANNEL) + 0x00000010)




//GT Control Registers
#define XLNX_ETHERNET_GT_CONTROL_CLOCK_SELECT_OFFSET(CHANNEL)                   (XLNX_ETHERNET_GT_CONTROL_OFFSET(CHANNEL) + 0x00000000)
#define XLNX_ETHERNET_GT_CONTROL_LOOPBACK_OFFSET(CHANNEL)                       (XLNX_ETHERNET_GT_CONTROL_OFFSET(CHANNEL) + 0x00000004)
#define XLNX_ETHERNET_GT_CONTROL_POLARITY_OFFSET(CHANNEL)                       (XLNX_ETHERNET_GT_CONTROL_OFFSET(CHANNEL) + 0x00000008)
#define XLNX_ETHERNET_GT_CONTROL_TX_PRBS_CONTROL_OFFSET(CHANNEL)                (XLNX_ETHERNET_GT_CONTROL_OFFSET(CHANNEL) + 0x0000000C)
#define XLNX_ETHERNET_GT_CONTROL_RX_PRBS_CONTROL_OFFSET(CHANNEL)                (XLNX_ETHERNET_GT_CONTROL_OFFSET(CHANNEL) + 0x00000010)
#define XLNX_ETHERNET_GT_CONTROL_PRBS_STATUS_OFFSET(CHANNEL)                    (XLNX_ETHERNET_GT_CONTROL_OFFSET(CHANNEL) + 0x00000014)
#define XLNX_ETHERNET_GT_CONTROL_TX_RESET_INIT_CONTROL_OFFSET(CHANNEL)          (XLNX_ETHERNET_GT_CONTROL_OFFSET(CHANNEL) + 0x00000018)
#define XLNX_ETHERNET_GT_CONTROL_TX_RESET_INIT_STATUS_OFFSET(CHANNEL)           (XLNX_ETHERNET_GT_CONTROL_OFFSET(CHANNEL) + 0x0000001C)
#define XLNX_ETHERNET_GT_CONTROL_RX_RESET_INIT_CONTROL_OFFSET(CHANNEL)          (XLNX_ETHERNET_GT_CONTROL_OFFSET(CHANNEL) + 0x00000020)
#define XLNX_ETHERNET_GT_CONTROL_RX_RESET_INIT_STATUS_OFFSET(CHANNEL)           (XLNX_ETHERNET_GT_CONTROL_OFFSET(CHANNEL) + 0x00000024)
#define XLNX_ETHERNET_GT_CONTROL_TX_DRIVER_OFFSET(CHANNEL)                      (XLNX_ETHERNET_GT_CONTROL_OFFSET(CHANNEL) + 0x00000028)
#define XLNX_ETHERNET_GT_CONTROL_RX_EQUALIZER_OFFSET(CHANNEL)                   (XLNX_ETHERNET_GT_CONTROL_OFFSET(CHANNEL) + 0x0000002C)
#define XLNX_ETHERNET_GT_CONTROL_RX_MARGIN_ANALYSIS_OFFSET(CHANNEL)             (XLNX_ETHERNET_GT_CONTROL_OFFSET(CHANNEL) + 0x00000030)




#endif

