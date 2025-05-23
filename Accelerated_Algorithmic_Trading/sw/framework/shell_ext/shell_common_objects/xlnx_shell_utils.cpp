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

#include <cinttypes>

#include "xlnx_shell_utils.h"


using namespace XLNX;


bool XLNX::ParseMACAddress(Shell* pShell, char* pToken, uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d, uint8_t* e, uint8_t* f)
{
	bool bOKToContinue = true;
	static const char* MACTemplate = "AA:BB:CC:DD:EE:FF";
	int numTokens = 0;


	if (strlen(pToken) != strlen(MACTemplate))
	{
		pShell->printf("[ERROR] MAC address format incorrect - should be in the form %s\n", MACTemplate);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		for (int i = 0; i < (int)strlen(pToken); i++)
		{
			if (MACTemplate[i] == ':')
			{
				if (pToken[i] != ':')
				{
					pShell->printf("[ERROR] MAC address format incorrect - should be in the form %s\n", MACTemplate);
					bOKToContinue = false;
					break; //out of loop
				}
			}
			else
			{
				if (isxdigit(pToken[i]) == 0)
				{
					pShell->printf("[ERROR] MAC address format incorrect - should be in the form %s\n", MACTemplate);
					bOKToContinue = false;
					break; //out of loop
				}
			}
		}
	}


	if (bOKToContinue)
	{
		//if we get to here, the supplied MAC address is in the correct format...we can safely parse it into binary form...

		//first convert to lower case... 
		for (int i = 0; i < (int)strlen(pToken); i++)
		{
			pToken[i] = tolower(pToken[i]);
		}

		numTokens = sscanf(pToken, "%02" SCNx8 ":%02" SCNx8 ":%02" SCNx8 ":%02" SCNx8 ":%02" SCNx8 ":%02" SCNx8, a, b, c, d, e, f);

		if (numTokens != 6)
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] Failed to parse MAC address\n");
		}

	}


	return bOKToContinue;
}







bool XLNX::ParseIPv4Address(Shell* pShell, char* pToken, uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d)
{
	bool bOKToContinue = true;
	static const int NUM_QUAD_VALUES = 4;
	uint32_t intQuadValues[NUM_QUAD_VALUES];
	uint32_t numTokens;


	for (int i = 0; i < (int)strlen(pToken); i++)
	{
		if ((isdigit(pToken[i]) == 0) && (pToken[i] != '.'))
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] Invalid IPv4 address format");
			break; //out of loop
		}
	}



	if (bOKToContinue)
	{
		numTokens = sscanf(pToken, "%u.%u.%u.%u", &intQuadValues[0],
												  &intQuadValues[1],
												  &intQuadValues[2],
												  &intQuadValues[3]);

		if (numTokens != NUM_QUAD_VALUES)
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] Failed to parse IP address\n");
		}
	}


	if (bOKToContinue)
	{
		for (int i = 0; i < NUM_QUAD_VALUES; i++)
		{
			if (intQuadValues[i] >= 256)
			{
				bOKToContinue = false;
				pShell->printf("[ERROR] A dotted-quad value is out of range\n");
				break; //out of loop
			}
		}
	}




	if (bOKToContinue)
	{
		*a = (uint8_t)(intQuadValues[0] & 0xFF);
		*b = (uint8_t)(intQuadValues[1] & 0xFF);
		*c = (uint8_t)(intQuadValues[2] & 0xFF);
		*d = (uint8_t)(intQuadValues[3] & 0xFF);
	}

	return bOKToContinue;
}







bool XLNX::ParsePort(Shell* pShell, char* pToken, uint16_t* port)
{
	bool bOKToContinue = true;
	uint32_t value;

	bOKToContinue = pShell->parseUInt32(pToken, &value);
	
	if (bOKToContinue)
	{
		if (value <= 0x0000FFFF)
		{
			*port = (uint16_t)value;
		}
		else
		{
			pShell->printf("[ERROR] Specified port number too large\n");
			bOKToContinue = false;
		}
	}

	return bOKToContinue;
}


