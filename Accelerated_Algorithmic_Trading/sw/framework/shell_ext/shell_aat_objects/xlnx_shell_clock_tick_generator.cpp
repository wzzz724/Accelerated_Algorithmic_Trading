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



#include "xlnx_shell_clock_tick_generator.h"
#include "xlnx_shell_utils.h"

#include "xlnx_clock_tick_generator.h"
#include "xlnx_clock_tick_generator_error_codes.h"
using namespace XLNX;



static const char* LINE_STRING = "-------------------------------------------------------------------------";








#define STR_CASE(TAG)	case(TAG):					\
						{							\
							pString = (char*) #TAG;	\
							break;					\
						}





static char* ClockTickGenerator_ErrorCodeToString(uint32_t errorCode)
{
    char* pString;

    switch (errorCode)
    {
        STR_CASE(XLNX_OK)
        STR_CASE(XLNX_CLOCK_TICK_GENERATOR_ERROR_NOT_INITIALISED)		 
        STR_CASE(XLNX_CLOCK_TICK_GENERATOR_ERROR_IO_FAILED)
        STR_CASE(XLNX_CLOCK_TICK_GENERATOR_ERROR_INVALID_PARAMETER)
        STR_CASE(XLNX_CLOCK_TICK_GENERATOR_ERROR_CU_NAME_NOT_FOUND)
        STR_CASE(XLNX_CLOCK_TICK_GENERATOR_ERROR_CU_INDEX_NOT_FOUND)
        STR_CASE(XLNX_CLOCK_TICK_GENERATOR_ERROR_STREAM_INDEX_OUT_OF_RANGE)
        STR_CASE(XLNX_CLOCK_TICK_GENERATOR_ERROR_INTERVAL_TOO_LARGE)


        default:
        {
            pString = (char*)"UKNOWN_ERROR";
            break;
        }
    }

    return pString;
}




static int ClockTickGenerator_SetEnable(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    bool bOKToContinue = true;
    ClockTickGenerator* pClockTickGen = (ClockTickGenerator*)pObjectData;
    uint32_t tickStream;
    bool bEnabled;

   
    if (argc != 3)
    {
        pShell->printf("Usage: %s <index> <bool>\n", argv[0]);
        bOKToContinue = false;
    }



    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[1], &tickStream);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse tick stream index parameter");
        }
    }


    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseBool(argv[2], &bEnabled);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse bool parameter");
        }
    }


    if (bOKToContinue)
    {
        retval = pClockTickGen->SetTickEnabled(tickStream, bEnabled);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] %s (0x%08X)\n", ClockTickGenerator_ErrorCodeToString(retval), retval);
        }
    }


    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }

    return retval;
}




static int ClockTickGenerator_SetInterval(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    bool bOKToContinue = true;
    ClockTickGenerator* pClockTickGen = (ClockTickGenerator*)pObjectData;
    uint32_t tickStream;
    uint32_t microseconds;


    if (argc != 3)
    {
        pShell->printf("Usage: %s <index> <usecs>\n", argv[0]);
        bOKToContinue = false;
    }



    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[1], &tickStream);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse tick stream index parameter");
        }
    }


    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[2], &microseconds);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse tick interval parameter");
        }
    }


    if (bOKToContinue)
    {
        retval = pClockTickGen->SetTickInterval(tickStream, microseconds);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] %s (0x%08X)\n", ClockTickGenerator_ErrorCodeToString(retval), retval);
        }
    }

    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }


    return retval;
}









static int ClockTickGenerator_GetStatus(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    ClockTickGenerator* pClockTickGen = (ClockTickGenerator*)pObjectData;
    bool bIsInitialised = false;
    uint64_t cuAddress;
    uint32_t cuIndex;
    uint32_t clockFreqMHz;
    ClockTickGenerator::Stats stats;
    bool bTickEnabled;
    uint32_t tickIntervalMicroseconds;
    uint32_t tickIntervalClockCycles;
   


    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);


    pClockTickGen->IsInitialised(&bIsInitialised);
    if (bIsInitialised == false)
    {
        retval = XLNX_CLOCK_TICK_GENERATOR_ERROR_NOT_INITIALISED;
    }


    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.25s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }


    if (retval == XLNX_OK)
    {
        retval = pClockTickGen->GetCUAddress(&cuAddress);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-25s |   0x%016" PRIX64 " |\n", "CU Address", cuAddress);
        }
    }




    if (retval == XLNX_OK)
    {
        retval = pClockTickGen->GetCUIndex(&cuIndex);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-25s | %20u |\n", "CU Index", cuIndex);
        }
    }

    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.25s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }




    if (retval == XLNX_OK)
    {
        retval = pClockTickGen->GetClockFrequency(&clockFreqMHz);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-25s | %20u |\n", "Clock Frequency (MHz)", clockFreqMHz);
        }
    }

    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.25s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }



    if (retval == XLNX_OK)
    {
        retval = pClockTickGen->GetStats(&stats);
    }


    if (retval == XLNX_OK)
    {
        pShell->printf("| %-25s | %20u |\n", "Free Running Tick Count", stats.tickCount);      
    }

    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.25s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }





    if (retval == XLNX_OK)
    {
        pShell->printf("\n");
    }


    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.5s-+-%.7s-+-%.13s-+-%.21s-+-%.16s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
        pShell->printf("| %-5s | %-7s | %-13s | %-21s | %-16s |\n", "Index", "Enabled", "Interval (us)", "Interval (Clk Cycles)", "Num Events Fired");
        pShell->printf("+-%.5s-+-%.7s-+-%.13s-+-%.21s-+-%.16s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);

        for (uint32_t i = 0; i < ClockTickGenerator::NUM_SUPPORTED_TICK_STREAMS; i++)
        {
            retval = pClockTickGen->GetTickEnabled(i, &bTickEnabled);

            if (retval == XLNX_OK)
            {
                retval = pClockTickGen->GetTickInterval(i, &tickIntervalMicroseconds);
            }

            if (retval == XLNX_OK)
            {
                retval = pClockTickGen->GetTickIntervalClockCycles(i, &tickIntervalClockCycles);
            }

            if (retval == XLNX_OK)
            {
                pShell->printf("| %-5u | %7s | %13u | %21u | %16u |\n", i, 
                                                                        pShell->boolToString(bTickEnabled), 
                                                                        tickIntervalMicroseconds, 
                                                                        tickIntervalClockCycles, 
                                                                        stats.numTickEventsFired[i]);
            }

            if (retval != XLNX_OK)
            {
                break; //out of loop;
            }
        }

        pShell->printf("+-%.5s-+-%.7s-+-%.13s-+-%.21s-+-%.16s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
    }



    if (retval != XLNX_OK)
    {
        pShell->printf("[ERROR] retval = %s (0x%08X)\n", ClockTickGenerator_ErrorCodeToString(retval), retval);
    }

    return retval;
}










CommandTableElement XLNX_CLOCK_TICK_GENERATOR_COMMAND_TABLE[] =
{
    {"setenable",           ClockTickGenerator_SetEnable,               "<index> <bool>",               "Enable/disable a tick stream"      },
    {"setinterval",         ClockTickGenerator_SetInterval,             "<index> <usecs>",              "Set the tick event interval"       },
    {/*-----------------------------------------------------------------------------------------------------------------------------------*/},
    {"getstatus",	        ClockTickGenerator_GetStatus,			    "",					            "Get block status"			        },
    


};


const uint32_t XLNX_CLOCK_TICK_GENERATOR_COMMAND_TABLE_LENGTH = (uint32_t)(sizeof(XLNX_CLOCK_TICK_GENERATOR_COMMAND_TABLE) / sizeof(XLNX_CLOCK_TICK_GENERATOR_COMMAND_TABLE[0]));
