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
#include <cinttypes>
#include <cstdio>
#include <cstring>

#include "xlnx_shell.h"
#include "xlnx_shell_cmds.h"
#include "xlnx_shell_bitstream_utils.h"

using namespace XLNX;





#ifndef XLNX_UNUSED_ARG
#define XLNX_UNUSED_ARG(x)		(x = x)
#endif







static int HelpCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;

	XLNX_UNUSED_ARG(pShell);
	XLNX_UNUSED_ARG(argc);
	XLNX_UNUSED_ARG(argv);
	XLNX_UNUSED_ARG(pObjectData);

	// This is a effectively a dummy function 
	// The handling of the "help" is done differently, from within the command manager itself
	

	return retval;
}





static int ExitCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;

	XLNX_UNUSED_ARG(argc);
	XLNX_UNUSED_ARG(argv);
	XLNX_UNUSED_ARG(pObjectData);

	pShell->Exit();

	return retval;
}






static int HistoryCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	HistoryBuffer* pHistoryBuffer;
	uint32_t numHistoryCommands;

	XLNX_UNUSED_ARG(argc);
	XLNX_UNUSED_ARG(argv);
	XLNX_UNUSED_ARG(pObjectData);


	pHistoryBuffer = pShell->GetHistoryBuffer();

	numHistoryCommands = pHistoryBuffer->GetNumCommands();

	for (uint32_t i = 0; i < numHistoryCommands; i++)
	{
		pShell->printf("% 4d  %s\n\r", i + 1, pHistoryBuffer->GetCommand(i));
	}

	return retval;
}






static int RegRDCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	bool bOKToContinue = true;
	uint64_t address;
	uint32_t value;
	DeviceInterface* pDeviceInterface;


	XLNX_UNUSED_ARG(pObjectData);

	if (argc != 2)
	{
		pShell->printf("Usage: %s <address>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseHex64(argv[1], &address);

		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Invalid Address\n");
		}
	}




	if (bOKToContinue)
	{
		pDeviceInterface = pShell->GetDeviceInterface();

		retval = pDeviceInterface->ReadReg32(address, &value);

		if (retval == XLNX_OK)
		{
			pShell->printf("0x%08X : 0x%08X\n", address, value);
		}
		else
		{
			pShell->printf("[ERROR] Read Failed\n");
			bOKToContinue = false;
		}
		
	}

	



	return retval;
}







static int RegWRCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	bool bOKToContinue = true;
	uint64_t address;
	uint32_t value;
	DeviceInterface* pDeviceInterface;

	XLNX_UNUSED_ARG(pObjectData);

	if (argc != 3)
	{
		pShell->printf("Usage: %s <address> <value>\n", argv[0]);
		bOKToContinue = false;
	}




	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseHex64(argv[1], &address);

		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Invalid Address\n");
		}
	}



	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseHex32(argv[2], &value);

		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Invalid Value\n");
		}
	}




	if (bOKToContinue)
	{
		pDeviceInterface = pShell->GetDeviceInterface();

		retval = pDeviceInterface->WriteReg32(address, value);

		if (retval != XLNX_OK)
		{
			pShell->printf("[ERROR] Write Failed\n");
			bOKToContinue = false;
		}
	}





	return retval;
}







static int RegWRMCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	bool bOKToContinue = true;
	uint64_t address;
	uint32_t value;
	uint32_t mask;
	DeviceInterface* pDeviceInterface;

	XLNX_UNUSED_ARG(pObjectData);

	if (argc != 4)
	{
		pShell->printf("Usage: %s <address> <value> <mask>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseHex64(argv[1], &address);

		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Invalid Address\n");
		}
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseHex32(argv[2], &value);

		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Invalid Value\n");
		}
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseHex32(argv[3], &mask);

		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Invalid Mask\n");
		}
	}


	if (bOKToContinue)
	{
		pDeviceInterface = pShell->GetDeviceInterface();

		retval = pDeviceInterface->WriteRegWithMask32(address, value, mask);
		if (retval != XLNX_OK)
		{
			pShell->printf("[ERROR] Write Failed\n");
			bOKToContinue = false;
		}

	}





	return retval;
}









static char ConvertToPrintable(char c)
{
	char retChar;

	if (isprint((unsigned char)c))
	{
		retChar = c;
	}
	else
	{
		retChar = '.';
	}

	return retChar;

}











#define HEX_DUMP_NUM_WORDS_PER_ROW			(4)
#define HEX_DUMP_NUM_WORDS_TO_READ          (64)

static void PrintHexDump(Shell* pShell, uint64_t address, uint32_t* buffer)
{
	uint32_t numWords;
	uint32_t numRows;
	uint32_t bufferIndex;
	uint32_t i;
	uint32_t j;
	
	uint32_t data[HEX_DUMP_NUM_WORDS_TO_READ];
	char ascii[HEX_DUMP_NUM_WORDS_PER_ROW][sizeof(uint32_t)];
	uint64_t addressOffset;

	numWords = HEX_DUMP_NUM_WORDS_TO_READ;
	numRows = numWords / HEX_DUMP_NUM_WORDS_PER_ROW;
	if (numWords % HEX_DUMP_NUM_WORDS_PER_ROW != 0)
	{
		numRows++;
	}

	pShell->printf("\n");
	pShell->printf("+--------------------+-------------------------------------+---------------------+\n");
	pShell->printf("|       Offset       |              Hex Data               |        ASCII        |\n");
	pShell->printf("+--------------------+-------------------------------------+---------------------+\n");


	for (i = 0; i < numRows; i++)
	{
		for (j = 0; j < HEX_DUMP_NUM_WORDS_PER_ROW; j++)
		{
			bufferIndex = (i * HEX_DUMP_NUM_WORDS_PER_ROW) + j;
			if (bufferIndex < numWords)
			{
				data[j] = buffer[bufferIndex];
			}
			else
			{
				data[j] = 0x00000000;
			}
		}

		for (j = 0; j < HEX_DUMP_NUM_WORDS_PER_ROW; j++)
		{
			ascii[j][0] = ConvertToPrintable((char)((data[j] >> 24) & 0xFF));
			ascii[j][1] = ConvertToPrintable((char)((data[j] >> 16) & 0xFF));
			ascii[j][2] = ConvertToPrintable((char)((data[j] >> 8) & 0xFF));
			ascii[j][3] = ConvertToPrintable((char)((data[j] >> 0) & 0xFF));
		}

		addressOffset = address + (uint32_t)(i * (HEX_DUMP_NUM_WORDS_PER_ROW * sizeof(uint32_t)));
		pShell->printf("| 0x%016" PRIx64 " | ", addressOffset);

		for (j = 0; j < HEX_DUMP_NUM_WORDS_PER_ROW; j++)
		{
			pShell->printf("%08X ", data[j]);
		}

		pShell->printf("| ");

		for (j = 0; j < HEX_DUMP_NUM_WORDS_PER_ROW; j++)
		{
			pShell->printf("%c%c%c%c ", ascii[j][0], ascii[j][1], ascii[j][2], ascii[j][3]);
		}

		pShell->printf("|\n");
	}

	pShell->printf("+--------------------+-------------------------------------+---------------------+\n");

}







static int RegHexDumpCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	bool bOKToContinue = true;
	int retval;
	uint64_t address;
	uint32_t buffer[HEX_DUMP_NUM_WORDS_TO_READ];
	
	DeviceInterface* pDeviceInterface = pShell->GetDeviceInterface();

	XLNX_UNUSED_ARG(pObjectData);

	if (argc != 2)
	{
		pShell->printf("Usage: %s <address>\n", argv[0]);
		bOKToContinue = false;
	}



	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseHex64(argv[1], &address);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Invalid Address\n");
		}
	}




	if (bOKToContinue)
	{
		retval = pDeviceInterface->BlockReadReg32(address, buffer, HEX_DUMP_NUM_WORDS_TO_READ);

		if (retval != XLNX_OK)
		{
			pShell->printf("[ERROR] Read Failed\n");
			bOKToContinue = false;
		}
	}




	if (bOKToContinue)
	{
		PrintHexDump(pShell, address, buffer);
	}
	

	return bOKToContinue;
}













static int MemRDCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	uint64_t address;
	uint32_t value;
	DeviceInterface* pDeviceInterface;

	XLNX_UNUSED_ARG(pObjectData);

	if (argc != 2)
	{
		pShell->printf("Usage: %s <address>\n", argv[0]);
		bOKToContinue = false;
	}




	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseHex64(argv[1], &address);

		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Invalid Address\n");
		}
	}




	if (bOKToContinue)
	{
		pDeviceInterface = pShell->GetDeviceInterface();

		retval = pDeviceInterface->ReadMem32(address, &value);

		if (retval == XLNX_OK)
		{
			pShell->printf("0x%08X : 0x%08X\n", address, value);
		}
		else
		{
			pShell->printf("[ERROR] Read Failed\n");
		}
		
	}





	return retval;
}







static int MemWRCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	uint64_t address;
	uint32_t value;
	DeviceInterface* pDeviceInterface;

	XLNX_UNUSED_ARG(pObjectData);

	if (argc != 3)
	{
		pShell->printf("Usage: %s <address> <value>\n", argv[0]);
		bOKToContinue = false;
	}




	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseHex64(argv[1], &address);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Invalid Address\n");
		}
	}




	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseHex32(argv[2], &value);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Invalid Value\n");
		}
	}





	if (bOKToContinue)
	{
		pDeviceInterface = pShell->GetDeviceInterface();

		retval = pDeviceInterface->WriteMem32(address, value);

		if (retval != XLNX_OK)
		{
			pShell->printf("[ERROR] Write Failed\n");
		}
	}





	return retval;
}







static int MemWRMCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	uint64_t address;
	uint32_t value;
	uint32_t mask;
	DeviceInterface* pDeviceInterface;

	XLNX_UNUSED_ARG(pObjectData);

	if (argc != 4)
	{
		pShell->printf("Usage: %s <address> <value> <mask>\n", argv[0]);
		bOKToContinue = false;
	}




	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseHex64(argv[1], &address);

		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Invalid Address\n");
		}
	}




	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseHex32(argv[2], &value);

		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Invalid Value\n");
		}
	}




	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseHex32(argv[3], &mask);

		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Invalid Mask\n");
		}
	}




	if (bOKToContinue)
	{
		pDeviceInterface = pShell->GetDeviceInterface();

		retval = pDeviceInterface->WriteMemWithMask32(address, value, mask);

		if (retval != XLNX_OK)
		{
			pShell->printf("[ERROR] Write Failed\n");
		}
	}





	return retval;
}







static int MemHexDumpCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	bool bOKToContinue = true;
	uint64_t address;
	uint32_t buffer[HEX_DUMP_NUM_WORDS_TO_READ];

	DeviceInterface* pDeviceInterface = pShell->GetDeviceInterface();

	XLNX_UNUSED_ARG(pObjectData);



	if (argc != 2)
	{
		pShell->printf("Usage: %s <address>\n", argv[0]);
		bOKToContinue = false;
	}



	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseHex64(argv[1], &address);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Invalid Address\n");
		}
	}




	if (bOKToContinue)
	{
		retval = pDeviceInterface->BlockReadMem32(address, buffer, HEX_DUMP_NUM_WORDS_TO_READ);
		
		if (retval != XLNX_OK)
		{
			pShell->printf("[ERROR] Read Failed\n");
		}
	}




	if (bOKToContinue)
	{
		PrintHexDump(pShell, address, buffer);
	}


	return retval;
}







static int DownloadCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	DeviceInterface* pDeviceInterface = pShell->GetDeviceInterface();
	char* pFilePath;

	XLNX_UNUSED_ARG(pObjectData);

	if (argc != 2)
	{
		pShell->printf("Usage: %s <filepath>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		pShell->InvokePreDownloadCallback();
	}


	if(bOKToContinue)
	{

		pFilePath = argv[1];

		pShell->printf("Downloading XCLBIN file: %s\n", pFilePath);

		retval = pDeviceInterface->DownloadBitstream(pFilePath);

		switch (retval)
		{
			case(XLNX_OK):
			{
				pShell->printf("Download SUCCESSFUL\n");
				break;
			}

			case(XLNX_DEV_INTERFACE_ERROR_FAILED_TO_OPEN_FILE):
			{
				pShell->printf("[ERROR] Download FAILED - Failed to open file '%s'\n", argv[1]);
				break;
			}

			case(XLNX_DEV_INTERFACE_ERROR_FAILED_TO_LOCK_DEVICE):
			{
				pShell->printf("[ERROR] Download FAILED - Failed to lock device\n");
				break;
			}

			case(XLNX_DEV_INTERFACE_ERROR_FAILED_TO_DOWNLOAD_BITSTREAM):
			{
				pShell->printf("[ERROR] Download FAILED - Failed to download bitstream\n");
				break;
			}

			default:
			{
				pShell->printf("[ERROR] Download FAILED - Unknown reason (%d)\n", retval);
				break;
			}

		}


		//If the download was successful, invoke the user-defined callback.
		//This is intended to re-initialise any design-specific software objects to use the new HW load...
		if (retval == XLNX_OK)
		{
			pShell->InvokePostDownloadCallback();
		}
	}


	return retval;
}









static int XCLBINInfoCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	bool bOKToContinue = true;
	int retval = 0;

	FILE* pFile = nullptr;
	unsigned long fileLength = 0;

	struct axlf* pAXLF = nullptr;
	

	XLNX_UNUSED_ARG(pObjectData);


	if (argc != 2)
	{
		pShell->printf("Usage: %s <xclbinpath>\n", argv[0]);
		bOKToContinue = false;
	}



	

	if (bOKToContinue)
	{
		pFile = fopen(argv[1], "rb");
		if (pFile == nullptr)
		{
			pShell->printf("[ERROR] Failed to open file %s\n", argv[1]);
			bOKToContinue = false;
		}
	}
	



	if (bOKToContinue)
	{
		// Allocate a buffer big enough to hold the entire XCLBIN, and suck the file contents into it...
		fseek(pFile, 0, SEEK_END);
		fileLength = ftell(pFile);
		rewind(pFile);

		pAXLF = (struct axlf*) new uint8_t[fileLength];

		fread(pAXLF, 1, fileLength, pFile);
		fclose(pFile);
	}



	///////////////
	// Header Info 
	///////////////
	if (bOKToContinue)
	{
		pShell->printf("\nHEADER INFO\n");
		PrintXCLBINHeaderInfo(pShell, pAXLF);

	}



	////////////////
	// Section Info
	////////////////
	if (bOKToContinue)
	{
		pShell->printf("\n\nSECTION INFO\n");
		PrintXCLBINSectionInfo(pShell, pAXLF);
	}


	/////////////
	// IP Layout 
	/////////////
	if (bOKToContinue)
	{
		pShell->printf("\n\nIP LAYOUT\n");
		PrintXCLBINIPLayout(pShell, pAXLF);
	}



	///////////////////
	// Memory Topology
	///////////////////
	if (bOKToContinue)
	{
		pShell->printf("\n\nMEM TOPOLOGY\n");
		PrintXCLBINMemTopology(pShell, pAXLF);
	}



	////////////////
	// Connectivity
	////////////////
	if (bOKToContinue)
	{
		pShell->printf("\n\nCONNECTIVITY\n");
		PrintXCLBINConnectivity(pShell, pAXLF);
	}



	////////////////////////////
	// Clock Frequency Topology
	///////////////////////////
	if (bOKToContinue)
	{
		pShell->printf("\n\nCLOCK FREQUENCY TOPOLOGY\n");
		PrintXCLBINClockFrequency(pShell, pAXLF);
	}


	//////////////
	// Kernel Args
	//////////////
	if (bOKToContinue)
	{
		pShell->printf("\n\nKERNEL ARGS\n");
		PrintXCLBINKernelArgs(pShell, pAXLF);
	}




	// Cleanup
	if (pAXLF != nullptr)
	{
		delete[] pAXLF;
		pAXLF = nullptr;
	}

	
	return retval;
}








static int BitstreamInfoCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	struct mem_data memDataElement;
	struct connection connectionElement;
	struct ip_data ipDataElement;
	//struct clock_freq clockFreqElement; TODO
	size_t size;
	int elementIndex;
	xuid_t* pBitstreamUUID;
	static const uint32_t BUFFER_SIZE = 128;
	char formatBuffer[BUFFER_SIZE + 1];
	bool bOKToContinue = true;

	DeviceInterface* pDeviceInterface = pShell->GetDeviceInterface();


	XLNX_UNUSED_ARG(argc);
	XLNX_UNUSED_ARG(argv);
	XLNX_UNUSED_ARG(pObjectData);


	pShell->printf("\n\nBITSTREAM UUID\n");
	pBitstreamUUID = pDeviceInterface->GetBitstreamUUID();
	uuid_unparse_lower(*pBitstreamUUID, formatBuffer);
	pShell->printf("%s\n", formatBuffer);



//In emulation mode, the UUID will be empty, so we want to skip the null check
//i.e. only do the check if we are NOT in emulation mode
#ifndef XCL_EMULATION_MODE
	if (uuid_is_null(*pBitstreamUUID))
	{
		pShell->printf("Device NOT PROGRAMMED\n");
		bOKToContinue = false;
	}
#endif


	if (bOKToContinue)
	{

		pShell->printf("\n\nIP LAYOUT\n");
		PrintIPLayoutTableHeader(pShell);

		retval = XLNX_OK;
		elementIndex = 0;
		while (retval == XLNX_OK)
		{
			retval = pDeviceInterface->GetBitstreamSectionInfo(&ipDataElement, &size, axlf_section_kind::IP_LAYOUT, elementIndex);

			if (retval == XLNX_OK)
			{
				PrintIPLayoutTableElement(pShell, elementIndex, &ipDataElement);
				elementIndex++;
			}
		}

		PrintIPLayoutTableFooter(pShell);










		pShell->printf("\n\nMEM TOPOLOGY\n");
		PrintMemTopologyTableHeader(pShell);

		retval = XLNX_OK;
		elementIndex = 0;
		while (retval == XLNX_OK)
		{
			retval = pDeviceInterface->GetBitstreamSectionInfo(&memDataElement, &size, axlf_section_kind::MEM_TOPOLOGY, elementIndex);

			if (retval == XLNX_OK)
			{
				PrintMemTopologyTableElement(pShell, elementIndex, &memDataElement);
				elementIndex++;
			}
		}

		PrintMemTopologyTableFooter(pShell);












		pShell->printf("\n\nCONNECTIVITY\n");
		PrintConnectivityTableHeader(pShell);

		retval = XLNX_OK;
		elementIndex = 0;

		while (retval == XLNX_OK)
		{
			retval = pDeviceInterface->GetBitstreamSectionInfo(&connectionElement, &size, axlf_section_kind::CONNECTIVITY, elementIndex);

			if (retval == XLNX_OK)
			{
				PrintConnectivityTableElement(pShell, elementIndex, &connectionElement);
				elementIndex++;
			}
		}

		PrintConnectivityTableFooter(pShell);








		//TODO - Retrieving clock frequency topology is not yet supported by XRT.

		//pShell->printf("\n\nCLOCK FREQUENCY TOPOLOGY\n");
		//PrintClockFrequencyTopologyTableHeader(pShell);
		//
		//retval = XLNX_OK;
		//elementIndex = 0;
		//
		//while (retval == XLNX_OK)
		//{
		//	retval = pDeviceInterface->GetBitstreamSectionInfo(&clockFreqElement, &size, axlf_section_kind::CLOCK_FREQ_TOPOLOGY, elementIndex);
		//
		//	if (retval == XLNX_OK)
		//	{
		//		PrintClockFrequencyTopologyTableElement(pShell, elementIndex, &clockFreqElement);
		//		elementIndex++;
		//	}
		//}
		//
		//PrintClockFrequencyTopologyTableFooter(pShell);
	}



	return retval;
}




static int MACAddrCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	DeviceInterface* pDeviceInterface;
	MACAddresses* pMACAddresses;

	XLNX_UNUSED_ARG(argc);
	XLNX_UNUSED_ARG(argv);
	XLNX_UNUSED_ARG(pObjectData);

	pDeviceInterface = pShell->GetDeviceInterface();
	pMACAddresses = pDeviceInterface->GetMACAddresses();

	pShell->printf("\n");
	pShell->printf("Addresses from EEPROM:\n");
	pShell->printf("----------------------\n");
	for (uint32_t i = 0; i < MACAddresses::NUM_SUPPORTED_MAC_ADDRESSES; i++)
	{
		if (pMACAddresses->isValid[i])
		{
			pShell->printf("MAC Address [%u] = %02X:%02X:%02X:%02X:%02X:%02X\n", i, pMACAddresses->addr[i][0],
																				    pMACAddresses->addr[i][1],
																				    pMACAddresses->addr[i][2],
																				    pMACAddresses->addr[i][3],
																				    pMACAddresses->addr[i][4],
																				    pMACAddresses->addr[i][5]);
		}
		else
		{
			pShell->printf("MAC Address [%u] = N/A\n", i);
		}
		
	}

	pShell->printf("\n");

	return retval;
}






static int ClocksCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	DeviceInterface* pDeviceInterface;
	uint32_t clocksMHz[DeviceInterface::MAX_SUPPORTED_CLOCKS];
	uint32_t numClocks;

	XLNX_UNUSED_ARG(argc);
	XLNX_UNUSED_ARG(argv);
	XLNX_UNUSED_ARG(pObjectData);

	pDeviceInterface = pShell->GetDeviceInterface();

	retval = pDeviceInterface->GetClocks(clocksMHz, &numClocks);

	if (retval == XLNX_OK)
	{
		for (uint32_t i = 0; i < numClocks; i++)
		{
			if (i < DeviceInterface::MAX_SUPPORTED_CLOCKS)
			{
				pShell->printf("Clock[%u] = %u MHz\n", i, clocksMHz[i]);
			}
		}
	}
	else
	{
		pShell->printf("[ERROR] Failed to read clock frequencies - retval = 0x%08X\n", retval);
	}

	return retval;
}




static int RunCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	bool bOKToContinue = true;

	XLNX_UNUSED_ARG(pObjectData);

	if (argc != 2)
	{
		pShell->printf("Usage: %s <filepath>\n", argv[0]);
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->PlayScript(argv[1]);
	}

	return retval;
}












CommandTableElement XLNX::XLNX_SHELL_COMMANDS_TABLE[] =
{
	//	Command				Function				Args List					Help String
	//	--------------------------------------------------------------------------------------------------------------------
	{	"help",				HelpCommand,			"[all]",					"Prints list of objects and commands"		},
	{	"exit",				ExitCommand,			"",							"Exit shell instance"						},
	{	"quit",				ExitCommand,			"",							"Same as 'exit'"							},
	{	"history",			HistoryCommand,			"",							"Lists previous commands"					},
	{	"regrd",			RegRDCommand,			"<addr>",					"Reads a single 32-bit register"			},
	{	"regwr",			RegWRCommand,			"<addr> <value>",			"Write to a single 32-bit register"			},
	{	"regwrm",			RegWRMCommand,			"<addr> <value> <mask>",	"Write to a register with mask"				},
	{	"reghexdump",		RegHexDumpCommand,		"<addr>",					"Hex dump registers starting at <addr>"		},
	{	"memrd",			MemRDCommand,			"<addr>",					"Read a single 32-bit memory location"		},
	{	"memwr",			MemWRCommand,			"<addr> <value>",			"Write to a single 32-bit memory location"	},
	{	"memwrm",			MemWRMCommand,			"<addr> <value> <mask>",	"Write to a memory location with mask"		},
	{	"memhexdump",		MemHexDumpCommand,		"<addr>",					"Hex dump for memory starting at <add>"		},
	{	"download",			DownloadCommand,		"<filepath>",				"Download a bitstream"						},
	{	"xclbininfo",		XCLBINInfoCommand,		"<filepath>",				"Outputs details on XCLBIN file contents"	},
	{	"bitstreaminfo",	BitstreamInfoCommand,	"",							"Outputs details of RUNNING bitstream"		},
	{	"macaddr",			MACAddrCommand,			"",							"Lists the MAC addresses for the card"		},
	{	"clocks",			ClocksCommand,			"",							"Lists clocks of the RUNNING bitstream"		},
	{	"run",				RunCommand,				"<filepath>",				"Executes a script containing shell cmds"	}
};




const uint32_t XLNX::NUM_SHELL_COMMANDS = (uint32_t) (sizeof(XLNX_SHELL_COMMANDS_TABLE) / sizeof(XLNX_SHELL_COMMANDS_TABLE[0]));










