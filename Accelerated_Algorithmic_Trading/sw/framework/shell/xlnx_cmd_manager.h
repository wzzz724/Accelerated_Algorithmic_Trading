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

#ifndef XLNX_CMD_MANAGER_H
#define XLNX_CMD_MANAGER_H

#include <cstdint>

#include "xlnx_cmd_defs.h"



namespace XLNX
{



class Shell;


class CommandManager
{

public:
	CommandManager();
	virtual ~CommandManager();




public:

	//The built-in "headless" command table
	void SetBuiltInCommandTable(CommandTableElement* pTable, uint32_t tableLength);
	
	// Dynamically bindable object command tables
	bool AddObjectCommandTable(const char* objectName, void* pObjectData, CommandTableElement* pTable, uint32_t tableLength);
	


public:
	bool ExecuteCommand(Shell* pShell, int argc, char* argv[]);





public:
	typedef struct _ObjectCommandTableDescriptor
	{
		const char* pObjectName;
		void* pObjectData;
		CommandTableElement* pTable;
		uint32_t tableLength;

	}ObjectCommandTableDescriptor;


	void GetNumObjects(uint32_t* pNumObjects);

	bool GetObjectDescriptor(uint32_t objectIndex, ObjectCommandTableDescriptor** ppDescriptor);
	bool GetObjectDescriptor(char* objectName, ObjectCommandTableDescriptor** ppDescriptor);



public:
	char* GetNextCompletionCandidate(int argc, char* argv[], int argToComplete);
	void ResetCompletion(void);


private:
    void PrintBuiltInCommandHelp(Shell* pShell);
    void PrintSingleExternalObjectHelp(Shell* pShell, ObjectCommandTableDescriptor* pDescriptor);
    void PrintAllExternalObjectsHelp(Shell* pShell);
    void PrintExternalObjectHelpTableHeader(Shell* pShell);
    void PrintExternalObjectHelpTableDataRows(Shell* pShell, ObjectCommandTableDescriptor* pDescriptor);
    void PrintExternalObjectHelpTableFooter(Shell* pShell);

	void PrintExternalObjectNamesOnly(Shell* pShell);
	void PrintExternalObjectNamesOnlyHeader(Shell* pShell);
	void PrintExternalObjectNamesOnlyFooter(Shell* pShell);
	void PrintExternalObjectNamesOnlyTableDataRow(Shell* pShell, char* pObjectName, uint32_t rowIndex);



private:
	char* GetNextCompletionCandidateLayer1(char* token);
	char* GetNextCompletionCandidateLayer2(char* objectName, char* token);
	char* SearchForPartialObjectName(uint32_t startIndex, char* token);
	char* SearchForPartialObjectCommand(char* objectName, uint32_t startIndex, char* token);
	char* SearchForPartialBuiltInCommand(uint32_t startIndex, char* token);


private:
	ObjectCommandTableDescriptor m_builtInCommandTableDescriptor;



private:
	static const int MAX_OBJECT_COMMAND_TABLES = 20;
	ObjectCommandTableDescriptor m_objectCommandTableDescriptors[MAX_OBJECT_COMMAND_TABLES];
	uint32_t m_numObjectCommandTables;


private:
	uint32_t m_nextCompletionStartIndex;
	bool m_bLayer1CompletionDoObjectLookup;

	
};
	






}





















#endif
