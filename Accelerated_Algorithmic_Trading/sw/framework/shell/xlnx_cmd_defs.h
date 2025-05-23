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

#ifndef XLNX_CMD_DEFS_H
#define XLNX_CMD_DEFS_H

#include <cstdint>


namespace XLNX
{

class Shell;


typedef int (*CommandFunctionType)(Shell* pShell, int argc, char* argv[], void* pObjectData);





class CommandTableElement
{
public:
	const char* commandName;
	CommandFunctionType handler;
	const char* argsListString;
	const char* descriptionString;

public:
	CommandTableElement(const char* commandName,
						CommandFunctionType handlerFunction,
						const char* argsListString,
						const char* descriptionString) : commandName(commandName),
														 handler(handlerFunction),
														 argsListString(argsListString),
														 descriptionString(descriptionString) {}

	CommandTableElement() : commandName(""), 
							handler(nullptr), 
							argsListString(""), 
							descriptionString("") {}

};



} //end namespace XLNX















#endif

