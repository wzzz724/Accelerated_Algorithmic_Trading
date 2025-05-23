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


#include "xlnx_shell_pricing_engine.h"
#include "xlnx_shell_utils.h"

#include "xlnx_pricing_engine.h"
#include "xlnx_pricing_engine_error_codes.h"
using namespace XLNX;





static const char* LINE_STRING = "--------------------------------------------------------------------";

static const char* NONE_STRING  = "NONE";
static const char* PEG_STRING   = "PEG";
static const char* LIMIT_STRING = "LIMIT";



#define STR_CASE(TAG)	case(TAG):					\
						{							\
							pString = (char*) #TAG;	\
							break;					\
						}





static char* PricingEngine_ErrorCodeToString(uint32_t errorCode)
{
    char* pString;

    switch (errorCode)
    {
        STR_CASE(XLNX_OK)
        STR_CASE(XLNX_PRICING_ENGINE_ERROR_NOT_INITIALISED)
        STR_CASE(XLNX_PRICING_ENGINE_ERROR_IO_FAILED)
        STR_CASE(XLNX_PRICING_ENGINE_ERROR_INVALID_PARAMETER)
        STR_CASE(XLNX_PRICING_ENGINE_ERROR_CU_NAME_NOT_FOUND)
        STR_CASE(XLNX_PRICING_ENGINE_ERROR_CU_INDEX_NOT_FOUND)
        STR_CASE(XLNX_PRICING_ENGINE_ERROR_SYMBOL_INDEX_OUT_OF_RANGE)
        STR_CASE(XLNX_PRICING_ENGINE_ERROR_FAILED_TO_OPEN_STREAM_INTERFACE)
        STR_CASE(XLNX_PRICING_ENGINE_ERROR_STREAM_INTERFACE_ALREADY_OPEN)
        STR_CASE(XLNX_PRICING_ENGINE_ERROR_STREAM_INFERFACE_NOT_OPEN)


        default:
        {
            pString = (char*)"UKNOWN_ERROR";
            break;
        }
    }

    return pString;
}






static char* PricingEngine_OrderSideToString(PricingEngine::OrderSide orderSide)
{
    char* pString;

    switch (orderSide)
    {
        case(PricingEngine::OrderSide::ORDER_SIDE_BID):
        {
            pString = (char*)"BID";
            break;
        }

        case(PricingEngine::OrderSide::ORDER_SIDE_ASK):
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










static char* PricingEngine_OrderOperationToString(PricingEngine::OrderOperation orderOperation)
{
    char* pString;

    switch (orderOperation)
    {
        case(PricingEngine::OrderOperation::ORDER_OPERATION_ADD):
        {
            pString = (char*)"ADD";
            break;
        }

        case(PricingEngine::OrderOperation::ORDER_OPERATION_MODIFY):
        {
            pString = (char*)"MODIFY";
            break;
        }

        case(PricingEngine::OrderOperation::ORDER_OPERATION_DELETE):
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





static char* PricingEngine_PricingStrategyToString(PricingEngine::PricingStrategy strategy)
{
    char* pString;

    switch (strategy)
    {
        case(PricingEngine::PricingStrategy::STRATEGY_NONE):
        {
            pString = (char*)NONE_STRING;
            break;
        }

        case(PricingEngine::PricingStrategy::STRATEGY_PEG):
        {
            pString = (char*)PEG_STRING;
            break;
        }

        case(PricingEngine::PricingStrategy::STRATEGY_LIMIT):
        {
            pString = (char*)LIMIT_STRING;
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





static bool PricingEngine_ParsePricingStrategy(char* pToken, PricingEngine::PricingStrategy* pStrategy)
{
    bool bOKToContinue = true;
    uint32_t tokenLength;

    tokenLength = strlen(pToken);

    //convert to all upper case...makes comparison easier...
    for (uint32_t i = 0; i < tokenLength; i++)
    {
        pToken[i] = (char)toupper(pToken[i]);
    }


    if (strcmp(pToken, NONE_STRING) == 0)
    {
        *pStrategy = PricingEngine::PricingStrategy::STRATEGY_NONE;
    }
    else if(strcmp(pToken, PEG_STRING) == 0)
    {
        *pStrategy = PricingEngine::PricingStrategy::STRATEGY_PEG;
    }
    else if (strcmp(pToken, LIMIT_STRING) == 0)
    {
        *pStrategy = PricingEngine::PricingStrategy::STRATEGY_LIMIT;
    }
    else
    {
        bOKToContinue = false;
    }

    return bOKToContinue;

}













static int PricingEngine_PrintStats(Shell* pShell, PricingEngine* pPricingEngine)
{
    int retval = 0;
    PricingEngine::Stats statsCounters;

    retval = pPricingEngine->GetStats(&statsCounters);

    if (retval == XLNX_OK)
    {
        pShell->printf("\n");
        pShell->printf("+-%.26s-+-%.10s-+\n", LINE_STRING, LINE_STRING);
        pShell->printf("| %-26s | %-10s |\n", "Counter Name", "Value");
        pShell->printf("+-%.26s-+-%.10s-+\n", LINE_STRING, LINE_STRING);
        pShell->printf("| %-26s | %10u |\n", "Rx Responses",        statsCounters.numRxResponses);
        pShell->printf("| %-26s | %10u |\n", "Processed Responses", statsCounters.numProcessedResponses);
        pShell->printf("| %-26s | %10u |\n", "Tx Operations",       statsCounters.numTxOperations);
        pShell->printf("+-%.26s-+-%.10s-+\n", LINE_STRING, LINE_STRING);
        pShell->printf("| %-26s | %10u |\n", "Strategy NONE",       statsCounters.numStrategyNone);
        pShell->printf("| %-26s | %10u |\n", "Strategy PEG",        statsCounters.numStrategyPeg);
        pShell->printf("| %-26s | %10u |\n", "Strategy LIMIT",      statsCounters.numStrategyLimit);
        pShell->printf("| %-26s | %10u |\n", "Strategy UNKNOWN",    statsCounters.numStrategyUnknown);
        pShell->printf("+-%.26s-+-%.10s-+\n", LINE_STRING, LINE_STRING);
        pShell->printf("| %-26s | %10u |\n", "Clock Tick Events",    statsCounters.numClockTickEvents);
        pShell->printf("+-%.26s-+-%.10s-+\n", LINE_STRING, LINE_STRING);


    }

    return retval;
}








static int PricingEngine_GetStatus(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    PricingEngine* pPricingEngine = (PricingEngine*)pObjectData;
    bool bIsInitialised;
    uint32_t cuIndex = 0;
    uint64_t cuAddress;
    bool bIsRunning;
    uint32_t captureSymbolIndex;
    bool bGlobalStrategyEnabled;
    PricingEngine::PricingStrategy globalStrategy;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);


    pPricingEngine->IsInitialised(&bIsInitialised);
    if (bIsInitialised == false)
    {
        retval = XLNX_PRICING_ENGINE_ERROR_NOT_INITIALISED;
    }


    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }



    if (retval == XLNX_OK)
    {
        retval = pPricingEngine->GetCUIndex(&cuIndex);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-30s | %20u |\n", "CU Index", cuIndex);
        }
    }





    if (retval == XLNX_OK)
    {
        retval = pPricingEngine->GetCUAddress(&cuAddress);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-30s |   0x%016" PRIX64 " |\n", "CU Address", cuAddress);
        }
    }



    if (retval == XLNX_OK)
    {
        retval = pPricingEngine->IsRunning(&bIsRunning);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-30s | %20s |\n", "IsRunning", pShell->boolToString(bIsRunning));
        }
    }



    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }



    if (retval == XLNX_OK)
    {
        retval = pPricingEngine->GetCaptureFilter(&captureSymbolIndex);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-30s | %20u |\n", "Capture Filter Symbol Index", captureSymbolIndex);
        }
    }





    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }






    if (retval == XLNX_OK)
    {
        retval = pPricingEngine->GetGlobalStrategyMode(&bGlobalStrategyEnabled);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-30s | %20s |\n", "Global Strategy Enabled", pShell->boolToString(bGlobalStrategyEnabled));
        }
    }


    if (retval == XLNX_OK)
    {
        retval = pPricingEngine->GetGlobalStrategy(&globalStrategy);

        if (retval == XLNX_OK)
        {
            //We will only print the global strategy if global mode is enabled....
            if (bGlobalStrategyEnabled)
            {
                pShell->printf("| %-30s | %20s |\n", "Global Strategy", PricingEngine_PricingStrategyToString(globalStrategy));
            }
            else
            {
                pShell->printf("| %-30s | %20s |\n", "Global Strategy", "N/A");
            }
        }
    }


    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
        pShell->printf("\n");
    }




    if (retval == XLNX_OK)
    {
        retval = PricingEngine_PrintStats(pShell, pPricingEngine);
    }







    if (retval != XLNX_OK)
    {
        pShell->printf("[ERROR] retval = %s (0x%08X)\n", PricingEngine_ErrorCodeToString(retval), retval);
    }


    return retval;
}










static int PricingEngine_ReadData(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    PricingEngine* pPricingEngine = (PricingEngine*)pObjectData;
    PricingEngine::PricingEngineData data;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);


    retval = pPricingEngine->ReadData(&data);


    if (retval == XLNX_OK)
	{
        pShell->printf("+----------------------+------------+\n");
        pShell->printf("| Symbol Index         | %10u |\n", data.symbolIndex);
        pShell->printf("| Order Side           | %10s |\n", PricingEngine_OrderSideToString(data.orderSide));
        pShell->printf("| Order Operation      | %10s |\n", PricingEngine_OrderOperationToString(data.orderOperation));
        pShell->printf("| Order ID             | %10u |\n", data.orderID);
        pShell->printf("| Order Price          | %10.2f |\n", (double)(data.orderPrice / 100.0));
        pShell->printf("| Order Quantity       | %10u |\n", data.orderQuantity);
        pShell->printf("+----------------------+------------+\n");
	}
	else
	{
		pShell->printf("[ERROR] retval = %s (0x%08X)\n", PricingEngine_ErrorCodeToString(retval), retval);
	}

    return retval;
}











static int PricingEngine_ResetStats(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    PricingEngine* pPricingEngine = (PricingEngine*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    retval = pPricingEngine->ResetStats();

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        pShell->printf("[ERROR] %s (0x%08X)\n", PricingEngine_ErrorCodeToString(retval), retval);
    }

    return retval;
}








static int PricingEngine_SetGlobalMode(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    PricingEngine* pPricingEngine = (PricingEngine*)pObjectData;
    bool bOKToContinue = true;
    bool bEnabled;

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
        retval = pPricingEngine->SetGlobalStrategyMode(bEnabled);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] %s (0x%08X)\n", PricingEngine_ErrorCodeToString(retval), retval);
        }
    }

    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }

    return retval;
}






static int PricingEngine_SetGlobalStrategy(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    bool bOKToContinue = true;
    PricingEngine* pPricingEngine = (PricingEngine*)pObjectData;
    PricingEngine::PricingStrategy strategy;

    if (argc != 2)
    {
        pShell->printf("Usage: %s <none|peg|limit>\n", argv[0]);
        bOKToContinue = false;
    }


    if (bOKToContinue)
    {
        bOKToContinue = PricingEngine_ParsePricingStrategy(argv[1], &strategy);
    }

    if (bOKToContinue)
    {
        retval = pPricingEngine->SetGlobalStrategy(strategy);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] %s (0x%08X)\n", PricingEngine_ErrorCodeToString(retval), retval);
        }
    }

    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }


    return retval;
}






CommandTableElement XLNX_PRICING_ENGINE_COMMAND_TABLE[] =
{
    {"setglobalmode",       PricingEngine_SetGlobalMode,        "<bool>",                   "Enables/Disable global pricing strategy"   },
    {"setglobalstrategy",   PricingEngine_SetGlobalStrategy,    "<none|peg|limit>",         "Sets strategy to be applied to ALL symbols"},
    {/*-------------------------------------------------------------------------------------------------------------------------------*/},
    {"getstatus",	        PricingEngine_GetStatus,	        "",			                "Get block status"	                        },
    {"readdata",	        PricingEngine_ReadData,		        "",		                    "Read data"	                                },
    {"resetstats",          PricingEngine_ResetStats,           "",                         "Reset stats counters"                      }
};


const uint32_t XLNX_PRICING_ENGINE_COMMAND_TABLE_LENGTH = (uint32_t)(sizeof(XLNX_PRICING_ENGINE_COMMAND_TABLE) / sizeof(XLNX_PRICING_ENGINE_COMMAND_TABLE[0]));