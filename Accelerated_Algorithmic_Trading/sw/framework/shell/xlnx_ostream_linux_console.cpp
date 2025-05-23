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

#include <cstdio>
#include <cstring>


#ifdef __linux__

#include "xlnx_ostream_linux_console.h"
using namespace XLNX;


void OutStreamLinuxConsole::write(const char* s, uint32_t numChars)
{
	::fwrite(s, 1, numChars, stdout);
}






void OutStreamLinuxConsole::erase(uint32_t numChars)
{
	const char* eraseSequence = "\b \b";
	uint32_t sequenceLength = (uint32_t) strlen(eraseSequence);

	for (uint32_t i = 0; i < numChars; i++)
	{
		this->write(eraseSequence, sequenceLength);
	}

}



#endif //__linux__