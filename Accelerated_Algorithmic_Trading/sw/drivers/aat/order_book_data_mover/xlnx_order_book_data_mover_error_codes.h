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


#ifndef XLNX_ORDER_BOOK_DATA_MOVER_ERROR_CODES_H
#define XLNX_ORDER_BOOK_DATA_MOVER_ERROR_CODES_H


#include <cstdint>


#ifndef XLNX_OK
#define XLNX_OK														            (0x00000000)
#endif

#define XLNX_ORDER_BOOK_DATA_MOVER_ERROR_NOT_INITIALISED			            (0x00000001)
#define XLNX_ORDER_BOOK_DATA_MOVER_ERROR_IO_FAILED					            (0x00000002)
#define XLNX_ORDER_BOOK_DATA_MOVER_ERROR_INVALID_PARAMETER         	            (0x00000003)
#define XLNX_ORDER_BOOK_DATA_MOVER_ERROR_CU_NAME_NOT_FOUND			            (0x00000004)
#define XLNX_ORDER_BOOK_DATA_MOVER_ERROR_CU_INDEX_NOT_FOUND			            (0x00000005)
#define XLNX_ORDER_BOOK_DATA_MOVER_ERROR_FAILED_TO_ALLOCATE_BUFFER_OBJECT       (0x00000006)
#define XLNX_ORDER_BOOK_DATA_MOVER_ERROR_FAILED_TO_MAP_BUFFER_OBJECT            (0x00000007)
#define XLNX_ORDER_BOOK_DATA_MOVER_ERROR_FAILED_TO_SYNC_BUFFER_OBJECT           (0x00000008)
















#endif
