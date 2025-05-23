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


#include "xlnx_shell_order_entry.h"
#include "xlnx_shell_utils.h"

#include "xlnx_order_entry.h"
#include "xlnx_order_entry_error_codes.h"
using namespace XLNX;




static const char* LINE_STRING = "---------------------------------------------------------------------";





#define STR_CASE(TAG)	case(TAG):					\
						{							\
							pString = (char*) #TAG;	\
							break;					\
						}





static char* OrderEntry_ErrorCodeToString(uint32_t errorCode)
{
    char* pString;

    switch (errorCode)
    {
        STR_CASE(XLNX_OK)
        STR_CASE(XLNX_ORDER_ENTRY_ERROR_NOT_INITIALISED)
        STR_CASE(XLNX_ORDER_ENTRY_ERROR_IO_FAILED)
        STR_CASE(XLNX_ORDER_ENTRY_ERROR_INVALID_PARAMETER)
        STR_CASE(XLNX_ORDER_ENTRY_ERROR_CU_NAME_NOT_FOUND)
        STR_CASE(XLNX_ORDER_ENTRY_ERROR_CU_INDEX_NOT_FOUND)
        STR_CASE(XLNX_ORDER_ENTRY_ERROR_SYMBOL_INDEX_OUT_OF_RANGE)
        STR_CASE(XLNX_ORDER_ENTRY_ERROR_CONNECTION_ALREADY_REQUESTED)
        STR_CASE(XLNX_ORDER_ENTRY_ERROR_NOT_CONNECTED)


        default:
        {
            pString = (char*)"UKNOWN_ERROR";
            break;
        }
    }

    return pString;
}





static char* OrderEntry_ConnectionErrorCodeToString(OrderEntry::ConnectionErrorCode errorCode)
{
    const char* pString;

    switch (errorCode)
    {
        
        case(OrderEntry::ConnectionErrorCode::SUCCESS):
        {
            pString = "SUCCESS";
            break;
        }

        case(OrderEntry::ConnectionErrorCode::CLOSED):
        {
            pString = "CLOSED";
            break;
        }

        case(OrderEntry::ConnectionErrorCode::OUT_OF_SPACE):
        {
            pString = "OUT_OF_SPACE";
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






static int OrderEntry_PrintStats(Shell* pShell, OrderEntry* pOrderEntry)
{
    int retval = 0;
    OrderEntry::Stats statsCounters;

    retval = pOrderEntry->GetStats(&statsCounters);

    if (retval == XLNX_OK)
    {
        pShell->printf("\n");
        pShell->printf("+-%.26s-+-%.10s-+\n", LINE_STRING, LINE_STRING);
        pShell->printf("| %-26s | %-10s |\n", "Counter Name", "Value");
        pShell->printf("+-%.26s-+-%.10s-+\n", LINE_STRING, LINE_STRING);
        pShell->printf("| %-26s | %10u |\n", "Rx Operations",           statsCounters.numRxOperations);
        pShell->printf("| %-26s | %10u |\n", "Processed Operations",    statsCounters.numProcessedOperations);
        pShell->printf("| %-26s | %10u |\n", "Tx Data Frames",          statsCounters.numTxDataFrames);
        pShell->printf("| %-26s | %10u |\n", "Tx Meta Frames",          statsCounters.numTxMetaFrames);
        pShell->printf("| %-26s | %10u |\n", "Tx Messages",             statsCounters.numTxMessages);
        pShell->printf("| %-26s | %10u |\n", "Tx Dropped Messages",     statsCounters.numTxDroppedMessages);
        pShell->printf("| %-26s | %10u |\n", "Rx Data Frames",          statsCounters.numRxDataFrames);
        pShell->printf("| %-26s | %10u |\n", "Rx Meta Frames",          statsCounters.numRxMetaFrames);
        pShell->printf("| %-26s | %10u |\n", "Notifications Received",  statsCounters.numNotificationsReceived);
        pShell->printf("| %-26s | %10u |\n", "Read Requests Sent",      statsCounters.numReadRequestsSent);
        pShell->printf("+-%.26s-+-%.10s-+\n", LINE_STRING, LINE_STRING);
        pShell->printf("| %-26s | %10u |\n", "Clock Tick Events",       statsCounters.numClockTickEvents);
        pShell->printf("+-%.26s-+-%.10s-+\n", LINE_STRING, LINE_STRING);
    }

    return retval;
}










static int OrderEntry_GetStatus(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    OrderEntry* pOrderEntry = (OrderEntry*)pObjectData;
    bool bIsInitialised = false;
    uint32_t cuIndex = 0;
    uint64_t cuAddress;
    bool bIsRunning;
    uint32_t captureSymbolIndex;
    bool bConnectionRequested;
    uint8_t ipAddrA, ipAddrB, ipAddrC, ipAddrD;
    uint16_t port;
    static const uint32_t BUFFER_SIZE = 64;
    char formatBuffer[BUFFER_SIZE + 1];
    OrderEntry::TxStatus txStatus;
    bool bEnabled;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    pOrderEntry->IsInitialised(&bIsInitialised);
    if (bIsInitialised == false)
    {
        retval = XLNX_ORDER_ENTRY_ERROR_NOT_INITIALISED;
    }




    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.28s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }


    if (retval == XLNX_OK)
    {
        retval = pOrderEntry->GetCUIndex(&cuIndex);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-28s | %20u |\n", "CU Index", cuIndex);
        }
    }




    if (retval == XLNX_OK)
    {
        retval = pOrderEntry->GetCUAddress(&cuAddress);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-28s |   0x%016" PRIX64 " |\n", "CU Address", cuAddress);
        }
    }


    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.28s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }



    if (retval == XLNX_OK)
    {
        retval = pOrderEntry->IsRunning(&bIsRunning);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-28s | %20s |\n", "Is Running", pShell->boolToString(bIsRunning));
        }

    }






    if (retval == XLNX_OK)
    {
        retval = pOrderEntry->GetCaptureFilter(&captureSymbolIndex);

        if (retval == XLNX_OK)
        {

            pShell->printf("| %-28s | %20u |\n", "Capture Filter Symbol Index", captureSymbolIndex);
        }
    }



    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.28s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }



    if (retval == XLNX_OK)
    {
        retval = pOrderEntry->IsConnectionRequested(&bConnectionRequested);


        if (retval == XLNX_OK)
        {
            if (bConnectionRequested)
            {
                retval = pOrderEntry->GetConnectionDetails(&ipAddrA, &ipAddrB, &ipAddrC, &ipAddrD, &port);
            }
        }


        if (retval == XLNX_OK)
        {
            pShell->printf("| %-28s | %20s |\n", "Connection Requested", pShell->boolToString(bConnectionRequested));


            if (bConnectionRequested)
            {
                snprintf(formatBuffer, BUFFER_SIZE, "%u.%u.%u.%u", ipAddrA, ipAddrB, ipAddrC, ipAddrD);
                pShell->printf("| %-28s | %20s |\n", "Target IP Address", formatBuffer);
                pShell->printf("| %-28s | %20u |\n", "Target Port", port);
            }
            else
            {
                pShell->printf("| %-28s | %20s |\n", "Target IP Address", "N/A");
                pShell->printf("| %-28s | %20s |\n", "Target Port", "N/A");
            }
        }
    }



    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.28s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }

    if (retval == XLNX_OK)
    {
        retval = pOrderEntry->GetTxStatus(&txStatus);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-28s | %20s |\n", "Connection Established", pShell->boolToString(txStatus.bConnectionEstablished));
            pShell->printf("| %-28s | %20s |\n", "Connection Status",      OrderEntry_ConnectionErrorCodeToString(txStatus.errorCode));
            pShell->printf("| %-28s | %20u |\n", "Send Buffer Space",      txStatus.sendBufferSpace);
        }
    }

    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.28s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }


    if (retval == XLNX_OK)
    {
        retval = pOrderEntry->GetChecksumGeneration(&bEnabled);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-28s | %20s |\n", "Partial Checksum Generation", pShell->boolToString(bEnabled));
        }
    }


    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.28s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }

    if (retval == XLNX_OK)
    {
        pShell->printf("\n\n");
    }


    if (retval == XLNX_OK)
    {
        retval = OrderEntry_PrintStats(pShell, pOrderEntry);
    }





    if (retval != XLNX_OK)
    {
        pShell->printf("[ERROR] retval = %s (0x%08X)\n", OrderEntry_ErrorCodeToString(retval), retval);
    }


    return retval;
}







static int OrderEntry_ReadLastMessage(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    OrderEntry* pOrderEntry = (OrderEntry*)pObjectData;
    OrderEntry::OrderEntryMessage message;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    retval = pOrderEntry->ReadMessage(&message);

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
        pShell->printf("%s\n", message.message);
    }




    if (retval != XLNX_OK)
    {
        pShell->printf("[ERROR] retval = %s (0x%08X)\n", OrderEntry_ErrorCodeToString(retval), retval);
    }

    return retval;
}









static int OrderEntry_ResetStats(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    OrderEntry* pOrderEntry = (OrderEntry*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    retval = pOrderEntry->ResetStats();

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        pShell->printf("[ERROR] %s (0x%08X)\n", OrderEntry_ErrorCodeToString(retval), retval);
    }

    return retval;
}







static int OrderEntry_Connect(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    OrderEntry* pOrderEntry = (OrderEntry*)pObjectData;
    bool bOKToContinue = true;
    uint8_t ipAddrA, ipAddrB, ipAddrC, ipAddrD;
    uint16_t port;

    if (argc != 3)
    {
        pShell->printf("Usage: %s <ipaddr> <port>\n", argv[0]);
        bOKToContinue = false;
    }

    if (bOKToContinue)
    {
        bOKToContinue = ParseIPv4Address(pShell, argv[1], &ipAddrA, &ipAddrB, &ipAddrC, &ipAddrD);
    }

    if (bOKToContinue)
    {
        bOKToContinue = ParsePort(pShell, argv[2], &port);
    }

    if (bOKToContinue)
    {
        retval = pOrderEntry->Connect(ipAddrA, ipAddrB, ipAddrC, ipAddrD, port);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] %s (0x%08X)\n", OrderEntry_ErrorCodeToString(retval), retval);
        }
    }

    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }

    return retval;
}




static int OrderEntry_Disconnect(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    OrderEntry* pOrderEntry = (OrderEntry*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);


    retval = pOrderEntry->Disconnect();

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        pShell->printf("[ERROR] %s (0x%08X)\n", OrderEntry_ErrorCodeToString(retval), retval);
    }



    return retval;
}



static int OrderEntry_Reconnect(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    OrderEntry* pOrderEntry = (OrderEntry*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);


    retval = pOrderEntry->Reconnect();

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        pShell->printf("[ERROR] %s (0x%08X)\n", OrderEntry_ErrorCodeToString(retval), retval);
    }



    return retval;
}






static int OrderEntry_SetChecksumGeneration(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    OrderEntry* pOrderEntry = (OrderEntry*)pObjectData;
    bool bOKToContinue = true;
    bool bEnabled = false;

    if (argc != 2)
    {
        pShell->printf("Usage: %s <bool>\n", argv[0]);
        bOKToContinue = false;
    }


    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseBool(argv[1], &bEnabled);
    }

    if (bOKToContinue)
    {
        retval = pOrderEntry->SetChecksumGeneration(bEnabled);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] %s (0x%08X)\n", OrderEntry_ErrorCodeToString(retval), retval);
        }
    }


    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }


    return retval;
}




CommandTableElement XLNX_ORDER_ENTRY_COMMAND_TABLE[] =
{
    {"getstatus",       OrderEntry_GetStatus,               "",                         "Get block status"                              },
    {"readmsg",	        OrderEntry_ReadLastMessage,	        "",			                "Read the last message to be emitted"           },
    {"resetstats",      OrderEntry_ResetStats,              "",                         "Reset stats counters"                          },
    {"connect",         OrderEntry_Connect,                 "<ipaddr> <port>",          "Establish connection to remote system"         },
    {"disconnect",      OrderEntry_Disconnect,              "",                         "Close connection to remote system"             },
    {"reconnect",       OrderEntry_Reconnect,               "",                         "Close and re-open existing connection"         },
    {"setcsumgen",      OrderEntry_SetChecksumGeneration,   "<bool>",                   "Control partial checksum generation"           },    

};


const uint32_t XLNX_ORDER_ENTRY_COMMAND_TABLE_LENGTH = (uint32_t)(sizeof(XLNX_ORDER_ENTRY_COMMAND_TABLE) / sizeof(XLNX_ORDER_ENTRY_COMMAND_TABLE[0]));
