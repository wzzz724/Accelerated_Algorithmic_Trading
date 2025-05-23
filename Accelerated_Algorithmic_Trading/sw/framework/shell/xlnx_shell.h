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

#ifndef XLNX_SHELL_H
#define XLNX_SHELL_H

#include <cstdint>
#include <iostream>


#include "xlnx_cmd_defs.h"
#include "xlnx_cmd_manager.h"

#include "xlnx_device_interface.h"
#include "xlnx_history_buffer.h"
#include "xlnx_istream.h"
#include "xlnx_ostream.h"
#include "xlnx_shell_config.h"






using namespace std;




namespace XLNX
{

class Shell
{
	
public:
	Shell();
	virtual ~Shell();
		
public:
	void Run(XLNX::InStream& inputStream, XLNX::OutStream& outputStream, DeviceInterface* pDeviceInterface);
	void RunOneShot(XLNX::InStream& inputStream, XLNX::OutStream& outputStream, DeviceInterface* pDeviceInterface, int argc, char* argv[]);

	void Exit(void);


public:
	bool parseUInt16(char* pToken, uint16_t* pValue);
	bool parseHex16(char* pToken, uint16_t* pValue);

	bool parseUInt32(char* pToken, uint32_t* pValue);
	bool parseHex32(char* pToken, uint32_t* pValue);

	bool parseUInt64(char* pToken, uint64_t* pValue);
	bool parseHex64(char* pToken, uint64_t* pValue);

	bool parseBool(char* pToken, bool* pValue);

public:
	int printf(const char* fmt, ...);

	const char* boolToString(bool bValue);


public:
	DeviceInterface* GetDeviceInterface(void);
	HistoryBuffer* GetHistoryBuffer(void);
	CommandManager* GetCommandManager(void);

public:
	bool PlayScript(char* filePath);


public:
	static const uint32_t COMMAND_PARSING_ERROR = -1;


public:
	bool AddObjectCommandTable(const char* objectName, void* objectData, CommandTableElement* pCommandTable, uint32_t commandTableLength);


public: //Download Callbacks - allows user to bind in a pre and post download callback that will be called before/after a bitstream has been downloaded 
		//This is intended to allow cleanup and reinitialisation of design-specific software objects that interact with HW
	typedef void (*DownloadCallbackType)(void* pDataObject, DeviceInterface* pDeviceInterface);
	void SetDownloadCallbacks(DownloadCallbackType preDownloadCallback, DownloadCallbackType postDownloadCallback, void* pDataObject);

public:
	void InvokePreDownloadCallback(void);
	void InvokePostDownloadCallback(void);

private:
	void Initialise(XLNX::InStream& inputStream, XLNX::OutStream& outputStream, DeviceInterface* pDeviceInterface);

private:
	void TokenizeLine(char* line);


private:
	void CloseDevice(void);


private:
	void ProcessInputChar(char c);
	void ProcessBackspace(void);
	void ProcessEOL(void);
	void ProcessNormalChar(char c);
	void ProcessTab(void);
	void ProcessUpArrow(void);
	void ProcessDownArrow(void);
	void ProcessLeftArrow(void);
	void ProcessRightArrow(void);
	void ProcessEscape(void);
	void ProcessHome(void);
	void ProcessEnd(void);
	void ProcessDel(void);

private:
	bool GetInputLine(void);
	void ResetForNewCommand(void);


private:
	void InsertCharAtCursorIndex(char c);
	void EraseLine(void);


private:
	char* GetCompletionCandidate(char* line);
	void ResetCompletion(void);


private:
	bool HandleOutputRedirection(void);
	void DisableOutputRedirection(void);

private:
	XLNX::InStream* m_pInputStream;
	XLNX::OutStream* m_pOutputStream;

	static const char* PROMPT;
	bool m_bExitRequested;



private:
	static const char* SCRIPT_PROMPT_PREFIX;



private:
	DeviceInterface* m_pDeviceInterface;


private:

	char m_inputBuffer[XLNX_SHELL_INPUT_BUFFER_SIZE + 1];
	uint32_t m_numCharsInInputBuffer;
	uint32_t m_cursorIndex;



private:
	bool m_bGotEOL;



private:
	static const int MAX_ARGS = 32;
	int m_argc;
	char* m_argv[MAX_ARGS];
	
private:
	char m_outputBuffer[XLNX_SHELL_OUTPUT_BUFFER_SIZE + 1];


private:
	HistoryBuffer m_historyBuffer;


private:
	CommandManager m_commandManager;


private:
	DownloadCallbackType m_preDownloadCallback;
	DownloadCallbackType m_postDownloadCallback;
	void* m_downloadCallbackObject;


private: //Command Auto-completion
	bool m_bCompletionInProgress;
	char m_completionInitialInputBuffer[XLNX_SHELL_INPUT_BUFFER_SIZE + 1];
	char m_completionTokenBuffer[XLNX_SHELL_INPUT_BUFFER_SIZE + 1];
	char m_completionOutputBuffer[XLNX_SHELL_INPUT_BUFFER_SIZE + 1];


private:
	bool m_bOutputRedirectionEnabled;
	FILE* m_pOutputRedirectionFile;

};




}




#endif
