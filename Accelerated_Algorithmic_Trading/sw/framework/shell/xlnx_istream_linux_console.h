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

#ifndef XLNX_ISTREAM_LINUX_CONSOLE_H
#define XLNX_ISTREAM_LINUX_CONSOLE_H

#ifdef __linux__

#include <iostream>

#include <termios.h>
#include <unistd.h>


#include "xlnx_istream.h"
#include "xlnx_ostream.h"

namespace XLNX
{ 


class InStreamLinuxConsole : public InStream
{


public:
	InStreamLinuxConsole(OutStream* pOutputStream = nullptr);
	virtual ~InStreamLinuxConsole();



public:
	virtual bool getChar(char* pChar);
	virtual KeyPressEnum getKeyPress(char c);


private:
	struct termios m_originalTermios;

}; //class InStreamLinuxConsole




} //namespace XLNX

#endif //__linux__

#endif //XLNX_ISTREAM_LINUX_CONSOLE_H

