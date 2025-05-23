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



#include "xlnx_shell_order_book.h"
#include "xlnx_shell_utils.h"

#include "xlnx_order_book.h"
#include "xlnx_order_book_error_codes.h"
using namespace XLNX;






#define STR_CASE(TAG)	case(TAG):					\
						{							\
							pString = (char*) #TAG;	\
							break;					\
						}


static const char* LINE_STRING = "--------------------------------------------------------------------------------------------------";


static char* OrderBook_ErrorCodeToString(uint32_t errorCode)
{
	char* pString;

	switch (errorCode)
	{
		STR_CASE(XLNX_OK)
		STR_CASE(XLNX_ORDER_BOOK_ERROR_NOT_INITIALISED)
		STR_CASE(XLNX_ORDER_BOOK_ERROR_IO_FAILED)
		STR_CASE(XLNX_ORDER_BOOK_ERROR_INVALID_PARAMETER)
		STR_CASE(XLNX_ORDER_BOOK_ERROR_CU_NAME_NOT_FOUND)
		STR_CASE(XLNX_ORDER_BOOK_ERROR_CU_INDEX_NOT_FOUND)
        STR_CASE(XLNX_ORDER_BOOK_ERROR_SYMBOL_INDEX_OUT_OF_RANGE)


		default:
		{
			pString = (char*)"UKNOWN_ERROR";
			break;
		}
	}

	return pString;
}







static int OrderBook_ReadData(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	OrderBook* pOrderBook = (OrderBook*)pObjectData;
	OrderBook::OrderBookData data;
    static const uint32_t FORMAT_BUFFER_SIZE = 64;
    char formatBuffer[FORMAT_BUFFER_SIZE + 1];


	XLNX_UNUSED_ARG(argc);
	XLNX_UNUSED_ARG(argv);


    retval = pOrderBook->ReadData(&data);


	if (retval == XLNX_OK)
	{
        snprintf(formatBuffer, FORMAT_BUFFER_SIZE, "0x%016" PRIX64, data.timestamp);

        pShell->printf("+------------------------------------------------------------------------------+\n");
        pShell->printf("|         Symbol Index = %3u           ||    Timestamp = %18s    |\n", data.symbolIndex, formatBuffer);
		pShell->printf("+--------------------------------------++--------------------------------------+\n");
		pShell->printf("|                  BID                 ||                  ASK                 |\n");
		pShell->printf("+------------+------------+------------++------------+------------+------------+\n");
		pShell->printf("|   Count    |  Quantity  |   Price    ||   Price    |  Quantity  |   Count    |\n");
		pShell->printf("+------------+------------+------------++------------+------------+------------+\n");

		for (uint32_t i = 0; i < OrderBook::NUM_LEVELS; i++)
		{
			pShell->printf("| %10u | %10u | %10.2f || %10.2f | %10u | %10u |\n", data.bidCount[i],
																			     data.bidQuantity[i],
																			     (double)(data.bidPrice[i]) / 100.0,
																			     (double)(data.askPrice[i]) / 100.0,
																			     data.askQuantity[i],
																			     data.askCount[i]);
		}

		pShell->printf("+------------+------------+------------++------------+------------+------------+\n");
		pShell->printf("\n");
	}
	else
	{
		pShell->printf("[ERROR] retval = %s (0x%08X)\n", OrderBook_ErrorCodeToString(retval), retval);
	}




	return retval;
}












static int OrderBook_PrintStats(Shell* pShell, OrderBook* pOrderBook)
{
    int retval = 0;
    OrderBook::Stats statsCounters;

    retval = pOrderBook->GetStats(&statsCounters);

    if (retval == XLNX_OK)
    {
        pShell->printf("\n");
        pShell->printf("+----------------------------+------------+\n");
        pShell->printf("| Counter Name               |    Value   |\n");
        pShell->printf("+----------------------------+------------+\n");
        pShell->printf("| Rx Operations              | %10u |\n", statsCounters.numRxOperations);
        pShell->printf("| Processed Operations       | %10u |\n", statsCounters.numProcessedOperations);
        pShell->printf("| Invalid Operations         | %10u |\n", statsCounters.numInvalidOperations);
        pShell->printf("| Responses Generated        | %10u |\n", statsCounters.numResponsesGenerated);
        pShell->printf("| Responses Sent             | %10u |\n", statsCounters.numResponsesSent);
        pShell->printf("+----------------------------+------------+\n");
        pShell->printf("| ADD Operations             | %10u |\n", statsCounters.numAddOperations);
        pShell->printf("| MODIFY Operations          | %10u |\n", statsCounters.numModifyOperations);
        pShell->printf("| DELETE Operations          | %10u |\n", statsCounters.numDeleteOperations);
        pShell->printf("| TRANSACT Operations        | %10u |\n", statsCounters.numTransactOperations);
        pShell->printf("| HALT Operations            | %10u |\n", statsCounters.numHaltOperations);
        pShell->printf("+----------------------------+------------+\n");
        pShell->printf("| Timestamp Errors           | %10u |\n", statsCounters.numTimestampErrors);
        pShell->printf("| Unhandled OpCodes          | %10u |\n", statsCounters.numUnhandledOpCodes);
        pShell->printf("| Symbol Errors              | %10u |\n", statsCounters.numSymbolErrors);
        pShell->printf("| Direction Errors           | %10u |\n", statsCounters.numDirectionErrors);
        pShell->printf("| Level Errors               | %10u |\n", statsCounters.numLevelErrors);
        pShell->printf("+----------------------------+------------+\n");
        pShell->printf("| Clock Tick Events          | %10u |\n", statsCounters.numClockTickGeneratorEvents);
        pShell->printf("+----------------------------+------------+\n");
    }

    return retval;
}






static int OrderBook_GetStatus(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	OrderBook* pOrderBook = (OrderBook*)pObjectData;
    bool bIsInitialised = false;
	uint32_t cuIndex = 0;
	uint64_t cuAddress;
    bool bIsRunning;
    uint32_t captureSymbolIndex;
    bool bDataMoverOutputEnabled;

	XLNX_UNUSED_ARG(argc);
	XLNX_UNUSED_ARG(argv);


    pOrderBook->IsInitialised(&bIsInitialised);

    if (bIsInitialised == false)
    {
        retval = XLNX_ORDER_BOOK_ERROR_NOT_INITIALISED;
    }



    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }




   



    if (retval == XLNX_OK)
    {
        retval = pOrderBook->GetCUIndex(&cuIndex);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-30s | %20u |\n", "CU Index", cuIndex);
        }
    }




	if (retval == XLNX_OK)
	{
		retval = pOrderBook->GetCUAddress(&cuAddress);
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
        retval = pOrderBook->IsRunning(&bIsRunning);

        if (retval == XLNX_OK)
        {
            if (retval == XLNX_OK)
            {
                pShell->printf("| %-30s | %20s |\n", "Is Running", pShell->boolToString(bIsRunning));
            }
        }
    }






    if (retval == XLNX_OK)
    {
        retval = pOrderBook->GetCaptureFilter(&captureSymbolIndex);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-30s | %20u |\n", "Capture Filter Symbol Index", captureSymbolIndex);
        }
    }



    if (retval == XLNX_OK)
    {
        retval = pOrderBook->GetDataMoverOutput(&bDataMoverOutputEnabled);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-30s | %20s |\n", "Data Mover Output Enabled", pShell->boolToString(bDataMoverOutputEnabled));
        }
    }



    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
        pShell->printf("\n\n");
    }




    if (retval == XLNX_OK)
    {
        retval = OrderBook_PrintStats(pShell, pOrderBook);
    }





	if (retval != XLNX_OK)
	{
		pShell->printf("[ERROR] retval = %s (0x%08X)\n", OrderBook_ErrorCodeToString(retval), retval);
	}


	return retval;
}








static int OrderBook_ResetStats(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    OrderBook* pOrderBook = (OrderBook*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    retval = pOrderBook->ResetStats();

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        pShell->printf("[ERROR] %s (0x%08X)\n", OrderBook_ErrorCodeToString(retval), retval);
    }

    return retval;
}





static int OrderBook_SetCaptureIndex(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    bool bOKToContinue = true;
    uint32_t captureIndex;
    OrderBook* pOrderBook = (OrderBook*)pObjectData;

    if (argc < 2)
    {
        pShell->printf("Usage: %s <symbolindex>\n", argv[0]);
        bOKToContinue = false;
    }


    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[1], &captureIndex);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse symbol index\n");
        }
    }



    if (bOKToContinue)
    {
        retval = pOrderBook->SetCaptureFilter(captureIndex);


        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] retval = %s (0x%08X)\n", OrderBook_ErrorCodeToString(retval), retval);
        }
    }


    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }




    return retval;
}






static int OrderBook_Start(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    OrderBook* pOrderBook = (OrderBook*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    retval = pOrderBook->Start();

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        pShell->printf("[ERROR] %s (0x%08X)\n", OrderBook_ErrorCodeToString(retval), retval);
    }

    return retval;
}







static int OrderBook_Stop(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    OrderBook* pOrderBook = (OrderBook*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    retval = pOrderBook->Stop();

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        pShell->printf("[ERROR] %s (0x%08X)\n", OrderBook_ErrorCodeToString(retval), retval);
    }

    return retval;
}












static int OrderBook_SetDataMoverOutput(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    bool bOKToContinue = true;
    bool bEnabled;
    OrderBook* pOrderBook = (OrderBook*)pObjectData;

    if (argc < 2)
    {
        pShell->printf("Usage: %s <bool>\n", argv[0]);
        bOKToContinue = false;
    }

    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseBool(argv[1], &bEnabled);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse bool parameter\n");
        }
    }




    if (bOKToContinue)
    {
        retval = pOrderBook->SetDataMoverOutput(bEnabled);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] %s (0x%08X)\n", OrderBook_ErrorCodeToString(retval), retval);
        }
    }


    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }

    return retval;
}







CommandTableElement XLNX_ORDER_BOOK_COMMAND_TABLE[] =
{
    {"getstatus",	        OrderBook_GetStatus,	        "",			                "Get block status"	                            },
	{"readdata",	        OrderBook_ReadData,		        "",		                    "Read data"	                                    },
    {"resetstats",          OrderBook_ResetStats,           "",                         "Reset stats counters"                          },
    {"setcaptureindex",     OrderBook_SetCaptureIndex,      "<symbolindex>",            "Set the symbol index used to filter data"      },
    {/*-------------------------------------------------------------------------------------------------------------------------------*/},
    {"setdmoutput",         OrderBook_SetDataMoverOutput,   "<bool>",                   "Enable/disable output to data mover kernel"    },
    {/*-------------------------------------------------------------------------------------------------------------------------------*/},
    {"start",               OrderBook_Start,                "",                         "Starts the block running again"                },
    {"stop",                OrderBook_Stop,                 "",                         "Halts processing"                              },

};


const uint32_t XLNX_ORDER_BOOK_COMMAND_TABLE_LENGTH = (uint32_t)(sizeof(XLNX_ORDER_BOOK_COMMAND_TABLE) / sizeof(XLNX_ORDER_BOOK_COMMAND_TABLE[0]));
