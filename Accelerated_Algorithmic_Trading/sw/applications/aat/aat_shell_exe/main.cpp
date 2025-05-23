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


#include <cstdio>
#include <cstdint>
#include <cstring>

#include <signal.h>


#include "xlnx_shell.h"
#include "xlnx_istream_linux_console.h"
#include "xlnx_ostream_linux_console.h"
#include "xlnx_istream_windows_console.h"
#include "xlnx_ostream_windows_console.h"

#include "xlnx_device_manager.h"

#include "xlnx_aat.h"
#include "xlnx_shell_aat_objects.h"

#include "xlnx_shell_network_capture.h"
#include "xlnx_shell_network_tap.h"

using namespace XLNX;






//
//Globals
//
Shell g_shell;
DeviceManager g_deviceManager;
AAT g_aat;








#ifndef XLNX_UNUSED_ARG
#define XLNX_UNUSED_ARG(x)		(x = x)
#endif



#define XSTR(s) STR(s)
#define STR(s)	#s






typedef struct
{
	bool bDeviceIndexSupplied;
	uint32_t deviceIndex;

	bool bInlineCommandSupplied;
	uint32_t inlineCommandStartIndex;

	bool bInteractiveMode;

}CommandLineOptions;


CommandLineOptions commandLineOptions;












void PrintUsage(char* progName)
{
	printf("Usage: %s [-d deviceindex] [-i] [cmd]\n", progName);
	printf("[-d] device selection (when multiple cards are present)\n");
	printf("[-i] forces the shell in interactive mode instead of exiting (i.e. one-shot mode) after executing the supplied command\n");
}






bool PromptUserForDeviceSelection(uint32_t* pSelectedDeviceIndex)
{
	bool retval = true;
	uint32_t i;

	uint32_t value;
	bool bGotValidValue = false;
	const int LINE_BUFFER_SIZE = 10;
	char lineBuffer[LINE_BUFFER_SIZE + 1];
	char* deviceName;
	uint32_t bus;
	uint32_t device;
	uint32_t function;

	printf("\n");
	printf("Multiple devices found...please choose device:\n");
	for (i = 0; i < g_deviceManager.GetNumEnumeratedDevices(); i++)
	{
		deviceName = g_deviceManager.GetEnumeratedDeviceName(i);
		g_deviceManager.GetEnumeratedDeviceBDF(i, &bus, &device, &function);

		printf("[%u] %s (0000:%02x:%02x.%01x)\n", i + 1, deviceName, bus, device, function);
	}
	printf("[0] Virtual Device\n");
	printf("\n");


	while (bGotValidValue == false)
	{
		printf("Choose Device: ");

		fgets(lineBuffer, LINE_BUFFER_SIZE, stdin);

		value = (uint32_t)atoi(lineBuffer);

		if (value <= g_deviceManager.GetNumEnumeratedDevices())
		{
			bGotValidValue = true;
		}


		if (bGotValidValue == false)
		{
			printf("Invalid value...try again\n");
		}
	}




	if (value != 0)
	{
		*pSelectedDeviceIndex = value - 1; //-1 because we displayed i+1 to the user above.....
	}
	else
	{
		//user has chosen to cancel...
		retval = false;
	}

	return retval;
}









DeviceInterface* ChooseDeviceInterface(CommandLineOptions* pCommandLineOptions)
{
	bool bOKToContinue = true;
	uint32_t numDevices;
	uint32_t selectedDeviceIndex;
	DeviceInterface* pDeviceInterface;

	numDevices = g_deviceManager.EnumerateDevices();



	if (pCommandLineOptions->bDeviceIndexSupplied)
	{
		if (pCommandLineOptions->deviceIndex == 0) //special case - 0 means virtual device
		{
			pDeviceInterface = g_deviceManager.CreateVirtualDeviceInterface();
		}
		else
		{
			if (pCommandLineOptions->deviceIndex <= numDevices)
			{
				pDeviceInterface = g_deviceManager.CreateHWDeviceInterface(pCommandLineOptions->deviceIndex - 1); //minus 1 because device manager enumerates from zero...
			}
			else
			{
				printf("[ERROR] Specified -d device index = %u\n", pCommandLineOptions->deviceIndex);
				printf("        Found %u HW device(s)\n", numDevices);
				exit(0);
			}
		}
	}
	else //No device index supplied on command line...
	{
		//We need to determine if we should prompt the user to select a device...

		if (numDevices == 1)
		{
			//only one device present...select it automatically
			pDeviceInterface = g_deviceManager.CreateHWDeviceInterface(0);
		}
		else if (numDevices > 1)
		{
			//more than one device present...need to prompt user to select which one to use...
			bOKToContinue = PromptUserForDeviceSelection(&selectedDeviceIndex);
			if (bOKToContinue)
			{
				pDeviceInterface = g_deviceManager.CreateHWDeviceInterface(selectedDeviceIndex);
			}
			else
			{
				//user chose to cancel...just use virtual device instead...
				pDeviceInterface = g_deviceManager.CreateVirtualDeviceInterface();
			}
		}
		else
		{
			pDeviceInterface = g_deviceManager.CreateVirtualDeviceInterface();
		}
	}



#ifdef XCL_EMULATION_MODE
	printf("XCL_EMULATION_MODE = %s\n", XSTR(XCL_EMULATION_MODE));
#endif


	printf("Using device: %s...\n\n", pDeviceInterface->GetDeviceName());
	return pDeviceInterface;
}




bool ParseCommandLineOptions(int argc, char* argv[], CommandLineOptions* pCommandLineOptions)
{
	bool bValid = true;
	char* pToken;
	bool bTokenValid;
	uint32_t numTokensParsed;


	pCommandLineOptions->bDeviceIndexSupplied = false;
	pCommandLineOptions->bInlineCommandSupplied = false;
	pCommandLineOptions->bInteractiveMode = false;

	//start at 1 to skip the program name....
	for (uint32_t currentArgIndex = 1; currentArgIndex < (uint32_t)argc; /*no increment*/) 
	{
		if (strcmp(argv[currentArgIndex], "-d") == 0)
		{
			currentArgIndex++;

			if (currentArgIndex < (uint32_t)argc)
			{
				pToken = argv[currentArgIndex];
				currentArgIndex++;

				bTokenValid = true;

				//check to ensure the device index argument is a number...
				for (uint32_t j = 0; j < strlen(pToken); j++)
				{
					if (isdigit(pToken[j]) == 0)
					{
						printf("Invalid -d argument\n");
						bValid = false;
					}
				}

				if (bTokenValid)
				{
					numTokensParsed = sscanf(pToken, "%u", &(pCommandLineOptions->deviceIndex));
					if (numTokensParsed == 1)
					{
						pCommandLineOptions->bDeviceIndexSupplied = true;
					}
				}
			}
			else
			{
				printf("Missing -d argument");
				bValid = false;
			}
		}
		else if (strcmp(argv[currentArgIndex], "-i") == 0)
		{
			pCommandLineOptions->bInteractiveMode = true;
			currentArgIndex++;
		}
		else
		{
			pCommandLineOptions->bInlineCommandSupplied = true;
			pCommandLineOptions->inlineCommandStartIndex = currentArgIndex;
			break;
		}
	}


	return bValid;
}
















void sigHandler(int s)
{
	XLNX_UNUSED_ARG(s);

	//Do nothing
}





//The following function will be invoked automatically if the user chooses to download a bitstream (XCLBIN file)
//This allows the software objects to cleanup/release resources before the download occurs.
void PreDownloadHandler(void* pDataObject, DeviceInterface* pDeviceInterface)
{
	AAT* pAAT = (AAT*)pDataObject;

	XLNX_UNUSED_ARG(pDeviceInterface);

	pAAT->Uninitialise();
}





//The following function will be invoked automatically if the user chooses to download a bitstream (XCLBIN file)
//This allows the software objects to be automatically reinitialised to use the new HW load.
void PostDownloadHandler(void* pDataObject, DeviceInterface* pDeviceInterface)
{
	AAT* pAAT = (AAT*)pDataObject;

	pAAT->Initialise(pDeviceInterface);
}






int main(int argc, char* argv[])
{
	bool bUDPSynthesized;
	const char* egressCommsShellName;

	bool bOptionsValid;


	//
	//install a signal handlers to catch various termination signals...
	//
#ifdef __linux__
	signal(SIGINT, sigHandler);		//CTRL-C
	signal(SIGTSTP, sigHandler);	//CTRL-Z
#endif



	bOptionsValid = ParseCommandLineOptions(argc, argv, &commandLineOptions);
	if (bOptionsValid == false)
	{
		PrintUsage(argv[0]);
		exit(0);
	}



	//NOTE - need to perform device selection BEFORE we set up input/output streams.
	//       Once we set up streams, we disable character echoing (to put it under control of the shell).
	//       Since device selection happens BEFORE we enter the shell, this has the effect of the user
	//       not being able to see what they type when selecting the device.
	DeviceInterface* pDeviceInterface;
	pDeviceInterface = ChooseDeviceInterface(&commandLineOptions);





#ifdef __linux__
	OutStreamLinuxConsole outputStream;
	InStreamLinuxConsole inputStream(&outputStream);
#endif

#ifdef _WIN32
	OutStreamWindowsConsole outputStream;
	InStreamWindowsConsole inputStream(&outputStream);

#endif



	//Uncomment the following lines to prompt and wait for user to attach debugger...
	//printf("ATTACH DEBUGGER NOW...PRESS RETURN TO CONTINUE...\n");
	//getc(stdin);





    //Initialise our main business object...
	g_aat.Initialise(pDeviceInterface);


	// The following is to handle the fact the egress comms block can be either TCP or UDP (depending on how the HW is built)
	// We will name the shell object the accordingly
	// NOTE - we are ALWAYS calling the SOFTWARE OBJECT "g_aat.egressTCPUDPIP" in code
	g_aat.egressTCPUDPIP.IsUDPSynthesized(&bUDPSynthesized);

	if (bUDPSynthesized)
	{
		egressCommsShellName = "udpip2";
	}
	else
	{
		egressCommsShellName = "tcpip";
	}




    //Bind in the external object command tables into the shell instance...

	//							  Textual Name			Data Object							Command Table								Command Table Length
	//							  --------------------------------------------------------------------------------------------------------------------------------
    g_shell.AddObjectCommandTable("aat",				&g_aat,								XLNX_AAT_COMMAND_TABLE,						XLNX_AAT_COMMAND_TABLE_LENGTH);
	g_shell.AddObjectCommandTable("ethernet",			&g_aat.ethernet,					XLNX_ETHERNET_COMMAND_TABLE,				XLNX_ETHERNET_COMMAND_TABLE_LENGTH);
	
	//The following 2 UDP entries are for when we have line arbitration present...
	g_shell.AddObjectCommandTable("udpip0",				&g_aat.ingressTCPUDPIP0,			XLNX_TCP_UDP_IP_COMMAND_TABLE,				XLNX_TCP_UDP_IP_COMMAND_TABLE_LENGTH);
	g_shell.AddObjectCommandTable("udpip1",				&g_aat.ingressTCPUDPIP1,			XLNX_TCP_UDP_IP_COMMAND_TABLE,				XLNX_TCP_UDP_IP_COMMAND_TABLE_LENGTH);

	g_shell.AddObjectCommandTable("feedhandler",		&g_aat.feedHandler,					XLNX_FEED_HANDLER_COMMAND_TABLE,			XLNX_FEED_HANDLER_COMMAND_TABLE_LENGTH);
	g_shell.AddObjectCommandTable("orderbook",			&g_aat.orderBook,					XLNX_ORDER_BOOK_COMMAND_TABLE,				XLNX_ORDER_BOOK_COMMAND_TABLE_LENGTH);
	g_shell.AddObjectCommandTable("datamover",			&g_aat.dataMover,					XLNX_ORDER_BOOK_DATA_MOVER_COMMAND_TABLE,	XLNX_ORDER_BOOK_DATA_MOVER_COMMAND_TABLE_LENGTH);
	g_shell.AddObjectCommandTable("pricingengine",		&g_aat.pricingEngine,				XLNX_PRICING_ENGINE_COMMAND_TABLE,			XLNX_PRICING_ENGINE_COMMAND_TABLE_LENGTH);
    g_shell.AddObjectCommandTable("orderentry",			&g_aat.orderEntry,					XLNX_ORDER_ENTRY_COMMAND_TABLE,				XLNX_ORDER_ENTRY_COMMAND_TABLE_LENGTH);
	g_shell.AddObjectCommandTable(egressCommsShellName, &g_aat.egressTCPUDPIP,				XLNX_TCP_UDP_IP_COMMAND_TABLE,				XLNX_TCP_UDP_IP_COMMAND_TABLE_LENGTH);
	g_shell.AddObjectCommandTable("clocktickgen",		&g_aat.clockTickGenerator,			XLNX_CLOCK_TICK_GENERATOR_COMMAND_TABLE,	XLNX_CLOCK_TICK_GENERATOR_COMMAND_TABLE_LENGTH);
	g_shell.AddObjectCommandTable("linehandler",		&g_aat.lineHandler,					XLNX_LINE_HANDLER_COMMAND_TABLE,			XLNX_LINE_HANDLER_COMMAND_TABLE_LENGTH);
	
	g_shell.AddObjectCommandTable("networkcapture",		&g_aat.networkCapture,				XLNX_NETWORK_CAPTURE_COMMAND_TABLE,			XLNX_NETWORK_CAPTURE_COMMAND_TABLE_LENGTH);	
	g_shell.AddObjectCommandTable("networktap",			&g_aat.networkTap,					XLNX_NETWORK_TAP_COMMAND_TABLE,				XLNX_NETWORK_TAP_COMMAND_TABLE_LENGTH);


	//Bind in the pre and post download handler functions - these will be invoked if the user issues the "download" command to download a bitstream (XCLBIN file).
	//This allows the software objects to be automtically cleaned up/reinitialised to use the new HW load.
	g_shell.SetDownloadCallbacks(PreDownloadHandler, PostDownloadHandler, &g_aat);










	//
	// Run the shell
	//
	if (commandLineOptions.bInlineCommandSupplied)
	{
		g_shell.RunOneShot(inputStream, outputStream, pDeviceInterface, (argc - commandLineOptions.inlineCommandStartIndex), &argv[commandLineOptions.inlineCommandStartIndex]);


		//if the user has also specified the "-i" option, we want to run in interactive mode...
		if (commandLineOptions.bInteractiveMode)
		{
			g_shell.Run(inputStream, outputStream, pDeviceInterface); //NOTE - does not return until user types "exit" command
		}
	}
	else
	{
		//Run the shell in INTERACTIVE mode...
		g_shell.Run(inputStream, outputStream, pDeviceInterface); //NOTE - does not return until user types "exit" command
	}



	//Clean-up our business object..
	g_aat.Uninitialise();


	return 0;
}