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

#ifndef XLNX_ISTREAM_WINDOWS_CONSOLE_H
#define XLNX_ISTREAM_WINDOWS_CONSOLE_H

#ifdef _WIN32

#include <iostream>

#include "xlnx_istream.h"
#include "xlnx_ostream.h"

namespace XLNX
{


class InStreamWindowsConsole : public InStream
{


public:
	InStreamWindowsConsole(OutStream* pOutputStream = nullptr);
	virtual ~InStreamWindowsConsole();



public:
	virtual bool getChar(char* pChar);
	virtual KeyPressEnum getKeyPress(char c);



}; //class InStreamWindowsConsole






} //namespace XLNX



#endif //_WIN32

#endif //XLNX_ISTREAM_WINDOWS_CONSOLE_H

