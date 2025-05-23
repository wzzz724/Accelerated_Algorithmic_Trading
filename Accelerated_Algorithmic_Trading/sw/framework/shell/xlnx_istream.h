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

#ifndef XLNX_ISTREAM_H
#define XLNX_ISTREAM_H


#include <iostream>

#include "xlnx_ostream.h"


namespace XLNX
{


class InStream
{
	
public:

	enum KeyPressEnum
	{
		NORMAL = 0,
		BACKSPACE,
		UPARROW,
		DOWNARROW,
		LEFTARROW,
		RIGHTARROW,
		HOME,
		END,
		DEL,
		ESCAPE,
		TAB,
		EOL,
		CONTROL
	};


	virtual ~InStream();


public:
	virtual bool getChar(char* pChar) = 0;
	virtual KeyPressEnum getKeyPress(char c) = 0;



public:
	void setEcho(bool bEnabled);
	bool getEcho(void);



protected:
	InStream(OutStream* pOstream = nullptr);




protected:
	bool m_bEchoState;
	OutStream* m_pOutputStream;
};









}


#endif

