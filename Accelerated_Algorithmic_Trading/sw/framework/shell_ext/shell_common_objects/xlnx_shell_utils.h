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


#ifndef XLNX_SHELL_UTILS_H
#define XLNX_SHELL_UTILS_H



#include <cstdint>
#include <cstring>

#include "xlnx_shell.h"

namespace XLNX
{



#ifndef XLNX_UNUSED_ARG
#define XLNX_UNUSED_ARG(x)		(x = x)
#endif





//
// Utility Functions
//
bool ParseMACAddress(Shell* pShell, char* pToken, uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d, uint8_t* e, uint8_t* f);
bool ParseIPv4Address(Shell* pShell, char* pToken, uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d);
bool ParsePort(Shell* pShell, char* pToken, uint16_t* port);





} //namespace XLNX

#endif //XLNX_SHELL_UTILS_H




