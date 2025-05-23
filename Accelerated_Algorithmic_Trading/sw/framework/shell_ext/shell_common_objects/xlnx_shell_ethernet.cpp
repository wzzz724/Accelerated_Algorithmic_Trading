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





#include "xlnx_ethernet.h"
#include "xlnx_ethernet_error_codes.h"
#include "xlnx_ethernet_types.h"

#include "xlnx_shell_ethernet.h"
#include "xlnx_shell_utils.h"
using namespace XLNX;


static const char* NoneStr			= "none";

//Loopback Strings
static const char* PCSNEARStr		= "pcsnear";
static const char* PCSFARStr		= "pcsfar";
static const char* PMANEARStr		= "pmanear";
static const char* PMAFARStr		= "pmafar";


//Clock Select Strings
static const char* Static1Str		= "static1";
static const char* OutClkPCSStr		= "outclkpcs";
static const char* OutClkPMAStr		= "outclkpma";
static const char* PLLRefClkDiv1Str = "pllrefclkdiv1";
static const char* PLLRefClkDiv2Str = "pllrefclkdiv2";
static const char* ProgDivClkStr	= "progdivclk";


//Polarity Strings
static const char* NormalStr		= "normal";
static const char* InvertedStr		= "inverted";


//Equalization Strings
static const char* DFEStr			= "dfe";
static const char* LPMStr			= "lpm";


//Test Pattern Strings
static const char* PRBS7Str			= "prbs7";
static const char* PRBS9Str			= "prbs9";
static const char* PRBS15Str		= "prbs15";
static const char* PRBS23Str		= "prbs23";
static const char* PRBS31Str		= "prbs31";



//RxBufStatus Strings
static const char* NominalStr					= "nominal";
static const char* FillUnderMinLatencyStr		= "fill under min lat.";
static const char* FillOverMaxLatencyStr		= "fill over max lat.";
static const char* ElasticBufferUnderflowStr	= "buffer underflow";
static const char* ElasticBufferOverflowStr		= "buffer overflow";

static const char* InResetStr					= "IN RESET";
static const char* OutOfResetStr				= "out of reset";



static const char* LINE_STRING = "-------------------------------------------------------------------------------------------------";



#define STR_CASE(TAG)	case(TAG):					\
						{							\
							pString = (char*) #TAG;	\
							break;					\
						}





static char* Ethernet_ErrorCodeToString(uint32_t errorCode)
{
	char* pString;

	switch (errorCode)
	{
		STR_CASE(XLNX_OK)
		STR_CASE(XLNX_ETHERNET_ERROR_NOT_INITIALISED)
		STR_CASE(XLNX_ETHERNET_ERROR_IO_FAILED)
		STR_CASE(XLNX_ETHERNET_ERROR_CHANNEL_OUT_OF_RANGE)
		STR_CASE(XLNX_ETHERNET_ERROR_INVALID_LOOPBACK_TYPE)
		STR_CASE(XLNX_ETHERNET_ERROR_CU_NAME_NOT_FOUND)
		STR_CASE(XLNX_ETHERNET_ERROR_INVALID_CLOCK_SELECT_TYPE)
		STR_CASE(XLNX_ETHERNET_ERROR_INVALID_POLARITY_TYPE)
		STR_CASE(XLNX_ETHERNET_ERROR_INVALID_TEST_PATTERN)
		STR_CASE(XLNX_ETHERNET_ERROR_INVALID_EQUALIZATION_MODE)
		STR_CASE(XLNX_ETHERNET_ERROR_CHANNEL_CONFIGURATION_DISABLED_DUE_TO_HW_LIMITATIONS)
	
		default:
		{
			pString = (char*)"UKNOWN_ERROR";
			break;
		}
	}

	return pString;
}









static bool Ethernet_ParseLoopbackString(char* pToken, GTLoopback* pLoopback)
{
	bool bOKToContinue = true;

	if (strcmp(PCSNEARStr, pToken) == 0)			{ *pLoopback = GTLoopback::NEAR_END_PCS_LOOPBACK;	}
	else if (strcmp(PCSFARStr, pToken) == 0)		{ *pLoopback = GTLoopback::FAR_END_PCS_LOOPBACK;	}
	else if (strcmp(PMANEARStr, pToken) == 0)		{ *pLoopback = GTLoopback::NEAR_END_PMA_LOOPBACK;	}
	else if (strcmp(PMAFARStr, pToken) == 0)		{ *pLoopback = GTLoopback::FAR_END_PMA_LOOPBACK;	}
	else if (strcmp(NoneStr, pToken) == 0)			{ *pLoopback = GTLoopback::NO_LOOPACK;				}
	else
	{
		bOKToContinue = false;
	}



	return bOKToContinue;
}






static char* Ethernet_LoopbackToString(GTLoopback loopback)
{
	const char* pString;

	switch (loopback)
	{
		case(GTLoopback::NEAR_END_PCS_LOOPBACK):
		{
			pString = PCSNEARStr;
			break;
		}

		case(GTLoopback::FAR_END_PCS_LOOPBACK):
		{
			pString = PCSFARStr;
			break;
		}

		case(GTLoopback::NEAR_END_PMA_LOOPBACK):
		{
			pString = PMANEARStr;
			break;
		}

		case(GTLoopback::FAR_END_PMA_LOOPBACK):
		{
			pString = PMAFARStr;
			break;
		}

		case(GTLoopback::NO_LOOPACK):
		{
			pString = NoneStr;
			break;
		}

		default:
		{
			pString = "UNKNOWN";
			break;
		}
	}

	return (char*)pString;
}







static bool Ethernet_ParseClockSelectString(char* pToken, GTClockSelect* pClockSelect)
{
	bool bOKToContinue = true;

	if (strcmp(Static1Str, pToken) == 0)				{ *pClockSelect = GTClockSelect::STATIC_1;			}
	else if (strcmp(OutClkPCSStr, pToken) == 0)			{ *pClockSelect = GTClockSelect::OUTCLKPCS;			}
	else if (strcmp(OutClkPMAStr, pToken) == 0)			{ *pClockSelect = GTClockSelect::OUTCLKPMA;			}
	else if (strcmp(PLLRefClkDiv1Str, pToken) == 0)		{ *pClockSelect = GTClockSelect::PLLREFCLK_DIV1;	}
	else if (strcmp(PLLRefClkDiv2Str, pToken) == 0)		{ *pClockSelect = GTClockSelect::PLLREFCLK_DIV2;	}
	else if(strcmp(ProgDivClkStr, pToken) == 0)			{ *pClockSelect = GTClockSelect::PROGDIVCLK;		}
	else
	{
		bOKToContinue = false;
	}



	return bOKToContinue;
}
		  
				 	


static char* Ethernet_ClockSelectToString(GTClockSelect clockSelect)
{
	const char* pString;

	switch (clockSelect)
	{
		case(GTClockSelect::STATIC_1):
		{
			pString = Static1Str;
			break;
		}

		case(GTClockSelect::OUTCLKPCS):
		{
			pString = OutClkPCSStr;
			break;
		}

		case(GTClockSelect::OUTCLKPMA):
		{
			pString = OutClkPMAStr;
			break;
		}

		case(GTClockSelect::PLLREFCLK_DIV1):
		{
			pString = PLLRefClkDiv1Str;
			break;
		}

		case(GTClockSelect::PLLREFCLK_DIV2):
		{
			pString = PLLRefClkDiv2Str;
			break;
		}

		case(GTClockSelect::PROGDIVCLK):
		{
			pString = ProgDivClkStr;
			break;
		}

		default:
		{
			pString = "UNKNOWN";
			break;
		}
	}

	return (char*)pString;
}








static bool Ethernet_ParsePolarityString(char* pToken, GTPolarity* pPolarity)
{
	bool bOKToContinue = true;

	if (strcmp(NormalStr, pToken) == 0)			{ *pPolarity = GTPolarity::NORMAL;		}
	else if (strcmp(InvertedStr, pToken) == 0)	{ *pPolarity = GTPolarity::INVERTED;	}
	else
	{
		bOKToContinue = false;
	}



	return bOKToContinue;
}








static char* Ethernet_PolarityToString(GTPolarity polarity)
{
	const char* pString;

	switch (polarity)
	{
		case(GTPolarity::NORMAL):
		{
			pString = NormalStr;
			break;
		}

		case(GTPolarity::INVERTED):
		{
			pString = InvertedStr;
			break;
		}

		default:
		{
			pString = "UNKNOWN";
			break;
		}
	}

	return (char*)pString;
}











static bool Ethernet_ParseEqualizationModeString(char* pToken, GTEqualizationMode* pEqualizationMode)
{
	bool bOKToContinue = true;

	if (strcmp(DFEStr, pToken) == 0)			{ *pEqualizationMode = GTEqualizationMode::DFE; }
	else if (strcmp(LPMStr, pToken) == 0)		{ *pEqualizationMode = GTEqualizationMode::LPM; }
	else
	{
		bOKToContinue = false;
	}

	return bOKToContinue;
}




static char* Ethernet_EqualizationModeToString(GTEqualizationMode equalizationMode)
{
	const char* pString;

	switch (equalizationMode)
	{
		case(GTEqualizationMode::DFE):
		{
			pString = DFEStr;
			break;
		}

		case(GTEqualizationMode::LPM):
		{
			pString = LPMStr;
			break;
		}

		default:
		{
			pString = "UNKNOWN";
			break;
		}
	}

	return (char*)pString;
}







static bool Ethernet_ParseTestPatternString(char* pToken, GTTestPattern* pTestPattern)
{
	bool bOKToContinue = true;

	if (strcmp(NoneStr, pToken) == 0)			{ *pTestPattern = GTTestPattern::NO_PATTERN;	}
	else if (strcmp(PRBS7Str, pToken) == 0)		{ *pTestPattern = GTTestPattern::PRBS_7;		}
	else if (strcmp(PRBS9Str, pToken) == 0)		{ *pTestPattern = GTTestPattern::PRBS_9;		}
	else if (strcmp(PRBS15Str, pToken) == 0)	{ *pTestPattern = GTTestPattern::PRBS_15;		}
	else if (strcmp(PRBS23Str, pToken) == 0)	{ *pTestPattern = GTTestPattern::PRBS_23;		}
	else if (strcmp(PRBS31Str, pToken) == 0)	{ *pTestPattern = GTTestPattern::PRBS_31;		}
	else
	{
		bOKToContinue = false;
	}

	return bOKToContinue;
}





static char* Ethernet_TestPatternToString(GTTestPattern testPattern)
{
	const char* pString;

	switch (testPattern)
	{
		case(GTTestPattern::NO_PATTERN):
		{
			pString = NoneStr;
			break;
		}

		case(GTTestPattern::PRBS_7):
		{
			pString = PRBS7Str;
			break;
		}


		case(GTTestPattern::PRBS_9):
		{
			pString = PRBS9Str;
			break;
		}

		case(GTTestPattern::PRBS_15):
		{
			pString = PRBS15Str;
			break;
		}

		case(GTTestPattern::PRBS_23):
		{
			pString = PRBS23Str;
			break;
		}

		case(GTTestPattern::PRBS_31):
		{
			pString = PRBS31Str;
			break;
		}

		default:
		{
			pString = "UNKNOWN";
			break;
		}
	}

	return (char*)pString;
}








static char* Ethernet_RxBufStatusToString(GTRxBufStatus bufStatus)
{
	const char* pString;

	switch (bufStatus)
	{
		case(GTRxBufStatus::NOMINAL):
		{
			pString = NominalStr;
			break;
		}

		case(GTRxBufStatus::FILL_UNDER_CLK_COR_MIN_LAT):
		{
			pString = FillUnderMinLatencyStr;
			break;
		}

		case(GTRxBufStatus::FILL_OVER_CLK_COR_MAX_LAT):
		{
			pString = FillOverMaxLatencyStr;
			break;
		}

		case(GTRxBufStatus::RX_ELASTIC_BUFFER_UNDERFLOW):
		{
			pString = ElasticBufferUnderflowStr;
			break;
		}

		case(GTRxBufStatus::RX_ELASTIC_BUFFER_OVERFLOW):
		{
			pString = ElasticBufferOverflowStr;
			break;
		}

		default:
		{
			pString = "UNKNOWN";
			break;
		}
	}


	return (char*)pString;
}





static char* Ethernet_ResetStateToString(bool bInReset)
{
	const char* pString;

	if (bInReset)
	{
		pString = InResetStr;
	}
	else
	{
		pString = OutOfResetStr;
	}

	return (char*)pString;
}






static int Ethernet_SetKernelReset(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	Ethernet* pEthernet;
	bool bInReset;
	bool bOKToContinue = true;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 2)
	{
		pShell->printf("Usage: %s <bool>\n", argv[0]);
		pShell->printf("       <true> to place block in reset\n");
		pShell->printf("       <false> to take block out of reset\n");
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[1], &bInReset);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}
	

	if (bOKToContinue)
	{
		retval = pEthernet->SetKernelReset(bInReset);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}

	}

	

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}





static int Ethernet_SetConfigAllowed(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	Ethernet* pEthernet;
	uint32_t channel;
	bool bOKToContinue = true;
	bool bAllowed = true;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bAllowed);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}

	if (bOKToContinue)
	{
		pEthernet->SetChannelConfigurationAllowed(channel, bAllowed);
		pShell->printf("OK\n");
	}

	return retval;

}



static int Ethernet_SetMACFilterAddress(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	Ethernet* pEthernet;
	uint32_t channel;
	uint8_t a, b, c, d, e, f;
	bool bOKToContinue = true;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <macaddr>\n", argv[0]);
		bOKToContinue = false;
	}


	if(bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}





	if (bOKToContinue)
	{
		bOKToContinue = ParseMACAddress(pShell, argv[2], &a, &b, &c, &d, &e, &f);
	}




	if (bOKToContinue)
	{
		retval = pEthernet->SetMACFilterAddress(channel, a, b, c, d, e, f);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}






static int Ethernet_SetUnicastPromiscuousMode(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	Ethernet* pEthernet;
	uint32_t channel;
	bool bEnabled;
	bool bOKToContinue = true;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bEnabled);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}



	if (bOKToContinue)
	{
		retval = pEthernet->SetUnicastPromiscuousMode(channel, bEnabled);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}


	return retval;
}









static int Ethernet_SetMulticastPromiscuousMode(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	Ethernet* pEthernet;
	uint32_t channel;
	bool bEnabled;
	bool bOKToContinue = true;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bEnabled);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}



	if (bOKToContinue)
	{
		retval = pEthernet->SetMulticastPromiscuousMode(channel, bEnabled);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}





static int Ethernet_SetTxFIFOThreshold(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	Ethernet* pEthernet;
	uint32_t channel;
	uint32_t thresholdValue;
	bool bOKToContinue = true;


	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <threshold>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[2], &thresholdValue);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse threshold parameter\n");
		}

		if (bOKToContinue)
		{
			if (thresholdValue > 255)
			{
				pShell->printf("[ERROR] Threshold parameter too large (max=255)\n");
				bOKToContinue = false;
			}
		}
	}

	



	if (bOKToContinue)
	{
		retval = pEthernet->SetTxFIFOThreshold(channel, (uint8_t)thresholdValue);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}


	return retval;
}











static int Ethernet_SetRxCutThroughFIFO(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	Ethernet* pEthernet;
	uint32_t channel;
	bool bEnabled;
	bool bOKToContinue = true;


	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bEnabled);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}





	if (bOKToContinue)
	{
		retval = pEthernet->SetRxCutThroughFIFO(channel, bEnabled);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}


	return retval;
}










static int Ethernet_SetLoopback(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	Ethernet* pEthernet;
	uint32_t channel;
	bool bOKToContinue = true;
	GTLoopback loopback;


	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <%s|%s|%s|%s|%s>\n", argv[0], NoneStr, PCSNEARStr, PCSFARStr, PMANEARStr, PMAFARStr);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}



	if (bOKToContinue)
	{
		bOKToContinue = Ethernet_ParseLoopbackString(argv[2], &loopback);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse loopback");
		}
	}



	if (bOKToContinue)
	{
		retval = pEthernet->SetLoopbackType(channel, loopback);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}



	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}









static int Ethernet_GetLoopback(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	Ethernet* pEthernet;
	uint32_t channel;
	bool bOKToContinue = true;
	GTLoopback loopback;


	pEthernet = (Ethernet*)pObjectData;


	if (argc < 2)
	{
		pShell->printf("Usage: %s <channel>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}



	if (bOKToContinue)
	{
		retval = pEthernet->GetLoopbackType(channel, &loopback);
		
		if (retval == XLNX_OK)
		{
			pShell->printf("Channel %u loopback => %s\n", channel, Ethernet_LoopbackToString(loopback));
		}
		else
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] Failed to retrieve loopback, error = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}








static int Ethernet_SetTxClockSelect(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	GTClockSelect clockSelect;

	pEthernet = (Ethernet*)pObjectData;
	

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <%s|%s|%s|%s|%s|%s>\n", argv[0], 
																	Static1Str, 
																	OutClkPCSStr, 
																	OutClkPMAStr, 
																	PLLRefClkDiv1Str, 
																	PLLRefClkDiv2Str, 
																	ProgDivClkStr);
		bOKToContinue = false;
	}



	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}



	if (bOKToContinue)
	{
		bOKToContinue = Ethernet_ParseClockSelectString(argv[2], &clockSelect);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse clock select parameter\n");
		}
	}


	if (bOKToContinue)
	{
		retval = pEthernet->SetTxOutClockSelect(channel, clockSelect);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}


	return retval;
}





static int Ethernet_SetRxClockSelect(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	GTClockSelect clockSelect;

	pEthernet = (Ethernet*)pObjectData;


	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <%s|%s|%s|%s|%s|%s>\n", argv[0],
																	Static1Str,
																	OutClkPCSStr,
																	OutClkPMAStr,
																	PLLRefClkDiv1Str,
																	PLLRefClkDiv2Str,
																	ProgDivClkStr);
		bOKToContinue = false;
	}



	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}



	if (bOKToContinue)
	{
		bOKToContinue = Ethernet_ParseClockSelectString(argv[2], &clockSelect);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse clock select parameter\n");
		}
	}


	if (bOKToContinue)
	{
		retval = pEthernet->SetRxOutClockSelect(channel, clockSelect);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}


	return retval;
}







static int Ethernet_SetTxPolarity(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	GTPolarity polarity;

	pEthernet = (Ethernet*)pObjectData;
	

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <%s|%s>\n", argv[0], 
														NormalStr, 
														InvertedStr);
		bOKToContinue = false;
	}



	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}



	if (bOKToContinue)
	{
		bOKToContinue = Ethernet_ParsePolarityString(argv[2], &polarity);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse polarity parameter\n");
		}
	}


	if (bOKToContinue)
	{
		retval = pEthernet->SetTxPolarity(channel, polarity);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}


	return retval;	
}






static int Ethernet_SetRxPolarity(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	GTPolarity polarity;

	pEthernet = (Ethernet*)pObjectData;
	

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <%s|%s>\n", argv[0], 
														NormalStr, 
														InvertedStr);
		bOKToContinue = false;
	}



	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}



	if (bOKToContinue)
	{
		bOKToContinue = Ethernet_ParsePolarityString(argv[2], &polarity);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse polarity parameter\n");
		}
	}


	if (bOKToContinue)
	{
		retval = pEthernet->SetRxPolarity(channel, polarity);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;	
}








static int Ethernet_SetTxDiffControl(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	uint32_t coeff;

	pEthernet = (Ethernet*)pObjectData;


	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <coeff>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[2], &coeff);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse coefficient\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->SetTxDiffControl(channel, coeff);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}



static int Ethernet_SetTxMainCursor(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	uint32_t coeff;

	pEthernet = (Ethernet*)pObjectData;


	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <coeff>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[2], &coeff);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse coefficient\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->SetTxMainCursor(channel, coeff);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}


	return retval;
}




static int Ethernet_SetTxPreCursor(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	uint32_t coeff;

	pEthernet = (Ethernet*)pObjectData;


	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <coeff>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[2], &coeff);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse coefficient\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->SetTxPreCursor(channel, coeff);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}


	return retval;
}



static int Ethernet_SetTxPostCursor(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	uint32_t coeff;

	pEthernet = (Ethernet*)pObjectData;


	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <coeff>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[2], &coeff);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse coefficient\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->SetTxPostCursor(channel, coeff);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}








static int Ethernet_SetTxInhibit(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	bool bEnabled;

	pEthernet = (Ethernet*)pObjectData;


	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bEnabled);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->SetTxInhibit(channel, bEnabled);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}




static int Ethernet_SetTxElecIdle(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	bool bEnabled;

	pEthernet = (Ethernet*)pObjectData;


	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bEnabled);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->SetTxElectricalIdle(channel, bEnabled);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}







static int Ethernet_SetRxEqualizerMode(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	GTEqualizationMode eqMode;

	pEthernet = (Ethernet*)pObjectData;


	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <dfe|lpm>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}

	if (bOKToContinue)
	{
		bOKToContinue = Ethernet_ParseEqualizationModeString(argv[2], &eqMode);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse equalization mode parameter\n");
		}
	}



	if (bOKToContinue)
	{
		retval = pEthernet->SetRxEqualizationMode(channel, eqMode);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}




static int Ethernet_ResetDFELPM(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;

	pEthernet = (Ethernet*)pObjectData;


	if (argc != 2)
	{
		pShell->printf("Usage: %s <channel>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->ResetLFEDPMDataPath(channel);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}







static int Ethernet_SetTxTestPattern(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	GTTestPattern testPattern;

	pEthernet = (Ethernet*)pObjectData;


	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <%s|%s|%s|%s|%s|%s>\n",	argv[0], 
																	NoneStr,
																	PRBS7Str,
																	PRBS9Str,
																	PRBS15Str,
																	PRBS23Str,
																	PRBS31Str);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}



	if (bOKToContinue)
	{
		bOKToContinue = Ethernet_ParseTestPatternString(argv[2], &testPattern);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse test pattern parameter\n");
		}
	}


	if (bOKToContinue)
	{
		retval = pEthernet->SetTxTestPattern(channel, testPattern);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}



	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}


	return retval;
}



static int Ethernet_SetTxErrorInjection(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	bool bEnabled;

	pEthernet = (Ethernet*)pObjectData;


	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}



	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bEnabled);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}


	if (bOKToContinue)
	{
		retval = pEthernet->SetTxErrorInjection(channel, bEnabled);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}


	return retval;
}






static int Ethernet_SetRxTestPattern(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	GTTestPattern testPattern;

	pEthernet = (Ethernet*)pObjectData;


	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <%s|%s|%s|%s|%s|%s>\n",	argv[0], 
																	NoneStr,
																	PRBS7Str,
																	PRBS9Str,
																	PRBS15Str,
																	PRBS23Str,
																	PRBS31Str);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}



	if (bOKToContinue)
	{
		bOKToContinue = Ethernet_ParseTestPatternString(argv[2], &testPattern);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse test pattern parameter\n");
		}
	}


	if (bOKToContinue)
	{
		retval = pEthernet->SetRxExpectedTestPattern(channel, testPattern);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}





static int Ethernet_GetRxPatternStatus(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	GTTestPattern testPattern;
	bool bLocked;
	bool bLiveError;
	bool bLatchedError;

	pEthernet = (Ethernet*)pObjectData;


	if (argc != 2)
	{
		pShell->printf("Usage: %s <channel>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}


	if (bOKToContinue)
	{
		retval = pEthernet->GetRxExpectedTestPattern(channel, &testPattern);
		if (retval != XLNX_OK)
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
			bOKToContinue = false;
		}
	}




	if (bOKToContinue)
	{
		retval = pEthernet->GetRxTestPatternStatus(channel, &bLocked, &bLiveError, &bLatchedError);
		if (retval != XLNX_OK)
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
			bOKToContinue = false;
		}
	}



	if (bOKToContinue)
	{
		pShell->printf("+-%.20s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
		pShell->printf("| %-20s | %20s |\n", "Expected Pattern",	Ethernet_TestPatternToString(testPattern));
		pShell->printf("| %-20s | %20s |\n", "Pattern Lock",		pShell->boolToString(bLocked));
		pShell->printf("| %-20s | %20s |\n", "Live Error",			pShell->boolToString(bLiveError));
		pShell->printf("| %-20s | %20s |\n", "Latched Error", 		pShell->boolToString(bLatchedError));
		pShell->printf("+-%.20s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
		pShell->printf("\n");
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}



static int Ethernet_ResetRxPatternStatus(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;

	pEthernet = (Ethernet*)pObjectData;


	if (argc != 2)
	{
		pShell->printf("Usage: %s <channel>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}


	if (bOKToContinue)
	{
		retval = pEthernet->ClearRxTestPatternErrorLatch(channel);
		if (retval != XLNX_OK)
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
			bOKToContinue = false;
		}
	}


	if (bOKToContinue)
	{
		retval = pEthernet->ResetRxTestPatternErrorCount(channel);
		if (retval != XLNX_OK)
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
			bOKToContinue = false;
		}
	}


	if (bOKToContinue)
	{
		pShell->printf("OK\n");
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}
	return retval;

}






static int Ethernet_SetTxFIFOReset(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	bool bInReset;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		pShell->printf("       <true> to place block in reset\n");
		pShell->printf("       <false> to take block out of reset\n");
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bInReset);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}


	if (bOKToContinue)
	{
		retval = pEthernet->SetTxFIFOReset(channel, bInReset);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}






static int Ethernet_SetRxFIFOReset(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	bool bInReset;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		pShell->printf("       <true> to place block in reset\n");
		pShell->printf("       <false> to take block out of reset\n");
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bInReset);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}


	if (bOKToContinue)
	{
		retval = pEthernet->SetRxFIFOReset(channel, bInReset);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}


	return retval;
}









static int Ethernet_SetTxTrafficProcessorReset(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	bool bInReset;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		pShell->printf("       <true> to place block in reset\n");
		pShell->printf("       <false> to take block out of reset\n");
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bInReset);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->SetTxTrafficProcessorReset(channel, bInReset);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}


	return retval;
}






static int Ethernet_SetRxTrafficProcessorReset(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	bool bInReset;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		pShell->printf("       <true> to place block in reset\n");
		pShell->printf("       <false> to take block out of reset\n");
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bInReset);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}


	if (bOKToContinue)
	{
		retval = pEthernet->SetRxTrafficProcessorReset(channel, bInReset);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}



	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}




static int Ethernet_SetRxPMAReset(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	uint32_t channel;
	Ethernet* pEthernet;
	bool bInReset;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		pShell->printf("       <true> to place block in reset\n");
		pShell->printf("       <false> to take block out of reset\n");
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bInReset);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->SetRxPMAReset(channel, bInReset);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}







static int Ethernet_SetRxBufReset(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	uint32_t channel;
	Ethernet* pEthernet;
	bool bInReset;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		pShell->printf("       <true> to place block in reset\n");
		pShell->printf("       <false> to take block out of reset\n");
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bInReset);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->SetRxBufReset(channel, bInReset);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}





static int Ethernet_SetRxPCSReset(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	uint32_t channel;
	Ethernet* pEthernet;
	bool bInReset;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		pShell->printf("       <true> to place block in reset\n");
		pShell->printf("       <false> to take block out of reset\n");
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bInReset);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->SetRxPCSReset(channel, bInReset);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}





static int Ethernet_SetEyescanReset(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	uint32_t channel;
	Ethernet* pEthernet;
	bool bInReset;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		pShell->printf("       <true> to place block in reset\n");
		pShell->printf("       <false> to take block out of reset\n");
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bInReset);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->SetEyeScanReset(channel, bInReset);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}









static int Ethernet_SetRxGTWizReset(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	uint32_t channel;
	Ethernet* pEthernet;
	bool bInReset;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		pShell->printf("       <true> to place block in reset\n");
		pShell->printf("       <false> to take block out of reset\n");
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bInReset);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->SetRxGTWizReset(channel, bInReset);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}





static int Ethernet_SetTxPMAReset(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	uint32_t channel;
	Ethernet* pEthernet;
	bool bInReset;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		pShell->printf("       <true> to place block in reset\n");
		pShell->printf("       <false> to take block out of reset\n");
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bInReset);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->SetTxPMAReset(channel, bInReset);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}






static int Ethernet_SetTxPCSReset(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	uint32_t channel;
	Ethernet* pEthernet;
	bool bInReset;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		pShell->printf("       <true> to place block in reset\n");
		pShell->printf("       <false> to take block out of reset\n");
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bInReset);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->SetTxPCSReset(channel, bInReset);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}





static int Ethernet_SetTxGTWizReset(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	uint32_t channel;
	Ethernet* pEthernet;
	bool bInReset;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <channel> <bool>\n", argv[0]);
		pShell->printf("       <true> to place block in reset\n");
		pShell->printf("       <false> to take block out of reset\n");
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[2], &bInReset);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->SetTxGTWizReset(channel, bInReset);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}







static int Ethernet_TriggerEyescan(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	uint32_t channel;
	Ethernet* pEthernet;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 2)
	{
		pShell->printf("Usage: %s <channel>\n", argv[0]);
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->TriggerEyescan(channel);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}










static int Ethernet_ClearStatusLatches(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	uint32_t channel;
	Ethernet* pEthernet;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 2)
	{
		pShell->printf("Usage: %s <channel>\n", argv[0]);
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}

	if (bOKToContinue)
	{
		retval = pEthernet->ClearStatusLatches(channel);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}








static int Ethernet_PrintChannelStatus(Shell* pShell, Ethernet* pEthernet, uint32_t channel)
{
	int retval = 0;
	static const uint32_t FORMAT_BUFFER_LENGTH = 64;
	char formatBuffer[FORMAT_BUFFER_LENGTH + 1];
	Ethernet::StatusFlags statusFlags;
	GTLoopback loopback;
	uint8_t a, b, c, d, e, f;
	bool bPromiscuousMode;
	uint8_t threshold;
	GTClockSelect clockSelect;
	GTPolarity polarity;
	uint32_t coeff;
	bool bEnabled;
	GTTestPattern testPattern;
	bool bLocked;
	bool bLiveError;
	bool bLatchedError;
	GTEqualizationMode eqMode;
	bool bInReset;


	pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
	
	snprintf(formatBuffer, FORMAT_BUFFER_LENGTH, "CHANNEL %u Status", channel);
	pShell->printf("| %-76s |\n", formatBuffer);

	pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);


	retval = pEthernet->GetStatusFlags(channel, &statusFlags);



	if (retval == XLNX_OK)
	{
		if (statusFlags.rxBlockLock.bLockedLive)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "Rx Block Lock", "Status (Live)", "LOCKED");
		}
		else
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "Rx Block Lock", "Status (Live)", "NOT LOCKED");
		}




		if (statusFlags.rxBlockLock.bLockedLatchedLow)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Status (Latched Low)", "LOCKED");
		}
		else
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Status (Latched Low)", "NOT LOCKED");
		}
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("| %-20s | %-30s | %20s |\n", "RxBufStatus", "Status (Live)",		Ethernet_RxBufStatusToString(statusFlags.rxBufStatus.statusLive));
		pShell->printf("| %-20s | %-30s | %20s |\n", "",			"Underflow (Latched)",	pShell->boolToString(statusFlags.rxBufStatus.bUnderflowLatched));
		pShell->printf("| %-20s | %-30s | %20s |\n", "",			"Overflow (Latched)",	pShell->boolToString(statusFlags.rxBufStatus.bOverflowLatched));
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("| %-20s | %-30s | %20s |\n", "TxBufStatus", "FIFO Half Full (Live)",	pShell->boolToString(statusFlags.txBufStatus.bFIFOHalfFull));
		pShell->printf("| %-20s | %-30s | %20s |\n", "",			"FIFO Half Full (Latched)", pShell->boolToString(statusFlags.txBufStatus.bFIFOHalfFullLatchedHigh));
		pShell->printf("| %-20s | %-30s | %20s |\n", "",			"Over/Underflow (Live)",	pShell->boolToString(statusFlags.txBufStatus.bFIFOOverflowUnderflow));
		pShell->printf("| %-20s | %-30s | %20s |\n", "", 			"Over/Underflow (Latched)", pShell->boolToString(statusFlags.txBufStatus.bFIFOOverflowUnderflowLatchedHigh));
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
	}

	if (retval == XLNX_OK)
	{
		pShell->printf("| %-20s | %-30s | %20s |\n", "GT Power Good",	"GT Power Good (Live)",			pShell->boolToString(statusFlags.gtPowerGood.bPowerGoodLive));
		pShell->printf("| %-20s | %-30s | %20s |\n", "",				"GT Power Good (Latched Low)",	pShell->boolToString(statusFlags.gtPowerGood.bPowerGoodLatchedLow));
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
	}

	if (retval == XLNX_OK)
	{
		pShell->printf("| %-20s | %-30s | %20s |\n", "Rx Traffic Proc", "Data FIFO Overflow (Live)",	pShell->boolToString(statusFlags.rxTrafficProcStatus.bDataFIFOOverflowLive));
		pShell->printf("| %-20s | %-30s | %20s |\n", "",				"Data FIFO Overflow (Latched)", pShell->boolToString(statusFlags.rxTrafficProcStatus.bDataFIFOOverflowLatched));
		pShell->printf("| %-20s | %-30s | %20s |\n", "",				"Cmd FIFO Overflow (Live)",		pShell->boolToString(statusFlags.rxTrafficProcStatus.bCommandFIFOOverflowLive));
		pShell->printf("| %-20s | %-30s | %20s |\n", "",				"Cmd FIFO Overflow (Latched)",	pShell->boolToString(statusFlags.rxTrafficProcStatus.bCommandFIFOOverflowLatched));
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("| %-20s | %-30s | %20s |\n", "Tx Traffic Proc",	"FIFO Full (Live)",		pShell->boolToString(statusFlags.txTrafficProcStatus.bFIFOFullLive));
		pShell->printf("| %-20s | %-30s | %20s |\n", "",				"FIFO Full (Latched)",	pShell->boolToString(statusFlags.txTrafficProcStatus.bFIFOFullLatched));
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("| %-20s | %-30s | %20s |\n", "Eysescan", "Data Error (Latched)", pShell->boolToString(statusFlags.eyescanStatus.bEyescanDataErrorLatchedHigh));
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
	}


	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetMACFilterAddress(channel, &a, &b, &c, &d, &e, &f);

		if (retval == XLNX_OK)
		{
			snprintf(formatBuffer, FORMAT_BUFFER_LENGTH, "%02X:%02X:%02X:%02X:%02X:%02X", a, b, c, d, e, f);
			pShell->printf("| %-20s | %-30s | %20s |\n", "Traffic Proc Config", "MAC Filter Address", formatBuffer);
		}
	}
	


	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetUnicastPromiscuousMode(channel, &bPromiscuousMode);

		if (retval == XLNX_OK)
		{	
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Unicast Promiscuous Mode", pShell->boolToString(bPromiscuousMode));
		}
	}



	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetMulticastPromiscuousMode(channel, &bPromiscuousMode);

		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Multicast Promiscuous Mode", pShell->boolToString(bPromiscuousMode));
		}
	}


	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetTxFIFOThreshold(channel, &threshold);

		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20u |\n", "", "Tx Data FIFO Threshold", threshold);
		}
	}



	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetRxCutThroughFIFO(channel, &bEnabled);

		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Rx Cut-Through FIFO Enabled", pShell->boolToString(bEnabled));
		}
	}




	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
	}





	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetLoopbackType(channel, &loopback);

		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "Loopback", "Loopback", Ethernet_LoopbackToString(loopback));
		}
	}



	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
	}



	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetTxOutClockSelect(channel, &clockSelect);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "Clock Selection", "Tx Clock", Ethernet_ClockSelectToString(clockSelect));
		}
	}


	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetRxOutClockSelect(channel, &clockSelect);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Rx Clock", Ethernet_ClockSelectToString(clockSelect));
		}
	}




	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
	}




	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetTxPolarity(channel, &polarity);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "Polarity", "Tx Polarity", Ethernet_PolarityToString(polarity));
		}
	}


	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetRxPolarity(channel, &polarity);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Rx Polarity", Ethernet_PolarityToString(polarity));
		}
	}




	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
	}



	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetTxDiffControl(channel, &coeff);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20u |\n", "Tx Driver", "Tx Diff Ctrl", coeff);
		}
	}

	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetTxMainCursor(channel, &coeff);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20u |\n", "", "Tx Main Cursor", coeff);
		}
	}

	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetTxPreCursor(channel, &coeff);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20u |\n", "", "Tx Pre Cursor", coeff);
		}
	}


	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetTxPostCursor(channel, &coeff);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20u |\n", "", "Tx Post Cursor", coeff);
		}
	}




	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
	}



	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetTxInhibit(channel, &bEnabled);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "Tx Control", "Tx Inhibit", pShell->boolToString(bEnabled));
		}
	}

	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetTxElectricalIdle(channel, &bEnabled);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Tx Electrical Idle", pShell->boolToString(bEnabled));
		}
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
	}


	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetRxEqualizationMode(channel, &eqMode);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "Rx Equalization", "Mode", Ethernet_EqualizationModeToString(eqMode));
		}
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
	}



	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetTxTestPattern(channel, &testPattern);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "Pattern Generator", "Tx Test Pattern", Ethernet_TestPatternToString(testPattern));
		}
	}

	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetTxErrorInjection(channel, &bEnabled);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Tx Error Injection", pShell->boolToString(bEnabled));
		}
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
	}

	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetRxExpectedTestPattern(channel, &testPattern);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "Pattern Checker", "Rx Expected Test Pattern", Ethernet_TestPatternToString(testPattern));
		}
	}

	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetRxTestPatternStatus(channel, &bLocked, &bLiveError, &bLatchedError);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Rx Test Pattern Locked",			pShell->boolToString(bLocked));
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Rx Test Pattern Live Error",		pShell->boolToString(bLiveError));
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Rx Test Pattern Latched Error",	pShell->boolToString(bLiveError));
		}
	}


	pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);




	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetKernelReset(&bInReset);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "Reset States", "Kernel", Ethernet_ResetStateToString(bInReset));
		}
	}

	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetTxFIFOReset(channel, &bInReset);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Tx FIFO", Ethernet_ResetStateToString(bInReset));
		}
	}

	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetRxFIFOReset(channel, &bInReset);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Rx FIFO", Ethernet_ResetStateToString(bInReset));
		}
	}


	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetTxTrafficProcessorReset(channel, &bInReset);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Tx Traffic Proc", Ethernet_ResetStateToString(bInReset));
		}
	}

	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetRxTrafficProcessorReset(channel, &bInReset);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Rx Traffic Proc", Ethernet_ResetStateToString(bInReset));
		}
	}

	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetRxPMAReset(channel, &bInReset);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Rx PMA", Ethernet_ResetStateToString(bInReset));
		}
	}

	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetRxBufReset(channel, &bInReset);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Rx Buf", Ethernet_ResetStateToString(bInReset));
		}
	}
	
	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetRxPCSReset(channel, &bInReset);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Rx PCS", Ethernet_ResetStateToString(bInReset));
		}
	}

	
	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetRxGTWizReset(channel, &bInReset);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Rx GT Wiz", Ethernet_ResetStateToString(bInReset));
		}
	}

	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetTxPMAReset(channel, &bInReset);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Tx PMA", Ethernet_ResetStateToString(bInReset));
		}
	}

	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetTxPCSReset(channel, &bInReset);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Tx PCS", Ethernet_ResetStateToString(bInReset));
		}
	}


	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetTxGTWizReset(channel, &bInReset);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Tx GT Wiz", Ethernet_ResetStateToString(bInReset));
		}
	}


	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetEyeScanReset(channel, &bInReset);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-20s | %-30s | %20s |\n", "", "Eyescan", Ethernet_ResetStateToString(bInReset));
		}
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.20s-+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
		pShell->printf("\n");
	}


	

	return retval;
}










static int Ethernet_GetStatus(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	Ethernet* pEthernet;
	bool bIsInitialised;
	uint32_t cuIndex;
	uint64_t cuAddress;
	uint32_t numSupportedChannels = 0;

	XLNX_UNUSED_ARG(argc);
	XLNX_UNUSED_ARG(argv);


	pEthernet = (Ethernet*)pObjectData;

	pEthernet->IsInitialised(&bIsInitialised);
	if (bIsInitialised == false)
	{
		retval = XLNX_ETHERNET_ERROR_NOT_INITIALISED;
	}




	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
	}



	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetCUIndex(&cuIndex);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-30s | %20u |\n", "CU Index", cuIndex);
		}

	}



	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetCUAddress(&cuAddress);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-30s |   0x%016" PRIX64 " |\n", "CU Address", cuAddress);
		}
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
	}


	if (retval == XLNX_OK)
	{
		retval = pEthernet->GetNumSupportedChannels(&numSupportedChannels);
		if (retval == XLNX_OK)
		{
			pShell->printf("| %-30s | %20u |\n", "Num Supported Channels (HW)", numSupportedChannels);
		}
	}




	if (retval == XLNX_OK)
	{
		pShell->printf("+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING);	
	}









	if (retval == XLNX_OK)
	{
		for (uint32_t i = 0; i < numSupportedChannels; i++)
		{
			pShell->printf("\n\n\n");

			retval = Ethernet_PrintChannelStatus(pShell, pEthernet, i);

			if (retval != XLNX_OK)
			{
				break; //out of loop
			}
		}
	}


	

	


	if (retval != XLNX_OK)
	{
		pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
	}




	return retval;
}











static int Ethernet_PrintStats(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	Ethernet* pEthernet;
	uint32_t channel;
	Ethernet::Stats stats;

	pEthernet = (Ethernet*)pObjectData;

	if (argc != 2)
	{
		pShell->printf("Usage: %s <channel>\n", argv[0]);
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &channel);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse channel\n");
		}
	}


	if (bOKToContinue)
	{
		retval = pEthernet->GetStats(channel, &stats);

		if (retval == XLNX_OK)
		{
			pShell->printf("+-%.32s-+-%.10s-+\n", LINE_STRING, LINE_STRING);
			pShell->printf("| %-32s | %10s |\n", "Counter", "Value");
			pShell->printf("+-%.32s-+-%.10s-+\n", LINE_STRING, LINE_STRING);

			pShell->printf("| %-32s | %10u |\n", "Rx FIFO Overflow Dropped Frames", stats.numFIFOOverflowDroppedFrames);
			pShell->printf("| %-32s | %10u |\n", "Unicast Filter Dropped Frames",	stats.numUnicastFilterDroppedFrames);
			pShell->printf("| %-32s | %10u |\n", "Multicast Filter Dropped Frames", stats.numMulticastFilterDroppedFrames);
			pShell->printf("| %-32s | %10u |\n", "Oversized Dropped Frames",		stats.numOversizedDroppedFrames);		
			pShell->printf("| %-32s | %10u |\n", "Tx FIFO Underflow Count",			stats.txFIFOUnderflowCount);
			pShell->printf("+-%.32s-+-%.10s-+\n", LINE_STRING, LINE_STRING);
			pShell->printf("\n");
		}
		else
		{
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", Ethernet_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}














CommandTableElement XLNX_ETHERNET_COMMAND_TABLE[] =
{
	{"getstatus",				Ethernet_GetStatus,						"",								"Get status"								},
	{"clearlatches",			Ethernet_ClearStatusLatches,			"<channel>",					"Clear/reset status latches"				},
	{"printstats",				Ethernet_PrintStats,					"<channel>",					"Prints stats counters"						},	
	{"setconfigallowed",		Ethernet_SetConfigAllowed,				"<channel> <bool>",				"Toggles config writes allowed"				},	
	{/*-------------------------------------------------------------------------------------------------------------------------------------------*/},
	{"setmacfilteraddr",		Ethernet_SetMACFilterAddress,			"<channel> <macaddr>",			"Set MAC address used for MAC filtering"	},
	{"setucastprom",			Ethernet_SetUnicastPromiscuousMode,		"<channel> <bool>",				"Set UNICAST promiscuous mode"				},
	{"setmcastprom",			Ethernet_SetMulticastPromiscuousMode,	"<channel> <bool>",				"Set MULTICAST promiscuous mode"			},
	{"settxfifothreshold",		Ethernet_SetTxFIFOThreshold,			"<channel> <theshold>",			"Set data release theshold for Tx Data FIFO"},
	{"setrxcutthroughfifo",		Ethernet_SetRxCutThroughFIFO,			"<channel> <bool>",				"Enable/disable the Rx Cut-Through FIFO"	},
	{/*-------------------------------------------------------------------------------------------------------------------------------------------*/},
	{"setloopback",				Ethernet_SetLoopback,					"<channel> <loopback>",			"Set a GT loopback type"					},
	{"getloopback",				Ethernet_GetLoopback,					"<channel>",					"Get the current GT loopback"				},
	{/*-------------------------------------------------------------------------------------------------------------------------------------------*/},
	{"settxclksel",				Ethernet_SetTxClockSelect,				"<channel> <clksel>",			"Set Tx Clock Select"						},
	{"setrxclksel",				Ethernet_SetRxClockSelect,				"<channel> <clksel>",			"Set Rx Clock Select"						},
	{/*-------------------------------------------------------------------------------------------------------------------------------------------*/},
	{"settxpolarity",			Ethernet_SetTxPolarity,					"<channel> <polarity>",			"Set Tx Polarity"							},
	{"setrxpolarity",			Ethernet_SetRxPolarity,					"<channel> <polarity>",			"Set Rx Polarity"							},
	{/*-------------------------------------------------------------------------------------------------------------------------------------------*/},
	{"settxdiffctrl",			Ethernet_SetTxDiffControl,				"<channel> <coeff>",			"Set Tx Driver Swing"						},
	{"settxmaincursor",			Ethernet_SetTxMainCursor,				"<channel> <coeff>",			"Set Tx Driver Main-Cursor"					},
	{"settxprecursor",			Ethernet_SetTxPreCursor,				"<channel> <coeff>",			"Set Tx Driver Pre-Cursor"					},
	{"settxpostcursor",			Ethernet_SetTxPostCursor,				"<channel> <coeff>",			"Set Tx Driver Post-Cursor"					},
	{/*-------------------------------------------------------------------------------------------------------------------------------------------*/},
	{"settxinhibit",			Ethernet_SetTxInhibit,					"<channel> <bool>",				"Inhibit data transmission",				},
	{"settxelecidle",			Ethernet_SetTxElecIdle,					"<channel> <bool>",				"Transmit electrical idle signal"			},
	{/*-------------------------------------------------------------------------------------------------------------------------------------------*/},
	{"setrxequalizermode",		Ethernet_SetRxEqualizerMode,			"<channel> <eqmode>",			"Set Rx equalizer mode (DFE/LPM)"			},
	{"resetdfelpm",				Ethernet_ResetDFELPM,					"<channel>",					"Reset DFE/LPM datapath"					},
	{/*-------------------------------------------------------------------------------------------------------------------------------------------*/},
	{"settxtestpattern",		Ethernet_SetTxTestPattern,				"<channel> <pattern>",			"Transmit a PRBS test pattern "				},
	{"settxerrorinject",		Ethernet_SetTxErrorInjection,			"<channel> <bool>",				"Inject errors into Tx test pattern"		},
	{/*-------------------------------------------------------------------------------------------------------------------------------------------*/},
	{"setrxtestpattern",		Ethernet_SetRxTestPattern,				"<channel> <pattern>",			"Set expected Rx test pattern"				},
	{"getrxpatternstatus",		Ethernet_GetRxPatternStatus,			"<channel>",					"Get status of Rx test pattern"				},
	{"resetrxpatternstatus",	Ethernet_ResetRxPatternStatus,			"<channel>",					"Reset Rx latched errors and count"			},
	{/*-------------------------------------------------------------------------------------------------------------------------------------------*/},
	{"setkernelreset",			Ethernet_SetKernelReset,				"<bool>",						"Set whole kernel reset"					},
	{"settxfiforeset",			Ethernet_SetTxFIFOReset,				"<channel> <bool>",				"Set Tx FIFO reset"							},
	{"setrxfiforeset",			Ethernet_SetRxFIFOReset,				"<channel> <bool>",				"Set Rx FIFO reset"							},
	{"settxtrafprocreset",		Ethernet_SetTxTrafficProcessorReset,	"<channel> <bool>",				"Set Tx Traffic Processor reset"			},
	{"setrxtrafprocreset",		Ethernet_SetRxTrafficProcessorReset,	"<channel> <bool>",				"Set Rx Traffic Processor reset"			},
	{/*-------------------------------------------------------------------------------------------------------------------------------------------*/},	
	{"setrxpmareset",			Ethernet_SetRxPMAReset,					"<channel> <bool>",				"Sets rxpmareset port on GT"				},
	{"setrxbufreset",			Ethernet_SetRxBufReset,					"<channel> <bool>",				"Sets rxbufreset port on GT"				},
	{"setrxpcsreset",			Ethernet_SetRxPCSReset,					"<channel> <bool>",				"Sets rxpcsreset port on GT"				},
	{"setrxgtwizreset",			Ethernet_SetRxGTWizReset,				"<channel> <bool>",				"Sets rx_datapath_reset pin on GT Wizard"	},
	{/*-------------------------------------------------------------------------------------------------------------------------------------------*/},
	{"settxpmareset",			Ethernet_SetTxPMAReset,					"<channel> <bool>",				"Sets txpmareset port on GT"				},
	{"settxpcsreset",			Ethernet_SetTxPCSReset,					"<channel> <bool>",				"Sets txpcsreset port on GT"				},
	{"settxgtwizreset",			Ethernet_SetTxGTWizReset,				"<channel> <bool>",				"Sets tx_datapath_reset pin on GT Wizard"	},
	{/*-------------------------------------------------------------------------------------------------------------------------------------------*/},
	{"seteyescanreset",			Ethernet_SetEyescanReset,				"<channel> <bool>",				"Sets eyescanreset port on GT"				},	
	{"triggereyescan",			Ethernet_TriggerEyescan,				"<channel>",					"Triggers an eyescan event"					}
};


const uint32_t XLNX_ETHERNET_COMMAND_TABLE_LENGTH = (uint32_t)(sizeof(XLNX_ETHERNET_COMMAND_TABLE) / sizeof(XLNX_ETHERNET_COMMAND_TABLE[0]));

