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
#include <cstring>

#include "xlnx_history_buffer.h"



using namespace XLNX;

HistoryBuffer::HistoryBuffer()
{
	Reset();
}


HistoryBuffer::~HistoryBuffer()
{

}


void HistoryBuffer::Reset(void)
{
	memset(m_historyBuffer, 0, sizeof(m_historyBuffer));

	m_numCommandsInHistoryBuffer = 0;
	m_nextFreeBufferIndex = 0;
	m_bBufferHasWrapped = false;

	m_iteratorIndex = UNINITIALISED_INDEX;
	m_oldestCommandIndex = UNINITIALISED_INDEX;
	m_newestCommandIndex = UNINITIALISED_INDEX;
}


void HistoryBuffer::AddCommand(char* pCommand)
{
	if (strlen(pCommand) > 0)
	{
		strcpy(m_historyBuffer[m_nextFreeBufferIndex], pCommand);

		m_newestCommandIndex = m_nextFreeBufferIndex;
		m_iteratorIndex = UNINITIALISED_INDEX;


		m_nextFreeBufferIndex++;
		if (m_nextFreeBufferIndex >= XLNX_SHELL_CMD_HISTORY_SIZE)
		{
			m_nextFreeBufferIndex = 0;
			m_bBufferHasWrapped = true;
		}



		if (m_numCommandsInHistoryBuffer < XLNX_SHELL_CMD_HISTORY_SIZE)
		{
			m_numCommandsInHistoryBuffer++;
		}


		if (m_bBufferHasWrapped)
		{
			m_oldestCommandIndex = m_nextFreeBufferIndex;
		}
	}

}


char* HistoryBuffer::GetCommand(uint32_t commandIndex)
{
	char* pCommand = nullptr;
	uint32_t index;

	if (commandIndex < m_numCommandsInHistoryBuffer)
	{
		if (m_bBufferHasWrapped == false)
		{
			index = commandIndex;
		}
		else
		{
			index = m_nextFreeBufferIndex + commandIndex;
			if (index >= XLNX_SHELL_CMD_HISTORY_SIZE)
			{
				index -= XLNX_SHELL_CMD_HISTORY_SIZE;
			}
		}

		pCommand = m_historyBuffer[index];
	}

	return pCommand;
}




uint32_t HistoryBuffer::GetNumCommands(void)
{
	return m_numCommandsInHistoryBuffer;
}










char* HistoryBuffer::GetPreviousCommand(void)
{
	char* pCommand = nullptr;

	if (m_iteratorIndex == UNINITIALISED_INDEX)
	{
		m_iteratorIndex = m_newestCommandIndex;
	}
	else
	{
		if (m_iteratorIndex != m_oldestCommandIndex)
		{
			m_iteratorIndex--;
			if (m_iteratorIndex < 0)
			{
				if (m_bBufferHasWrapped)
				{
					m_iteratorIndex = XLNX_SHELL_CMD_HISTORY_SIZE - 1;
				}
				else
				{
					m_iteratorIndex = 0;
				}
			}
		}
	}

	if (m_iteratorIndex != UNINITIALISED_INDEX)
	{
		pCommand = m_historyBuffer[m_iteratorIndex];
	}

	return pCommand;
}



char* HistoryBuffer::GetNextCommand(void)
{
	char* pCommand = nullptr;

	if (m_iteratorIndex != UNINITIALISED_INDEX)
	{
		if (m_iteratorIndex != m_newestCommandIndex)
		{
			m_iteratorIndex++;
			if (m_iteratorIndex >= XLNX_SHELL_CMD_HISTORY_SIZE)
			{
				if (m_bBufferHasWrapped)
				{
					m_iteratorIndex = 0;
				}
				else
				{
					m_iteratorIndex = XLNX_SHELL_CMD_HISTORY_SIZE - 1;
				}
			}
		}
		else
		{
			m_iteratorIndex = UNINITIALISED_INDEX;
		}
	}

	if (m_iteratorIndex != UNINITIALISED_INDEX)
	{
		pCommand = m_historyBuffer[m_iteratorIndex];
	}


	return pCommand;
}



