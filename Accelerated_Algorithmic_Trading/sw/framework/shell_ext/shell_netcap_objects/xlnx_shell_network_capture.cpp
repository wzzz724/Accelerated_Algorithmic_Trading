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

#include <chrono>
#include <thread>
using namespace std;

#include "xlnx_shell_network_capture.h"
#include "xlnx_shell_utils.h"

#include "xlnx_network_capture.h"
#include "xlnx_network_capture_error_codes.h"
using namespace XLNX;










#define STR_CASE(TAG)	case(TAG):					\
                        {							\
                            pString = (char*) #TAG;	\
                            break;					\
                        }


static const char* LINE_STRING = "--------------------------------------------------------------------------------------------------";


char* NetworkCapture_ErrorCodeToString(uint32_t errorCode)
{
    char* pString;

    switch (errorCode)
    {
        STR_CASE(XLNX_OK)
        STR_CASE(XLNX_NETWORK_CAPTURE_ERROR_NOT_INITIALISED)
        STR_CASE(XLNX_NETWORK_CAPTURE_ERROR_IO_FAILED)
        STR_CASE(XLNX_NETWORK_CAPTURE_ERROR_INVALID_PARAMETER)
        STR_CASE(XLNX_NETWORK_CAPTURE_ERROR_CU_NAME_NOT_FOUND)
        STR_CASE(XLNX_NETWORK_CAPTURE_ERROR_CU_INDEX_NOT_FOUND)
        STR_CASE(XLNX_NETWORK_CAPTURE_ERROR_FAILED_TO_ALLOCATE_BUFFER_OBJECT)
        STR_CASE(XLNX_NETWORK_CAPTURE_ERROR_FAILED_TO_MAP_BUFFER_OBJECT)
        STR_CASE(XLNX_NETWORK_CAPTURE_ERROR_FAILED_TO_SYNC_BUFFER_OBJECT)
        STR_CASE(XLNX_NETWORK_CAPTURE_ERROR_FAILED_TO_OPEN_PCAP_FILE_FOR_WRITING)
        STR_CASE(XLNX_NETWORK_CAPTURE_ERROR_PCAP_FILE_NOT_OPEN)
        STR_CASE(XLNX_NETWORK_CAPTURE_ERROR_ALREADY_RUNNING)
        STR_CASE(XLNX_NETWORK_CAPTURE_ERROR_NOT_RUNNING)

        default:
        {
            pString = (char*)"UKNOWN_ERROR";
            break;
        }
    }

    return pString;
}






static int NetworkCapture_GetStatus(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    NetworkCapture* pNetworkCapture = (NetworkCapture*)pObjectData;
    bool bIsInitialised = false;
    uint32_t cuIndex = 0;
    uint64_t cuAddress;
    uint32_t cuMemTopologyIndex;
    bool bIsRunning;
    uint32_t tailPointer;
    uint32_t numPackets;
    uint32_t pollRateMilliseconds;
    bool bYieldEnabled;
    uint32_t hwEmuPollDelaySeconds;
   
    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);


    pNetworkCapture->IsInitialised(&bIsInitialised);

    if (bIsInitialised == false)
    {
        retval = XLNX_NETWORK_CAPTURE_ERROR_NOT_INITIALISED;
    }



    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }








    if (retval == XLNX_OK)
    {
        retval = pNetworkCapture->GetCUIndex(&cuIndex);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20u |\n", "CU Index", cuIndex);
        }
    }




    if (retval == XLNX_OK)
    {
        retval = pNetworkCapture->GetCUAddress(&cuAddress);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s |   0x%016" PRIX64 " |\n", "CU Address", cuAddress);
        }
    }



    if (retval == XLNX_OK)
    {
        retval = pNetworkCapture->GetBufferCUMemTopologyIndex(&cuMemTopologyIndex);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20u |\n", "Buffer CU Mem Topology Index", cuMemTopologyIndex);
        }
    }







    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }






    if (retval == XLNX_OK)
    {
        void* pHostBuffer = pNetworkCapture->GetBufferHostVirtualAddress();

        if (pHostBuffer != nullptr)
        {
            pShell->printf("| %-35s |   0x%016" PRIXPTR " |\n", "Buffer Host Virtual Address", (uintptr_t)pHostBuffer);
        }
        else
        {
            pShell->printf("| %-35s | %20s |\n", "Buffer Host Virtual Address", "Deferred Until Sync");
        }
    }






    if (retval == XLNX_OK)
    {

        uint64_t hwBufferAddress = pNetworkCapture->GetBufferHWAddress();

        if (hwBufferAddress != NetworkCapture::INVALID_HW_BUFFER_ADDRESS)
        {
            pShell->printf("| %-35s |   0x%016" PRIX64 " |\n", "Buffer HW Address", hwBufferAddress);
        }
        else
        {
            pShell->printf("| %-35s | %20s |\n", "Buffer HW Address", "Deferred Until Sync");
        }
    }


    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }





    if (retval == XLNX_OK)
    {
        retval = pNetworkCapture->IsHWKernelRunning(&bIsRunning);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20s |\n", "HW Kernel Is Running", pShell->boolToString(bIsRunning));
        }
    }




    if (retval == XLNX_OK)
    {
        retval = pNetworkCapture->GetTailPointer(&tailPointer);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s |           0x%08X |\n", "Tail Pointer", tailPointer);
        }
    }



    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }



    if (retval == XLNX_OK)
    {
        retval = pNetworkCapture->IsRunning(&bIsRunning);
        
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20s |\n", "SW Thread Is Running", pShell->boolToString(bIsRunning));
        }
    }


    if (retval == XLNX_OK)
    {
        retval = pNetworkCapture->GetPollRate(&pollRateMilliseconds);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20u |\n", "Poll Rate (msecs)", pollRateMilliseconds);
        }
    }


    if (retval == XLNX_OK)
    {
        pNetworkCapture->GetHWEmulationPollDelay(&hwEmuPollDelaySeconds);

        pShell->printf("| %-35s | %20u |\n", "HW Emu Poll Delay (secs)", hwEmuPollDelaySeconds);
    }


    if (retval == XLNX_OK)
    {
        retval = pNetworkCapture->GetThreadYield(&bYieldEnabled);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20s |\n", "Thread Yielding Enabled", pShell->boolToString(bYieldEnabled));
        }
    }



    if (retval == XLNX_OK)
    {
        retval = pNetworkCapture->GetNumCapturedPackets(&numPackets);
        
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20u |\n", "Num Captured Packets", numPackets);
        }
    }


    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }


    if (retval != XLNX_OK)
    {
        pShell->printf("[ERROR] retval = %s (0x%08X)\n", NetworkCapture_ErrorCodeToString(retval), retval);
    }


    return retval;
}







static int NetworkCapture_Start(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    NetworkCapture* pNetworkCapture = (NetworkCapture*)pObjectData;
    bool bOKToContinue = true;
    char* filepath = nullptr;

    if (argc != 2)
    {
        pShell->printf("Usage: %s <filepath>\n", argv[0]);
        bOKToContinue = false;
    }


    if (bOKToContinue)
    {
        filepath = argv[1];

        retval = pNetworkCapture->Start(filepath);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] retval = %s (0x%08X)\n", NetworkCapture_ErrorCodeToString(retval), retval);
        }
    }

    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }

    return retval;
}











static int NetworkCapture_Stop(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    NetworkCapture* pNetworkCapture = (NetworkCapture*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    retval = pNetworkCapture->Stop();

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        pShell->printf("[ERROR] retval = %s (0x%08X)\n", NetworkCapture_ErrorCodeToString(retval), retval);
    }

    return retval;
}





static int NetworkCapture_SetPollRate(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    NetworkCapture* pNetworkCapture = (NetworkCapture*)pObjectData;
    bool bOKToContinue = true;
    uint32_t milliseconds;

    if (argc != 2)
    {
        pShell->printf("Usage: %s <milliseconds>\n", argv[0]);
        bOKToContinue = false;
    }


    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[1], &milliseconds);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse milliseconds parameter\n");
        }
    }


    if (bOKToContinue)
    {
        retval = pNetworkCapture->SetPollRate(milliseconds);
        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] retval = %s (0x%08X)\n", NetworkCapture_ErrorCodeToString(retval), retval);
        }

    }


    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }
  
    return retval;
}





static int NetworkCapture_SetThreadYield(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    NetworkCapture* pNetworkCapture = (NetworkCapture*)pObjectData;
    bool bOKToContinue = true;
    bool bYieldEnabled;

    if (argc != 2)
    {
        pShell->printf("Usage: %s <bool>\n", argv[0]);
        bOKToContinue = false;
    }

    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseBool(argv[1], &bYieldEnabled);
        if(bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse bool parameter\n");
        }
    }


    if (bOKToContinue)
    {
        retval = pNetworkCapture->SetThreadYield(bYieldEnabled);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] retval = %s (0x%08X)\n", NetworkCapture_ErrorCodeToString(retval), retval);
        }
    }

    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }

    return retval;
}




static int NetworkCapture_SetHWEmuPollDelay(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    NetworkCapture* pNetworkCapture = (NetworkCapture*)pObjectData;
    bool bOKToContinue = true;
    uint32_t delaySeconds;

    if (argc != 2)
    {
        pShell->printf("Usage: %s <seconds>\n", argv[0]);
        bOKToContinue = false;
    }


    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[1], &delaySeconds);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse seconds parameter\n");
        }
    }

    if (bOKToContinue)
    {
        pNetworkCapture->SetHWEmulationPollDelay(delaySeconds);
        pShell->printf("OK\n");
    }


    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }

    return retval;
}







CommandTableElement XLNX_NETWORK_CAPTURE_COMMAND_TABLE[] =
{
    {"getstatus",	        NetworkCapture_GetStatus,	        "",			                "Get block status"	                            },
    {"start",               NetworkCapture_Start,               "<filepath>",               "Start capturing packets to specified file"     },
    {"stop",                NetworkCapture_Stop,                "",                         "Stop capturing packets"                        },
    {/*-----------------------------------------------------------------------------------------------------------------------------------*/},
    {"setpollrate",         NetworkCapture_SetPollRate,         "<milliseconds>",           "Sets the rate the capture thread polls HW"     },
    {"setyield",            NetworkCapture_SetThreadYield,      "<bool>",                   "Controls capture thread yielding to others"    },
    {/*-----------------------------------------------------------------------------------------------------------------------------------*/},
    {"sethwemupolldelay",   NetworkCapture_SetHWEmuPollDelay,   "<seconds>",                "Sets a poll delay - only used in HW emulation" }
   
};


const uint32_t XLNX_NETWORK_CAPTURE_COMMAND_TABLE_LENGTH = (uint32_t)(sizeof(XLNX_NETWORK_CAPTURE_COMMAND_TABLE) / sizeof(XLNX_NETWORK_CAPTURE_COMMAND_TABLE[0]));