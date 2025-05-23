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



#ifndef XLNX_CLOCK_TICK_GENERATOR_ADDRESS_MAP_H
#define XLNX_CLOCK_TICK_GENERATOR_ADDRESS_MAP_H




 /* The following are the standard kernel control registers */
#define XLNX_CLOCK_TICK_GENERATOR_KERNEL_CONTROL_OFFSET						(0x00000000)
#define XLNX_CLOCK_TICK_GENERATOR_KERNEL_GLOBAL_INTERRUPT_ENABLE_OFFSET		(0x00000004)
#define XLNX_CLOCK_TICK_GENERATOR_KERNEL_IP_INTERRUPT_ENABLE_OFFSET			(0x00000008)
#define XLNX_CLOCK_TICK_GENERATOR_KERNEL_IP_INTERRUPT_STATUS_OFFSET			(0x0000000C)




/* Now we have the registers that are custom to this block */
#define XLNX_CLOCK_TICK_GENERATOR_STREAM_ENABLE_OFFSET                      (0x00000010)

#define XLNX_CLOCK_TICK_GENERATOR_TICK_INTERVAL_START                       (0x00000014)
#define XLNX_CLOCK_TICK_GENERATOR_TICK_INTERVAL_MULTIPLIER                  (0x00000004)
#define XLNX_CLOCK_TICK_GENERATOR_TICK_INTERVAL_OFFSET(INDEX)               (XLNX_CLOCK_TICK_GENERATOR_TICK_INTERVAL_START + (XLNX_CLOCK_TICK_GENERATOR_TICK_INTERVAL_MULTIPLIER * (INDEX)))

#define XLNX_CLOCK_TICK_GENERATOR_STATUS_OFFSET                             (0x00000058)

#define XLNX_CLOCK_TICK_GENERATOR_STATS_START                               (0x0000005C)
#define XLNX_CLOCK_TICK_GENERATOR_NUM_STATS_REGISTERS                       (5) 




#endif //XLNX_CLOCK_TICK_GENERATOR_ADDRESS_MAP_H
