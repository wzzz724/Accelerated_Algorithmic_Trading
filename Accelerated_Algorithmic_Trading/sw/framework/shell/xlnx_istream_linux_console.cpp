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


#ifdef __linux__

#include "xlnx_istream_linux_console.h"
using namespace XLNX;






InStreamLinuxConsole::InStreamLinuxConsole(OutStream* pOutputStream)
{
	struct termios newTermios;

	m_pOutputStream = pOutputStream;
	m_bEchoState = true;


	std::cin.unsetf(std::ios_base::basefield);



	tcgetattr(STDIN_FILENO, &m_originalTermios);

	newTermios = m_originalTermios;

	// ICANON normally takes care that one line at a time will be processed
	// that means it will return if it sees a "\n" or an EOF or an EOL
	newTermios.c_lflag &= ~(ICANON | ECHO);

	// Those new settings will be set to STDIN
	// TCSANOW tells tcsetattr to change attributes immediately. 
	tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);
}





InStreamLinuxConsole::~InStreamLinuxConsole()
{
	//revert back the stream to its original settings 
	tcsetattr(STDIN_FILENO, TCSANOW, &m_originalTermios);
}




bool InStreamLinuxConsole::getChar(char* pChar)
{
	bool retval = true;

	*pChar = (char) getchar();

	return retval;
}






InStream::KeyPressEnum InStreamLinuxConsole::getKeyPress(char c)
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

		case 0x1B: //ESC
		{
			retval = getChar((char*) &nextCharValue);
			
			if (retval)
			{
				if (nextCharValue == '[') 
				{
					retval = getChar((char*) &nextCharValue);

					if (retval)
					{
						switch (nextCharValue)
						{
							case('A'):
							{
								keyPress = InStream::UPARROW;
								break;
							}

							case('B'):
							{
								keyPress = InStream::DOWNARROW;
								break;
							}

							case('C'):
							{
								keyPress = InStream::RIGHTARROW;
								break;
							}

							case('D'):
							{
								keyPress = InStream::LEFTARROW;
								break;
							}

							//There are multiple different control codes for the HOME/END keys
							//depending on the terminal settings.
							//Here we handle a couple of different ones...

							case('F'):
							{
								keyPress = InStream::END;
								break;
							}

							case('H'):
							{
								keyPress = InStream::HOME;
								break;
							}


							case('1'):
							{
								retval = getChar((char*)&nextCharValue);

								if (retval)
								{
									if (nextCharValue == '~')
									{
										keyPress = InStream::HOME;
									}
								}
								break;
							}


							case('3'):
							{
								retval = getChar((char*)&nextCharValue);

								if (retval)
								{
									if (nextCharValue == '~')
									{
										keyPress = InStream::DEL;
									}
								}
								break;
							}

							case('4'):
							{
								retval = getChar((char*)&nextCharValue);

								if (retval)
								{
									if (nextCharValue == '~')
									{
										keyPress = InStream::END;
									}
								}
								
								break;
							}

							default:
							{
								keyPress = InStream::CONTROL;
								break;
							}
						}
					}
				}
				else if (nextCharValue == 0x1B)
				{
					keyPress = InStream::ESCAPE;
				}
				else
				{
					keyPress = InStream::CONTROL;
				}
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










#endif //_linux_




