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


#ifndef XLNX_NETWORK_CAPTURE_ADDRESS_MAP_H
#define XLNX_NETWORK_CAPTURE_ADDRESS_MAP_H




 /* The following are the standard kernel control registers */
#define XLNX_NETWORK_CAPTURE_KERNEL_CONTROL_OFFSET						        (0x00000000)
#define XLNX_NETWORK_CAPTURE_KERNEL_GLOBAL_INTERRUPT_ENABLE_OFFSET		        (0x00000004)
#define XLNX_NETWORK_CAPTURE_KERNEL_IP_INTERRUPT_ENABLE_OFFSET			        (0x00000008)
#define XLNX_NETWORK_CAPTURE_KERNEL_IP_INTERRUPT_STATUS_OFFSET			        (0x0000000C)


#define XLNX_NETWORK_CAPTURE_ENABLE_PROCESSING_OFFSET                           (0x00000010)

#define XLNX_NETWORK_CAPTURE_BUFFER_TAIL_POINTER_OFFSET                         (0x00000050)


#define XLNX_NETWORK_CAPTURE_BUFFER_ADDRESS_LOWER_WORD_OFFSET                   (0x000000B0)
#define XLNX_NETWORK_CAPTURE_BUFFER_ADDRESS_UPPER_WORD_OFFSET                   (0x000000B4)




#endif

