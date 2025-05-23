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


#include "xlnx_shell_aat_objects.h"
#include "xlnx_shell_utils.h"


#include "xlnx_aat.h"
#include "xlnx_aat_error_codes.h"

#include "xlnx_shell_network_capture.h"
#include "xlnx_shell_network_tap.h"
using namespace XLNX;


#define STR_CASE(TAG)	case(TAG):					\
                        {							\
                            pString = (char*) #TAG;	\
                            break;					\
                        }


static const char* LINE_STRING = "----------------------------------------------------------------------------------";






char* AAT_ErrorCodeToString(AAT* pAAT, uint32_t netCapErrorCode, uint32_t* pSubBlockError)
{
    char* pString;


    switch (netCapErrorCode)
    {
        STR_CASE(XLNX_OK)


        case(XLNX_AAT_ERROR_NETWORK_CAPTURE_ERROR):
        {
            pAAT->GetSubBlockError(pSubBlockError);
            pString = NetworkCapture_ErrorCodeToString(*pSubBlockError);
            break;
        }
   

        case(XLNX_AAT_ERROR_NETWORK_TAP_ERROR):
        {
            pAAT->GetSubBlockError(pSubBlockError);
            pString = NetworkTap_ErrorCodeToString(*pSubBlockError);
            break;
        }


        default:
        {
            pString = (char*)"UKNOWN_ERROR";
            break;
        }
    }

    return pString;
}





static void AAT_PrintInitialisedTableRow(Shell* pShell, char* objectName, bool bIsInitialised)
{
    if (bIsInitialised)
    {
        pShell->printf("| %-18s | %-16s |\n", objectName, "Yes");
    }
    else
    {
        pShell->printf("| %-18s | %-16s |\n", objectName, "No");
    }

}




static int AAT_GetStatusCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    AAT* pAAT = (AAT*)pObjectData;
    bool bIsInitialised;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    pShell->printf("+-%.18s-+-%.16s-+\n", LINE_STRING, LINE_STRING);
    pShell->printf("| %-18s | %-16s |\n", "Object", "Is Initialised");
    pShell->printf("+-%.18s-+-%.16s-+\n", LINE_STRING, LINE_STRING);

    pAAT->ethernet.IsInitialised(&bIsInitialised);
    AAT_PrintInitialisedTableRow(pShell, (char*)"Ethernet", bIsInitialised);

    pAAT->ingressTCPUDPIP0.IsInitialised(&bIsInitialised);
    AAT_PrintInitialisedTableRow(pShell, (char*)"Ingress TCPUDPIP 0", bIsInitialised);

    pAAT->ingressTCPUDPIP1.IsInitialised(&bIsInitialised);
    AAT_PrintInitialisedTableRow(pShell, (char*)"Ingress TCPUDPIP 1", bIsInitialised);
        
    pAAT->feedHandler.IsInitialised(&bIsInitialised);
    AAT_PrintInitialisedTableRow(pShell, (char*)"Feed Handler", bIsInitialised);

    pAAT->orderBook.IsInitialised(&bIsInitialised);
    AAT_PrintInitialisedTableRow(pShell, (char*)"Order Book", bIsInitialised);

    pAAT->dataMover.IsInitialised(&bIsInitialised);
    AAT_PrintInitialisedTableRow(pShell, (char*)"Data Mover", bIsInitialised);

    pAAT->pricingEngine.IsInitialised(&bIsInitialised);
    AAT_PrintInitialisedTableRow(pShell, (char*)"Pricing Engine", bIsInitialised);

    pAAT->orderEntry.IsInitialised(&bIsInitialised);
    AAT_PrintInitialisedTableRow(pShell, (char*)"Order Entry", bIsInitialised);

    pAAT->egressTCPUDPIP.IsInitialised(&bIsInitialised);
    AAT_PrintInitialisedTableRow(pShell, (char*)"Egress TCPUDPIP", bIsInitialised);

    pAAT->clockTickGenerator.IsInitialised(&bIsInitialised);
    AAT_PrintInitialisedTableRow(pShell, (char*)"Clock Tick Gen", bIsInitialised);

    pAAT->lineHandler.IsInitialised(&bIsInitialised);
    AAT_PrintInitialisedTableRow(pShell, (char*)"Line Handler", bIsInitialised);



    pShell->printf("+-%.18s-+-%.16s-+\n", LINE_STRING, LINE_STRING);

    pAAT->networkCapture.IsInitialised(&bIsInitialised);
    AAT_PrintInitialisedTableRow(pShell, (char*)"Network Capture", bIsInitialised);

    pAAT->networkTap.IsInitialised(&bIsInitialised);
    AAT_PrintInitialisedTableRow(pShell, (char*)"Network Tap", bIsInitialised);


    pShell->printf("+-%.18s-+-%.16s-+\n", LINE_STRING, LINE_STRING);


    return retval;
}















static int AAT_ReinitCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    AAT* pAAT = (AAT*)pObjectData;
    DeviceInterface* pDeviceInterface = pShell->GetDeviceInterface();

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    retval = pAAT->Initialise(pDeviceInterface);

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        pShell->printf("[ERROR] Failed to reinitialise - error = 0x%08X\n", retval);
    }


    return retval;
}







static int AAT_StartDataMoverCommand(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    AAT* pAAT = (AAT*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    retval = pAAT->StartDataMover();

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        pShell->printf("[ERROR] Failed start datamover HW kernel, retval = 0x%08X\n", retval);
    }

    return retval;
}






static int AAT_StartCapture(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    bool bOKToContinue = true;
    AAT* pAAT = (AAT*)pObjectData;
    char* filepath;
    char* subBlockErrorString;
    uint32_t subBlockErrorCode;

    if (argc != 2)
    {
        pShell->printf("Usage: %s <filepath>\n", argv[0]);
        bOKToContinue = false;
    }


    if (bOKToContinue)
    {
        filepath = argv[1];

        retval = pAAT->StartCapture(filepath);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            subBlockErrorString = AAT_ErrorCodeToString(pAAT, retval, &subBlockErrorCode);
            pShell->printf("[ERROR] retval = %s (0x%08X)\n", subBlockErrorString, subBlockErrorCode);
        }
    }

    return retval;
}







static int AAT_StopCapture(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    AAT* pAAT = (AAT*)pObjectData;
    char* subBlockErrorString;
    uint32_t subBlockErrorCode;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    retval = pAAT->StopCapture();

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        subBlockErrorString = AAT_ErrorCodeToString(pAAT, retval, &subBlockErrorCode);
        pShell->printf("[ERROR] retval = %s (0x%08X)\n", subBlockErrorString, subBlockErrorCode);
    }

    return retval;
}











CommandTableElement XLNX_AAT_COMMAND_TABLE[] =
{
    {"getstatus",       AAT_GetStatusCommand,           "",                 "Get the status of the individual objects"      },
    {"reinit",		    AAT_ReinitCommand,    	        "",                 "Reinitialise all AAT objects"                  },
    {/*-------------------------------------------------------------------------------------------------------------------*/},  
    {"startdatamover",  AAT_StartDataMoverCommand,      "",                 "Start datamover kernel and enable OB output"   },
    {/*-------------------------------------------------------------------------------------------------------------------*/},
    {"startcapture",    AAT_StartCapture,               "<filepath>",       "Starts capturing packets to specified file"    },
    {"stopcapture",     AAT_StopCapture,                "",                 "Stop capturing packets"                        },
};


const uint32_t XLNX_AAT_COMMAND_TABLE_LENGTH = (uint32_t)(sizeof(XLNX_AAT_COMMAND_TABLE) / sizeof(XLNX_AAT_COMMAND_TABLE[0]));
