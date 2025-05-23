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

#include <cstdarg>
#include <cstring>
#include <cinttypes>


#include "xlnx_shell.h"
#include "xlnx_shell_cmds.h"




using namespace XLNX;




#ifdef _MSC_VER
#define strtok_r strtok_s
#endif


const char* Shell::PROMPT = ">> ";
const char* Shell::SCRIPT_PROMPT_PREFIX = "--";

static const char* TRUE_LOWER_STRING = "true";
static const char* TRUE_UPPER_STRING = "TRUE";

static const char* FALSE_LOWER_STRING = "false";
static const char* FALSE_UPPER_STRING = "FALSE";

static const char* OUTPUT_REDIRECT_OVERWRITE_STRING = ">";
static const char* OUTPUT_REDIRECT_APPEND_STRING = ">>";


Shell::Shell()
{
	m_pInputStream = nullptr;
	m_pOutputStream = nullptr;


	m_numCharsInInputBuffer = 0;
	m_cursorIndex = 0;

	m_commandManager.SetBuiltInCommandTable(XLNX_SHELL_COMMANDS_TABLE, NUM_SHELL_COMMANDS);

	m_preDownloadCallback = nullptr;
	m_postDownloadCallback = nullptr;
	m_downloadCallbackObject = nullptr;

	ResetCompletion();

	m_bOutputRedirectionEnabled = false;
	m_pOutputRedirectionFile = nullptr;
}





Shell::~Shell()
{

}




void Shell::Initialise(XLNX::InStream& inputStream, XLNX::OutStream& outputStream, DeviceInterface* pDeviceInterface)
{
	m_pInputStream = &inputStream;
	m_pOutputStream = &outputStream;
	m_pDeviceInterface = pDeviceInterface;

	m_bExitRequested = false;
}




void Shell::Run(XLNX::InStream& inputStream, XLNX::OutStream& outputStream, DeviceInterface* pDeviceInterface)
{
	bool bOKToContinue = true;


	this->Initialise(inputStream, outputStream, pDeviceInterface);
	


	//
	// main loop
	//
	while (!m_bExitRequested)
	{
		

		this->printf(PROMPT);


		//
		// In order to be able to process special chars (e.g. TAB) we have to read
		// each character ourselves and determine if we need to do any special processing
		// (e.g. auto-completion) as a result.
		// 
		// This means we have to basically operate in an unbuffered mode, where we receive each
		// keypress.  This also means we will receive keypresses for things like backspace, 
		// which we must also handle correctly...
		bOKToContinue = GetInputLine();


		if (bOKToContinue)
		{
			if (m_numCharsInInputBuffer > 0)
			{
				m_historyBuffer.AddCommand(m_inputBuffer);

				TokenizeLine(m_inputBuffer);

				HandleOutputRedirection();

				if (m_argc > 0)
				{
					m_commandManager.ExecuteCommand(this, m_argc, m_argv);
				}				


				DisableOutputRedirection();
			}
		}
		else
		{
			break; //out of loop
		}


	} //END while (!m_bExitRequested)



	CloseDevice();
}






void Shell::RunOneShot(XLNX::InStream& inputStream, XLNX::OutStream& outputStream, DeviceInterface* pDeviceInterface, int argc, char* argv[])
{
	this->Initialise(inputStream, outputStream, pDeviceInterface);

	m_commandManager.ExecuteCommand(this, argc, argv);
}






void Shell::Exit(void)
{
	m_bExitRequested = true;
}






void Shell::ProcessInputChar(char c)
{
	InStream::KeyPressEnum keyPressEnum;
	bool bResetCompletion;


	//we want to reset auto-completion state on any keypress EXCEPT tab
	//so we will default a flag to true here and only set it to false
	//if we are processing a TAB
	bResetCompletion = true;

	keyPressEnum = m_pInputStream->getKeyPress(c);

	switch (keyPressEnum)
	{
		case(InStream::KeyPressEnum::NORMAL):
		{
			ProcessNormalChar(c);
			break;
		}

		case(InStream::KeyPressEnum::TAB):
		{
			ProcessTab();
			bResetCompletion = false;
			break;
		}


		case(InStream::KeyPressEnum::BACKSPACE):
		{
			ProcessBackspace();
			break;
		}


		case(InStream::KeyPressEnum::EOL):
		{
			ProcessEOL();
			break;
		}

		case(InStream::KeyPressEnum::UPARROW):
		{
			ProcessUpArrow();
			break;
		}

		case(InStream::KeyPressEnum::DOWNARROW):
		{
			ProcessDownArrow();
			break;
		}

		case(InStream::KeyPressEnum::LEFTARROW):
		{
			ProcessLeftArrow();
			break;
		}

		case(InStream::KeyPressEnum::RIGHTARROW):
		{
			ProcessRightArrow();
			break;
		}


		case(InStream::KeyPressEnum::HOME):
		{
			ProcessHome();
			break;
		}

		case(InStream::KeyPressEnum::END):
		{
			ProcessEnd();
			break;
		}


		case(InStream::KeyPressEnum::DEL):
		{
			ProcessDel();
			break;
		}


		case(InStream::KeyPressEnum::ESCAPE):
		{
			ProcessEscape();
			break;
		}


		default:
		{
			//Do nothing...
			break;
		}
	}

	if (bResetCompletion)
	{
		ResetCompletion();
	}


}






void Shell::ProcessNormalChar(char c)
{
	InsertCharAtCursorIndex(c);
	
}






void Shell::ProcessBackspace(void)
{
	uint32_t numCharsToShunt;
	uint32_t shuntStartIndex;
	uint32_t shuntEndIndex;

	//Processing a backspace involves
	//(1) Deleting the character to the LEFT of the cursor location
	//(2) Shunting down the characters AT and to the RIGHT of the cursor location


	if (m_cursorIndex > 0)
	{
		// we have to cope with the situation where the cursor is not at the end of the line
		// (i.e. the user has moved it back)


		numCharsToShunt = m_numCharsInInputBuffer - m_cursorIndex;

		shuntStartIndex = m_cursorIndex;
		shuntEndIndex = shuntStartIndex + numCharsToShunt;


		for (uint32_t i = shuntStartIndex; i < shuntEndIndex; i++)
		{
			m_inputBuffer[i - 1] = m_inputBuffer[i];
		}


		m_cursorIndex--;

		m_numCharsInInputBuffer--;
		m_inputBuffer[m_numCharsInInputBuffer] = '\0';



		//now update what the user sees in their terminal...

		this->printf("\b");

		this->printf("%s", &m_inputBuffer[m_cursorIndex]);


		//overwrite the excess char at the end with a space
		this->printf(" ");


		//the act of outputting the new string to the terminal will move the cursor along...
		//we need to move it back to the original place...
		for (uint32_t i = 0; i < numCharsToShunt + 1; i++)
		{
			this->printf("\b");
		}
	}
}












void Shell::ProcessEOL(void)
{
	m_inputBuffer[m_numCharsInInputBuffer] = '\0';

	this->printf("\n\r");

	m_bGotEOL = true;

}









void Shell::ProcessTab(void)
{

	char* newInputLine = GetCompletionCandidate(m_inputBuffer); 
	
	if(newInputLine != nullptr)
	{
		EraseLine();
		
		strcpy(m_inputBuffer, newInputLine);
		m_numCharsInInputBuffer = (uint32_t) strlen(m_inputBuffer);
		m_cursorIndex = m_numCharsInInputBuffer;
	
		this->printf("%s", m_inputBuffer);
	}

}




void Shell::ProcessUpArrow(void)
{
	char* pPreviousCommand = nullptr;

	pPreviousCommand = m_historyBuffer.GetPreviousCommand();

	if (pPreviousCommand != nullptr)
	{
		EraseLine();

		strcpy(m_inputBuffer, pPreviousCommand);
		m_numCharsInInputBuffer = (uint32_t) strlen(m_inputBuffer);
		m_cursorIndex = m_numCharsInInputBuffer;

		this->printf("%s", m_inputBuffer);
	}
}




void Shell::ProcessDownArrow(void)
{
	char* pNextCommand = nullptr;

	pNextCommand = m_historyBuffer.GetNextCommand();

	if (pNextCommand != nullptr)
	{
		EraseLine();

		strcpy(m_inputBuffer, pNextCommand);
		m_numCharsInInputBuffer = (uint32_t) strlen(m_inputBuffer);
		m_cursorIndex = m_numCharsInInputBuffer;
		this->printf("%s", m_inputBuffer);
	}
	else
	{
		EraseLine();
	}
}







void Shell::ProcessLeftArrow(void)
{
	if (m_cursorIndex > 0)
	{
		this->printf("\b");
		m_cursorIndex--;
	}


}




void Shell::ProcessRightArrow(void)
{
	if (m_cursorIndex < m_numCharsInInputBuffer)
	{
		this->printf("%c", m_inputBuffer[m_cursorIndex]);
		m_cursorIndex++;
	}
}






void Shell::ProcessEscape(void)
{
	//for the escape key, we want erase any input that has been entered...
	EraseLine();
}



void Shell::ProcessHome(void)
{
	uint32_t numCharsToMove;

	numCharsToMove = m_cursorIndex;

	for (uint32_t i = 0; i < numCharsToMove; i++)
	{
		ProcessLeftArrow();
	}
}


void Shell::ProcessEnd(void)
{
	uint32_t numCharsToMove;

	numCharsToMove = m_numCharsInInputBuffer - m_cursorIndex;

	for (uint32_t i = 0; i < numCharsToMove; i++)
	{
		ProcessRightArrow();
	}
}



void Shell::ProcessDel(void)
{
	uint32_t numCharsToShunt;


	//Processing a DEL involves
	//(1) Deleting the character AT the cursor location
	//(2) Shunting down the characters to the RIGHT of the cursor location


	if (m_cursorIndex < m_numCharsInInputBuffer)
	{
		numCharsToShunt = m_numCharsInInputBuffer - m_cursorIndex - 1;

		for (uint32_t i = 0; i < numCharsToShunt; i++)
		{
			m_inputBuffer[m_cursorIndex + i] = m_inputBuffer[m_cursorIndex + i + 1];
		}

		m_inputBuffer[m_cursorIndex + numCharsToShunt] = '\0';
		m_numCharsInInputBuffer = m_numCharsInInputBuffer - 1;


		//now update what the user sees in their terminal...
		this->printf("%s", &m_inputBuffer[m_cursorIndex]);


		//overwrite the excess char at the end with a space
		this->printf(" ");


		//the act of outputting the new string to the terminal will move the cursor along...
		//we need to move it back to the original place...
		for (uint32_t i = 0; i < numCharsToShunt+1; i++)
		{
			this->printf("\b");
		}
	}
}



void Shell::ResetForNewCommand(void)
{
	m_inputBuffer[0] = '\0';
	m_numCharsInInputBuffer = 0;
	m_cursorIndex = 0;
}





bool Shell::GetInputLine(void)
{
	bool bOKToContinue = true;
	char c;

	ResetForNewCommand();

	m_bGotEOL = false;
	while (!m_bGotEOL)
	{
		bOKToContinue = m_pInputStream->getChar(&c);

		if (bOKToContinue)
		{
			ProcessInputChar(c);
		}
		else
		{
			break; //out of loop 
		}
	}

	return bOKToContinue;
}




void Shell::TokenizeLine(char* line)
{
	char* pToken;
	char* position;
	const char* delimiters = " \t\r\n";
	uint32_t numTokens = 0;

	position = line;

	pToken = strtok_r(position, delimiters, &position);
	while (pToken != nullptr)
	{

		if (pToken[0] == '#') //# represents a comment...
		{
			break; //out of loop 
		}

		if (numTokens < MAX_ARGS)
		{
			m_argv[numTokens] = pToken;
			numTokens++;
		}

		pToken = strtok_r(position, delimiters, &position);
	}

	m_argc = numTokens;

}







void Shell::InsertCharAtCursorIndex(char c)
{
	uint32_t numCharsToShunt;
	uint32_t shuntStartIndex;
	uint32_t shuntEndIndex;

	char shuntChar1;
	char shuntChar2;


	if (m_numCharsInInputBuffer < XLNX_SHELL_INPUT_BUFFER_SIZE)
	{	
		// we have to cope with the situation where the cursor is not at the end of the line
		// (i.e. the user has moved it back)

		numCharsToShunt = m_numCharsInInputBuffer - m_cursorIndex;

		shuntStartIndex = m_cursorIndex;
		shuntEndIndex = shuntStartIndex + numCharsToShunt;


		//we start by shunting the chars to the right of the cursor index
		//to the right by one position.  This makes a space in the buffer
		//for our new character...
		shuntChar1 = m_inputBuffer[shuntStartIndex];
		for (uint32_t i = shuntStartIndex; i < shuntEndIndex; i++)
		{
			shuntChar2 = m_inputBuffer[i + 1];
			m_inputBuffer[i + 1] = shuntChar1;

			shuntChar1 = shuntChar2;
		}
		

		//now insert our new character into the buffer at the correct position..
		m_inputBuffer[m_cursorIndex] = c;
		m_cursorIndex++;

		m_numCharsInInputBuffer++;
		m_inputBuffer[m_numCharsInInputBuffer] = '\0';



		//now we need to update what the user sees in their terminal...	

		
		//output the updated string...
		if (m_pInputStream->getEcho())
		{
			this->printf("%s", &m_inputBuffer[shuntStartIndex]);
		}

		
		//the act of outputting the string will move the cursor along
		//we need to return the cursor to its original location
		//(if we were inserting in the middle of the line)
		for (uint32_t i = 0; i < numCharsToShunt; i++)
		{
			if (m_pInputStream->getEcho())
			{
				this->printf("\b");
			}
		}
		
	}
}













void Shell::EraseLine(void)
{
	// we have to cope with the situation where the cursor is not at the end of the line
	// (i.e. the user has moved it back)

	for (uint32_t i = 0; i < m_cursorIndex; i++)
	{
		this->printf("\b");
	}

	for (uint32_t i = 0; i < m_numCharsInInputBuffer; i++)
	{
		this->printf(" ");
	}

	for (uint32_t i = 0; i < m_numCharsInInputBuffer; i++)
	{
		this->printf("\b");
	}

	m_cursorIndex = 0;
	m_numCharsInInputBuffer = 0;
	m_inputBuffer[m_numCharsInInputBuffer] = '\0';
}



bool Shell::parseUInt16(char* pToken, uint16_t* pValue)
{
	bool bOKToContinue = true;
	int numTokens;
	size_t tokenLength = 0;

	tokenLength = strlen(pToken);


	for (uint32_t i = 0; i < tokenLength; i++)
	{
		if (isdigit(pToken[i]) == 0)
		{
			bOKToContinue = false;
			break;
		}
	}



	if (bOKToContinue)
	{
		numTokens = sscanf(pToken, "%" SCNu16, pValue);

		if (numTokens != 1)
		{
			bOKToContinue = false;
		}
	}


	return bOKToContinue;
}







bool Shell::parseHex16(char* pToken, uint16_t* pValue)
{
	bool bOKToContinue = true;
	int numTokens;
	size_t tokenLength = 0;

	tokenLength = strlen(pToken);

	if (tokenLength < 3) // minimum token length is '0x0'
	{
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		if (tokenLength > 6)	//max token length is '0x0000' (for a 16-bit value)
		{
			bOKToContinue = false;
		}
	}



	if (bOKToContinue)
	{
		//convert chars to lower case...eliminates need to check for both upper and lower case in a moment...
		for (uint32_t i = 0; i < tokenLength; i++)
		{
			pToken[i] = (char)tolower(pToken[i]);
		}
	}


	if (bOKToContinue)
	{
		//make sure first two chars are '0x' 
		if ((pToken[0] != '0') || (pToken[1] != 'x'))
		{
			bOKToContinue = false;
		}
	}


	if (bOKToContinue)
	{
		for (uint32_t i = 2; i < tokenLength; i++)
		{
			if (isxdigit(pToken[i]) == 0)
			{
				bOKToContinue = false;
				break;
			}
		}
	}



	if (bOKToContinue)
	{
		//if we get to here OK, we know token starts with 0x...so we can skip past it to get the data chars...	

		numTokens = sscanf(&(pToken[2]), "%" SCNx16, pValue);

		if (numTokens != 1)
		{
			bOKToContinue = false;
		}
	}

	return bOKToContinue;
}







bool Shell::parseUInt32(char* pToken, uint32_t* pValue)
{
	bool bOKToContinue = true;
	int numTokens;
	size_t tokenLength = 0;

	tokenLength = strlen(pToken);


	for (uint32_t i = 0; i < tokenLength; i++)
	{
		if (isdigit(pToken[i]) == 0)
		{
			bOKToContinue = false;
			break;
		}
	}



	if (bOKToContinue)
	{
		numTokens = sscanf(pToken, "%" SCNu32, pValue);

		if (numTokens != 1)
		{
			bOKToContinue = false;
		}
	}
	

	return bOKToContinue;
}







bool Shell::parseHex32(char* pToken, uint32_t* pValue)
{
	bool bOKToContinue = true;
	int numTokens;
	size_t tokenLength = 0;

	tokenLength = strlen(pToken);

	if (tokenLength < 3) // minimum token length is '0x0'
	{
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		if (tokenLength > 10)	//max token length is '0x00000000' (for a 32-bit value)
		{
			bOKToContinue = false;
		}
	}



	if (bOKToContinue)
	{
		//convert chars to lower case...eliminates need to check for both upper and lower case in a moment...
		for (uint32_t i = 0; i < tokenLength; i++)
		{
			pToken[i] = (char)tolower(pToken[i]);
		}
	}


	if (bOKToContinue)
	{
		//make sure first two chars are '0x' 
		if ((pToken[0] != '0') || (pToken[1] != 'x'))
		{
			bOKToContinue = false;
		}
	}


	if (bOKToContinue)
	{
		for (uint32_t i = 2; i < tokenLength; i++)
		{
			if (isxdigit(pToken[i]) == 0)
			{
				bOKToContinue = false;
				break;
			}
		}
	}



	if (bOKToContinue)
	{
		//if we get to here OK, we know token starts with 0x...so we can skip past it to get the data chars...	

		numTokens = sscanf(&(pToken[2]), "%" SCNx32, pValue);

		if (numTokens != 1)
		{
			bOKToContinue = false;
		}
	}

	return bOKToContinue;
}




bool Shell::parseUInt64(char* pToken, uint64_t* pValue)
{
	bool bOKToContinue = true;
	int numTokens;
	size_t tokenLength = 0;

	tokenLength = strlen(pToken);


	for (uint32_t i = 0; i < tokenLength; i++)
	{
		if (isdigit(pToken[i]) == 0)
		{
			bOKToContinue = false;
			break;
		}
	}



	if (bOKToContinue)
	{
		numTokens = sscanf(pToken, "%" SCNu64, pValue);

		if (numTokens != 1)
		{
			bOKToContinue = false;
		}
	}


	return bOKToContinue;

}






bool Shell::parseHex64(char* pToken, uint64_t* pValue)
{
	bool bOKToContinue = true;
	int numTokens;
	size_t tokenLength = 0;

	tokenLength = strlen(pToken);

	if (tokenLength < 3) // minimum token length is '0x0'
	{
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		if (tokenLength > 18)	//max token length is '0x0000000000000000' (for a 64-bit value)
		{
			bOKToContinue = false;
		}
	}



	if (bOKToContinue)
	{
		//convert chars to lower case...eliminates need to check for both upper and lower case in a moment...
		for (uint32_t i = 0; i < tokenLength; i++)
		{
			pToken[i] = (char)tolower(pToken[i]);
		}
	}


	if (bOKToContinue)
	{
		//make sure first two chars are '0x' 
		if ((pToken[0] != '0') || (pToken[1] != 'x'))
		{
			bOKToContinue = false;
		}
	}


	if (bOKToContinue)
	{
		for (uint32_t i = 2; i < tokenLength; i++)
		{
			if (isxdigit(pToken[i]) == 0)
			{
				bOKToContinue = false;
				break;
			}
		}
	}



	if (bOKToContinue)
	{
		//if we get to here OK, we know token starts with 0x...so we can skip past it to get the data chars...	

		numTokens = sscanf(&(pToken[2]), "%" SCNx64, pValue);

		if (numTokens != 1)
		{
			bOKToContinue = false;
		}
	}

	return bOKToContinue;
}









bool Shell::parseBool(char* pToken, bool* pValue)
{
	bool bOKToContinue = true;

	if ((strcmp(pToken, TRUE_LOWER_STRING) == 0) || (strcmp(pToken, TRUE_UPPER_STRING) == 0))
	{
		*pValue = true;
	}
	else if ((strcmp(pToken, FALSE_LOWER_STRING) == 0) || (strcmp(pToken, FALSE_UPPER_STRING) == 0))
	{
		*pValue = false;
	}
	else
	{
		bOKToContinue = false;
	}

	return bOKToContinue;
}










int Shell::printf(const char* fmt, ...)
{
	int length;
	va_list args;

	va_start(args, fmt);

	length = vsnprintf(m_outputBuffer, XLNX_SHELL_OUTPUT_BUFFER_SIZE, fmt, args);

	va_end(args);


	m_pOutputStream->write(m_outputBuffer, length);



	//If output redirection is also enabled, send the buffer to the output file...
	if (m_bOutputRedirectionEnabled)
	{
		if (m_pOutputRedirectionFile != nullptr)
		{
			fprintf(m_pOutputRedirectionFile, m_outputBuffer);
		}
	}

	

	return length;
}



const char* Shell::boolToString(bool bValue)
{
	const char* s;

	if (bValue)
	{
		s = TRUE_LOWER_STRING;
	}
	else
	{
		s = FALSE_LOWER_STRING;
	}

	return s;
}







void Shell::CloseDevice(void)
{
	m_pDeviceInterface->Close();
}




DeviceInterface* Shell::GetDeviceInterface(void)
{
	return m_pDeviceInterface;
}



HistoryBuffer* Shell::GetHistoryBuffer(void)
{
	return &m_historyBuffer;
}



CommandManager* Shell::GetCommandManager(void)
{
	return &m_commandManager;
}






bool Shell::AddObjectCommandTable(const char* pObjectName, void* pObjectData, CommandTableElement* pCommandTable, uint32_t commandTableLength)
{
	bool bOKToContinue = true;

	bOKToContinue = m_commandManager.AddObjectCommandTable(pObjectName, pObjectData, pCommandTable, commandTableLength);

	return bOKToContinue;
}




bool Shell::PlayScript(char* filePath)
{
	bool bOKToContinue = true;
	FILE* pFile = nullptr;
	static const uint32_t BUFFER_SIZE = 256;
	char lineBuffer[BUFFER_SIZE + 1];
	char* pLine = nullptr;
	uint32_t numErrors = 0;
	
	if (bOKToContinue)
	{
		pFile = fopen(filePath, "rb");
		if (pFile == nullptr)
		{
			this->printf("[ERROR] Failed to open script file (%s)\n", filePath);
			bOKToContinue = false;
		}
	}

	if (bOKToContinue)
	{
		this->printf("Executing script %s...\n", filePath);
		this->printf("\n");

		//read the first line from the script file...
		pLine = fgets(lineBuffer, BUFFER_SIZE, pFile);

		while (pLine != nullptr)
		{
			this->printf("%s%s%s", SCRIPT_PROMPT_PREFIX, PROMPT, pLine);

			if ((strrchr(pLine, '\n') == nullptr) && (strrchr(pLine, '\r') == nullptr))
			{
				this->printf("\n");
			}

			TokenizeLine(pLine);

			if (m_argc > 0)
			{
				bOKToContinue = m_commandManager.ExecuteCommand(this, m_argc, m_argv);

				if (bOKToContinue == false)
				{
					numErrors++;
				}
			}

			//...and continue with the next line until we reach the end of the file...
			pLine = fgets(lineBuffer, BUFFER_SIZE, pFile);
		}


		if (bOKToContinue)
		{
			this->printf("\n");
			this->printf("End of script %s\n", filePath);
			
			if (numErrors > 0)
			{
				this->printf("\n");
				this->printf("COMMAND ERRORS FOUND: %u\n", numErrors);
				this->printf("Please review output for details\n");
			}

			this->printf("\n");
		}
		
		//close the file
		fclose(pFile);
		pFile = nullptr;

	}



	return bOKToContinue;

}




void Shell::SetDownloadCallbacks(DownloadCallbackType preDownloadCallback, DownloadCallbackType postDownloadCallback, void* dataObject)
{
	m_preDownloadCallback = preDownloadCallback;
	m_postDownloadCallback = postDownloadCallback;
	m_downloadCallbackObject = dataObject;
}




void Shell::InvokePreDownloadCallback(void)
{
	if (m_preDownloadCallback != nullptr)
	{
		this->printf("Invoking USER-DEFINED pre-download callback...\n");
		(*m_preDownloadCallback)(m_downloadCallbackObject, GetDeviceInterface());
	}
}


void Shell::InvokePostDownloadCallback(void)
{
	if (m_postDownloadCallback != nullptr)
	{
		this->printf("Invoking USER-DEFINED post-download callback...\n");
		(*m_postDownloadCallback)(m_downloadCallbackObject, GetDeviceInterface());
	}
}




char* Shell::GetCompletionCandidate(char* line)
{
	char* candidate = nullptr;
	bool bHasTrailingSpace = false;
	uint32_t length;
	int argToComplete;
	uint32_t insertPosition;
	bool bCandidateInserted;


	if (m_bCompletionInProgress == false)
	{
		strncpy(m_completionInitialInputBuffer, line, XLNX_SHELL_INPUT_BUFFER_SIZE);
		m_bCompletionInProgress = true;
	}



	
	
	//In a moment we will be tokenizing the line to split it into discrete args.
	//However the tokenizer discards whitespace.
	//In order to indentify which argument we are attempting to auto-complete, we need
	//to know if there is any whitespace at the end of the line.
	//So we need to look for whitespace BEFORE we tokenize the line.

	length = strlen(m_completionInitialInputBuffer);

	if (length > 0)
	{
		if (isspace(m_completionInitialInputBuffer[length - 1]))
		{
			bHasTrailingSpace = true;
		}
	}
	else
	{
		bHasTrailingSpace = true;
	}


	//The act of tokenization inserts a '\0' character at the end of each word to form independent strings
	//But we need to maintain the original input unchanged.  
	//So copy to a different buffer before tokenization...

	strncpy(m_completionTokenBuffer, m_completionInitialInputBuffer, XLNX_SHELL_INPUT_BUFFER_SIZE);
	TokenizeLine(m_completionTokenBuffer);

	
	if (bHasTrailingSpace)
	{
		argToComplete = m_argc;
	}
	else
	{
		argToComplete = m_argc - 1;
	}
	

	
	candidate = m_commandManager.GetNextCompletionCandidate(m_argc, m_argv, argToComplete);


	if (candidate != nullptr)
	{
		//we have found a completion candidate...

		insertPosition = 0;
		bCandidateInserted = false;
		
		for (int i = 0; i < m_argc; i++)
		{
			if (i > 0)
			{
				insertPosition += sprintf(&m_completionOutputBuffer[insertPosition], " ");
			}

			if (i == argToComplete)
			{
				insertPosition += sprintf(&m_completionOutputBuffer[insertPosition], "%s", candidate);
				bCandidateInserted = true;
			}
			else
			{
				insertPosition += sprintf(&m_completionOutputBuffer[insertPosition], "%s", m_argv[i]);
			}			
		}


		if (bCandidateInserted == false)
		{
			if (argToComplete > 0)
			{
				insertPosition += sprintf(&m_completionOutputBuffer[insertPosition], " ");
			}

			insertPosition += sprintf(&m_completionOutputBuffer[insertPosition], "%s", candidate);
			bCandidateInserted = true;
		}
		
	}
	else
	{
		//No completion candidate found (or we have cycled through all available completion candidates)
		//just give back the original input line...
		strncpy(m_completionOutputBuffer, m_completionInitialInputBuffer, XLNX_SHELL_INPUT_BUFFER_SIZE);
	}


	return m_completionOutputBuffer;
}



void Shell::ResetCompletion(void)
{
	if (m_bCompletionInProgress == true)
	{
		m_bCompletionInProgress = false;
		memset(m_completionInitialInputBuffer, 0, sizeof(m_completionInitialInputBuffer));
		memset(m_completionTokenBuffer, 0, sizeof(m_completionTokenBuffer));
		memset(m_completionOutputBuffer, 0, sizeof(m_completionOutputBuffer));
	}

	m_commandManager.ResetCompletion();
}




bool Shell::HandleOutputRedirection(void)
{
	bool bOKToContinue = true;
	char* filePath;
	bool bRedirect = false;
	bool bOverwrite = false;

	if (m_argc >= 3)
	{
		if (strcmp(m_argv[m_argc - 2], OUTPUT_REDIRECT_OVERWRITE_STRING) == 0)
		{
			bRedirect = true;
			bOverwrite = true;
		}

		if (strcmp(m_argv[m_argc - 2], OUTPUT_REDIRECT_APPEND_STRING) == 0)
		{
			bRedirect = true;
			bOverwrite = false;
		}


		if(bRedirect)
		{
			filePath = m_argv[m_argc - 1];

			if (bOverwrite)
			{
				m_pOutputRedirectionFile = fopen(filePath, "wb");
			}
			else
			{
				m_pOutputRedirectionFile = fopen(filePath, "wb+");
			}



			if (m_pOutputRedirectionFile != nullptr)
			{
				//successfully opened a file for output redirection.
				m_bOutputRedirectionEnabled = true;

				//need to discard the "pipe" parameters 
				m_argv[m_argc - 2] = nullptr;
				m_argv[m_argc - 1] = nullptr;
				m_argc = m_argc - 2;
			}
			else
			{
				this->printf("[ERROR] Cannot open file for output redirection (%s)\n", filePath);
				bOKToContinue = false;
			}
		}
	}

	return bOKToContinue;
}




void Shell::DisableOutputRedirection(void)
{
	m_bOutputRedirectionEnabled = false;

	if (m_pOutputRedirectionFile != nullptr)
	{
		fflush(m_pOutputRedirectionFile);
		fclose(m_pOutputRedirectionFile);

		m_pOutputRedirectionFile = nullptr;
	}
}


