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

#ifndef XLNX_HISTORY_BUFFER_H
#define XLNX_HISTORY_BUFFER_H


#include <cstdint>

#include "xlnx_shell_config.h"



namespace XLNX
{


class HistoryBuffer
{

public:
	HistoryBuffer();
	virtual ~HistoryBuffer();

	void Reset(void);
	void AddCommand(char* pCommand);
	char* GetCommand(uint32_t commandIndex);
	uint32_t GetNumCommands(void);

	char* GetPreviousCommand(void);
	char* GetNextCommand(void);

private:

	char m_historyBuffer[XLNX_SHELL_CMD_HISTORY_SIZE][XLNX_SHELL_INPUT_BUFFER_SIZE + 1];

	uint32_t m_numCommandsInHistoryBuffer;
	uint32_t m_nextFreeBufferIndex;
	bool m_bBufferHasWrapped;

#define UNINITIALISED_INDEX	(-1)
	int32_t m_iteratorIndex;
	int32_t m_oldestCommandIndex;
	int32_t m_newestCommandIndex;



};





}






#endif


