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



#include "xlnx_shell_feed_handler.h"
#include "xlnx_shell_utils.h"

#include "xlnx_feed_handler.h"
#include "xlnx_feed_handler_error_codes.h"
using namespace XLNX;



static const char* LINE_STRING = "------------------------------------------------------------------";





#define STR_CASE(TAG)	case(TAG):					\
                        {							\
                            pString = (char*) #TAG;	\
                            break;					\
                        }





static char* FeedHandler_ErrorCodeToString(uint32_t errorCode)
{
    char* pString;

    switch (errorCode)
    {
        STR_CASE(XLNX_OK)
        STR_CASE(XLNX_FEED_HANDLER_ERROR_NOT_INITIALISED)
        STR_CASE(XLNX_FEED_HANDLER_ERROR_IO_FAILED)
        STR_CASE(XLNX_FEED_HANDLER_ERROR_CU_NAME_NOT_FOUND)
        STR_CASE(XLNX_FEED_HANDLER_ERROR_NO_FREE_SECURITY_INDEX)
        STR_CASE(XLNX_FEED_HANDLER_ERROR_SECURITY_ID_ALREADY_EXISTS)
        STR_CASE(XLNX_FEED_HANDLER_ERROR_SECURITY_ID_DOES_NOT_EXIST)
        STR_CASE(XLNX_FEED_HANDLER_ERROR_SECURITY_INDEX_OUT_OF_RANGE)
        STR_CASE(XLNX_FEED_HANDLER_ERROR_NO_SECURITY_AT_SPECIFIED_INDEX)
        STR_CASE(XLNX_FEED_HANDLER_ERROR_INVALID_SECURITY_ID)


        default:
        {
            pString = (char*)"UKNOWN_ERROR";
            break;
        }
    }

    return pString;
}













static int FeedHandler_AddSecurity(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    bool bOKToContinue = true;
    FeedHandler* pFeedHandler = (FeedHandler*)pObjectData;
    uint32_t securityID = 0;
    uint32_t index;

    if (argc != 2)
    {
        pShell->printf("Usage: %s <securityID>\n", argv[0]);
        bOKToContinue = false;
    }


    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[1], &securityID);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse security ID\n");
        }
    }



    if (bOKToContinue)
    {

        pShell->printf("Attempting to add security ID = %u (0x%08X)...\n", securityID, securityID);

        retval = pFeedHandler->AddSecurity(securityID, &index);

        if (retval == XLNX_OK)
        {
            pShell->printf("Security ID = %u (0x%08X) sucessfully added at index %u\n", securityID, securityID, index);
            pShell->printf("OK\n");
        }
        else
        {
            bOKToContinue = false;
            pShell->printf("[ERROR] Failed to add securityID, error = %s (0x%08X)\n", FeedHandler_ErrorCodeToString(retval), retval);
        }
    }



    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }


    return retval;
}










static int FeedHandler_AddSecurityAtIndex(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    bool bOKToContinue = true;
    FeedHandler* pFeedHandler = (FeedHandler*)pObjectData;
    uint32_t securityID = 0;
    uint32_t index = 0;

    if (argc != 3)
    {
        pShell->printf("Usage: %s <securityID> <index>\n", argv[0]);
        bOKToContinue = false;
    }


    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[1], &securityID);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse security ID\n");
        }
    }


    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[2], &index);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse index\n");
        }
    }



    if (bOKToContinue)
    {

        pShell->printf("Attempting to add security ID = %u (0x%08X) at index %u\n", securityID, securityID, index);

        retval = pFeedHandler->AddSecurityAtIndex(securityID, index);

        if (retval == XLNX_OK)
        {
            pShell->printf("Security ID = %u (0x%08X) sucessfully added at index %u\n", securityID, securityID, index);
            pShell->printf("OK\n");
        }
        else
        {
            bOKToContinue = false;
            pShell->printf("[ERROR] Failed to add securityID, error = %s (0x%08X)\n", FeedHandler_ErrorCodeToString(retval), retval);
        }
    }


    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }


    return retval;
}









static int FeedHandler_DeleteSecurity(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    bool bOKToContinue = true;
    FeedHandler* pFeedHandler = (FeedHandler*)pObjectData;
    uint32_t securityID;

    if (argc != 2)
    {
        pShell->printf("Usage: %s <securityID>\n", argv[0]);
        bOKToContinue = false;
    }

    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[1], &securityID);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse security ID\n");
        }
    }



    if (bOKToContinue)
    {
        pShell->printf("Attempting to remove security ID = %u (0x%08X)...\n", securityID, securityID);

        retval = pFeedHandler->RemoveSecurity(securityID);

        if (retval == XLNX_OK)
        {
            pShell->printf("Security ID = %u (0x%08X) successfully removed\n", securityID, securityID);
            pShell->printf("OK\n");
        }
        else
        {
            bOKToContinue = false;
            pShell->printf("[ERROR] Failed to remove security ID, error = %s (0x%08X)\n", FeedHandler_ErrorCodeToString(retval), retval);
        }
    }



    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }


    return retval;
}





static int FeedHandler_DeleteAllSecurities(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    FeedHandler* pFeedHandler = (FeedHandler*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    pShell->printf("Removing ALL securities...\n");


    retval = pFeedHandler->RemoveAllSecurities();

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        pShell->printf("[ERROR] Failed to remove symbols, error = %s (0x%08X)\n", FeedHandler_ErrorCodeToString(retval), retval);
    }

    return retval;
}










static int FeedHandler_PrintSecuritiesList(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    FeedHandler* pFeedHandler = (FeedHandler*)pObjectData;
    uint32_t rc;
    uint32_t i;
    uint32_t securityID;
    bool bFoundASymbol = false;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);



    pShell->printf("+-------+-------------------+-------------------+\n");
    pShell->printf("| Index | Security ID (dec) | Security ID (hex) |\n");
    pShell->printf("+-------+-------------------+-------------------+\n");

    for (i = 0; i < FeedHandler::MAX_NUM_SECURITIES; i++)
    {
        rc = pFeedHandler->GetSecurityIDAtIndex(i, &securityID);

        if (rc == XLNX_OK)
        {
            bFoundASymbol = true;
            pShell->printf("|  %3u  | %17u |        0x%08X |\n", i, securityID, securityID);
        }
    }

    if (bFoundASymbol == false)
    {
        pShell->printf("| %-45s |\n", "No Securities Found");
    }

    pShell->printf("+-------+-------------------+-------------------+\n");


    return retval;
}











static int FeedHandler_PrintStats(Shell* pShell, FeedHandler* pFeedHandler)
{
    int retval = 0;
    FeedHandler::Stats statsCounters;



    //Read the stats...
    retval = pFeedHandler->GetStats(&statsCounters);



    if (retval == XLNX_OK)
    {
        pShell->printf("\n");
        pShell->printf("+-%.26s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
        pShell->printf("| %-26s | %20s |\n", "Counter Name", "Value");
        pShell->printf("+-%.26s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
        pShell->printf("| %-26s | %20u |\n", "Processed Bytes",             statsCounters.numProcessedBytes);
        pShell->printf("| %-26s | %20u |\n", "Processed Packets",           statsCounters.numProcessedPackets);
        pShell->printf("| %-26s | %20u |\n", "Processed Binary Messages",   statsCounters.numProcessedBinaryMessages);
        pShell->printf("| %-26s | %20u |\n", "Processed FIX Messages",      statsCounters.numProcessedFIXMessages);
        pShell->printf("| %-26s | %20u |\n", "Tx Order Book Operations",    statsCounters.numTxOrderBookOperations);
        pShell->printf("+-%.26s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
        pShell->printf("| %-26s | %20u |\n", "Clock Tick Events",           statsCounters.numClockTickEvents);
        pShell->printf("+-%.26s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }

    return retval;
}







static int FeedHandler_GetStatus(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    FeedHandler* pFeedHandler = (FeedHandler*)pObjectData;
    bool bIsInitialised = false;
    uint64_t cuAddress;
    uint32_t cuIndex;
    bool bIsRunning;
 


    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    pFeedHandler->IsInitialised(&bIsInitialised);
    if (bIsInitialised == false)
    {
        retval = XLNX_FEED_HANDLER_ERROR_NOT_INITIALISED;
    }


    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.20s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }


    if (retval == XLNX_OK)
    {
        retval = pFeedHandler->GetCUAddress(&cuAddress);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-20s |   0x%016" PRIX64 " |\n", "CU Address", cuAddress);
        }
    }




    if (retval == XLNX_OK)
    {
        retval = pFeedHandler->GetCUIndex(&cuIndex);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-20s | %20u |\n", "CU Index", cuIndex);
        }
    }



    if (retval == XLNX_OK)
    {
        retval = pFeedHandler->IsRunning(&bIsRunning);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-20s | %20s |\n", "Is Running", pShell->boolToString(bIsRunning));
        }
    }







    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.20s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
        pShell->printf("\n");
    }




    if (retval == XLNX_OK)
    {
        retval = FeedHandler_PrintStats(pShell, pFeedHandler);
    }





    if (retval != XLNX_OK)
    {
        pShell->printf("[ERROR] retval = %s (0x%08X)\n", FeedHandler_ErrorCodeToString(retval), retval);
    }


    return retval;
}









static int FeedHandler_ResetStats(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    FeedHandler* pFeedHandler = (FeedHandler*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    retval = pFeedHandler->ResetStats();

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        pShell->printf("[ERROR] %s (0x%08X)\n", FeedHandler_ErrorCodeToString(retval), retval);
    }

    return retval;
}










static int FeedHandler_Start(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    FeedHandler* pFeedHandler = (FeedHandler*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    retval = pFeedHandler->Start();

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        pShell->printf("[ERROR] %s (0x%08X)\n", FeedHandler_ErrorCodeToString(retval), retval);
    }

    return retval;
}







static int FeedHandler_Stop(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    FeedHandler* pFeedHandler = (FeedHandler*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    retval = pFeedHandler->Stop();

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        pShell->printf("[ERROR] %s (0x%08X)\n", FeedHandler_ErrorCodeToString(retval), retval);
    }

    return retval;
}












static int FeedHandler_Refresh(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    FeedHandler* pFeedHandler = (FeedHandler*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    retval = pFeedHandler->RefreshCache();

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        pShell->printf("[ERROR] %s (0x%08X)\n", FeedHandler_ErrorCodeToString(retval), retval);
    }

    return retval;
}








static char* FeedHandler_OrderSideToString(FeedHandler::OrderSide orderSide)
{
    char* pString;

    switch (orderSide)
    {
        case(FeedHandler::OrderSide::ORDER_SIDE_BID):
        {
            pString = (char*)"BID";
            break;
        }

        case(FeedHandler::OrderSide::ORDER_SIDE_ASK):
        {
            pString = (char*)"ASK";
            break;
        }

        default:
        {
            pString = (char*)"UNKNOWN";
            break;
        }
    }

    return pString;
}










static char* FeedHandler_OrderOperationToString(FeedHandler::OrderOperation orderOperation)
{
    char* pString;

    switch (orderOperation)
    {
        case(FeedHandler::OrderOperation::ORDER_OPERATION_ADD):
        {
            pString = (char*)"ADD";
            break;
        }

        case(FeedHandler::OrderOperation::ORDER_OPERATION_MODIFY):
        {
            pString = (char*)"MODIFY";
            break;
        }

        case(FeedHandler::OrderOperation::ORDER_OPERATION_DELETE):
        {
            pString = (char*)"DELETE";
            break;
        }

        default:
        {
            pString = (char*)"UNKNOWN";
            break;
        }
    }

    return pString;
}




static int FeedHandler_ReadData(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    FeedHandler* pFeedHandler = (FeedHandler*)pObjectData;
    FeedHandler::FeedData data;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    retval = pFeedHandler->ReadData(&data);

    if (retval == XLNX_OK)
    {
        pShell->printf("+-------------------+----------------------+\n");
        pShell->printf("| Level             | %20u |\n", data.level);
        pShell->printf("| Side              | %20s |\n", FeedHandler_OrderSideToString(data.side));
        pShell->printf("| Price             | %20.2f |\n", (double)(data.price / 100.0));
        pShell->printf("| Quantity          | %20u |\n", data.quantity);
        pShell->printf("| Count             | %20u |\n", data.count);
        pShell->printf("| Order ID          | %20u |\n", data.orderID);
        pShell->printf("| Symbol Index      | %20u |\n", data.symbolIndex);
        pShell->printf("| Operation         | %20s |\n", FeedHandler_OrderOperationToString(data.operation));
        pShell->printf("| Timestamp         | %20" PRIu64 " |\n", data.timestamp);
        pShell->printf("+-------------------+----------------------+\n");
    }
    else
    {
        pShell->printf("[ERROR] %s (0x%08X)\n", FeedHandler_ErrorCodeToString(retval), retval);
    }

    return retval;
}








CommandTableElement XLNX_FEED_HANDLER_COMMAND_TABLE[] =
{
    {"add",			        FeedHandler_AddSecurity,			"<securityID>",	                "Add a security at first available index"   },
    {"addat",               FeedHandler_AddSecurityAtIndex,     "<securityID> <index>",         "Add a security at specific index"          },
    {"delete",		        FeedHandler_DeleteSecurity,		    "<securityID>",			        "Delete a security"	                        },
    {"deleteall",           FeedHandler_DeleteAllSecurities,    "",                             "Deletes ALL securities"                    },
    {/*-----------------------------------------------------------------------------------------------------------------------------------*/},
    {"start",               FeedHandler_Start,                  "",                             "Starts the block running again"            },
    {"stop",                FeedHandler_Stop,                   "",                             "Halts processing"                          },
    {/*-----------------------------------------------------------------------------------------------------------------------------------*/},
    {"getstatus",	        FeedHandler_GetStatus,			    "",					            "Get block status"			                },
    {"list",                FeedHandler_PrintSecuritiesList,    "",                             "Prints list of securities"                 },
    {"refresh",             FeedHandler_Refresh,                "",                             "Refreshes the internal security ID cache"  },
    {"resetstats",          FeedHandler_ResetStats,             "",                             "Reset stat counters"                       },
    {/*-----------------------------------------------------------------------------------------------------------------------------------*/},
    {"readdata",            FeedHandler_ReadData,               "",                             "Read last data captured"                   }       


};


const uint32_t XLNX_FEED_HANDLER_COMMAND_TABLE_LENGTH = (uint32_t)(sizeof(XLNX_FEED_HANDLER_COMMAND_TABLE) / sizeof(XLNX_FEED_HANDLER_COMMAND_TABLE[0]));
