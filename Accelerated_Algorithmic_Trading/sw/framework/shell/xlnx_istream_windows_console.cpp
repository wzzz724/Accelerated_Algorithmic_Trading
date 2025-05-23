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

#include <cstdint>
#include <iostream>

#ifdef _WIN32
#include <conio.h>
#include "xlnx_istream_windows_console.h"
using namespace XLNX;



InStreamWindowsConsole::InStreamWindowsConsole(OutStream* pOutputStream)
{

	m_pOutputStream = pOutputStream;
	m_bEchoState = true;

}





InStreamWindowsConsole::~InStreamWindowsConsole()
{

}




bool InStreamWindowsConsole::getChar(char* pChar)
{
	bool retval = true;

	*pChar = (char)_getch();

	return retval;
}






InStream::KeyPressEnum InStreamWindowsConsole::getKeyPress(char c)
{
	uint8_t charValue = c; //need to convert to a uint8 for switch comparison with hex constants to work correctly
	uint8_t nextCharValue;
	bool retval;
	InStream::KeyPressEnum keyPress = InStream::NORMAL;

	switch (charValue)
	{
		case '\b':
		case 0x7F: //DEL
		{
			keyPress = InStream::BACKSPACE;
			break;
		}


		case 0x1B:
		{
			keyPress = InStream::ESCAPE;
			break;
		}
	
		case 0xE0: //ESC
		{
			retval = getChar((char*)&nextCharValue);
			if (retval)
			{
				switch (nextCharValue)
				{
					case(0x48):
					{
						keyPress = InStream::UPARROW;
						break;
					}
	
					case(0x50):
					{
						keyPress = InStream::DOWNARROW;
						break;
					}
	
					case(0x4D):
					{
						keyPress = InStream::RIGHTARROW;
						break;
					}
	
					case(0x4B):
					{
						keyPress = InStream::LEFTARROW;
						break;
					}

					case(0x47):
					{
						keyPress = InStream::HOME;
						break;
					}

					case(0x4F):
					{
						keyPress = InStream::END;
						break;
					}

					case(0x53):
					{
						keyPress = InStream::DEL;
						break;
					}
	
					default:
					{
						keyPress = InStream::CONTROL;
						break;
					}
				}
			}
			else
			{
				keyPress = InStream::CONTROL;
			}
	
			break;
		}
	
		case '\t':
		{
			keyPress = InStream::TAB;
			break;
		}
	
	
		case '\r': //fall-through
		case '\n':
		{
			keyPress = InStream::EOL;
			break;
		}
	
	
		default:
		{
			if (c < ' ')
			{
				keyPress = InStream::CONTROL;
			}
			else
			{
				keyPress = InStream::NORMAL;
			}
			break;
		}
	}

	return keyPress;
}













#endif //_WIN32







