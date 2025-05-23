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




#include "xlnx_shell_line_handler.h"
#include "xlnx_shell_utils.h"

#include "xlnx_line_handler.h"
#include "xlnx_line_handler_error_codes.h"
using namespace XLNX;



static const char* LINE_STRING = "------------------------------------------------------------------";






//Forward Declarations
static int LineHandler_PrintStats(Shell* pShell, LineHandler* pLineHandler);
static int LineHandler_PrintFilters(Shell* pShell, LineHandler* pLineHandler);









#define STR_CASE(TAG)	case(TAG):					\
                        {							\
                            pString = (char*) #TAG;	\
                            break;					\
                        }





static char* LineHandler_ErrorCodeToString(uint32_t errorCode)
{
    char* pString;

    switch (errorCode)
    {
        STR_CASE(XLNX_OK)
        STR_CASE(XLNX_LINE_HANDLER_ERROR_NOT_INITIALISED)
        STR_CASE(XLNX_LINE_HANDLER_ERROR_IO_FAILED)
        STR_CASE(XLNX_LINE_HANDLER_ERROR_INVALID_PARAMETER)
        STR_CASE(XLNX_LINE_HANDLER_ERROR_CU_NAME_NOT_FOUND)
        STR_CASE(XLNX_LINE_HANDLER_ERROR_CU_INDEX_NOT_FOUND)
        STR_CASE(XLNX_LINE_HANDLER_ERROR_INVALID_INPUT_PORT)
        STR_CASE(XLNX_LINE_HANDLER_ERROR_TIMER_INTERVAL_TOO_LARGE)
        STR_CASE(XLNX_LINE_HANDLER_ERROR_INVALID_IP_ADDRESS)
        STR_CASE(XLNX_LINE_HANDLER_ERROR_INVALID_PORT)
        STR_CASE(XLNX_LINE_HANDLER_ERROR_INVALID_SPLIT_ID)
        STR_CASE(XLNX_LINE_HANDLER_ERROR_NO_FREE_FILTER_SLOTS)
        STR_CASE(XLNX_LINE_HANDLER_ERROR_FILTER_ALREADY_EXISTS)
        STR_CASE(XLNX_LINE_HANDLER_ERROR_FILTER_DOES_NOT_EXIST)
        
        default:
        {
            pString = (char*)"UKNOWN_ERROR";
            break;
        }
    }

    return pString;
}
















static int LineHandler_GetStatus(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    LineHandler* pLineHandler = (LineHandler*)pObjectData;
    bool bIsInitialised = false;
    uint64_t cuAddress;
    uint32_t cuIndex;
    uint32_t inputPort;
    static const uint32_t BUFFER_SIZE = 64;
    char stringFormatBuffer[BUFFER_SIZE + 1];
    char ipAddressFormatBuffer[BUFFER_SIZE + 1];
    uint8_t ipAddrA, ipAddrB, ipAddrC, ipAddrD;
    uint16_t port;
    bool bEchoEnabled;
    uint32_t sequenceResetTimerMicroseconds;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    pLineHandler->IsInitialised(&bIsInitialised);
    if (bIsInitialised == false)
    {
        retval = XLNX_LINE_HANDLER_ERROR_NOT_INITIALISED;
    }


    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }


    if (retval == XLNX_OK)
    {
        retval = pLineHandler->GetCUAddress(&cuAddress);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-30s |   0x%016" PRIX64 " |\n", "CU Address", cuAddress);
        }
    }




    if (retval == XLNX_OK)
    {
        retval = pLineHandler->GetCUIndex(&cuIndex);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-30s | %20u |\n", "CU Index", cuIndex);
        }
    }


    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }




    if (retval == XLNX_OK)
    {
        for (uint32_t i = 0; i < LineHandler::NUM_SUPPORTED_INPUT_PORTS; i++)
        {
            inputPort = i;


            if (retval == XLNX_OK)
            {
                retval = pLineHandler->GetEchoEnabled(inputPort, &bEchoEnabled);

                if (retval == XLNX_OK)
                {
                    snprintf(stringFormatBuffer, BUFFER_SIZE, "Port %u Debug Echo Enabled", inputPort);
                    pShell->printf("| %-30s | %20s |\n", stringFormatBuffer, pShell->boolToString(bEchoEnabled));
                }
            }




            if (retval == XLNX_OK)
            {
                retval = pLineHandler->GetEchoDestination(inputPort, &ipAddrA, &ipAddrB, &ipAddrC, &ipAddrD, &port);

                if (retval == XLNX_OK)
                {
                    snprintf(ipAddressFormatBuffer, BUFFER_SIZE, "%u.%u.%u.%u", ipAddrA, ipAddrB, ipAddrC, ipAddrD);

                    snprintf(stringFormatBuffer, BUFFER_SIZE, "Port %u Debug Echo Address", inputPort);
                    pShell->printf("| %-30s | %20s |\n", stringFormatBuffer, ipAddressFormatBuffer);

                    snprintf(stringFormatBuffer, BUFFER_SIZE, "Port %u Debug Echo Port", inputPort);
                    pShell->printf("| %-30s | %20u |\n", stringFormatBuffer, port);
                }
            }



            if (retval == XLNX_OK)
            {
                pShell->printf("+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
            }

        }
    }


    if (retval == XLNX_OK)
    {
        retval = pLineHandler->GetSequenceResetTimer(&sequenceResetTimerMicroseconds);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-30s | %20u |\n", "Sequence Reset Timer (usecs)", sequenceResetTimerMicroseconds);
        }
       
    }


    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.30s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }

    if (retval == XLNX_OK)
    {
        pShell->printf("\n");
    }



    if (retval == XLNX_OK)
    {
        retval = LineHandler_PrintFilters(pShell, pLineHandler);
    }



    if (retval == XLNX_OK)
    {
        retval = LineHandler_PrintStats(pShell, pLineHandler);
    }





    


    if (retval != XLNX_OK)
    {
        pShell->printf("[ERROR] retval = %s (0x%08X)\n", LineHandler_ErrorCodeToString(retval), retval);
    }

    return retval;

}




static int LineHandler_PrintFilters(Shell* pShell, LineHandler* pLineHandler)
{
    int retval = 0;
    uint32_t inputPort;
    bool bSlotUsed;
    uint32_t numFiltersFound;
    uint8_t ipAddrA, ipAddrB, ipAddrC, ipAddrD;
    uint16_t port;
    uint32_t splitID;
    static const uint32_t FORMAT_BUFFER_SIZE = 64;
    char ipAddressFormatBuffer[FORMAT_BUFFER_SIZE + 1];

    pShell->printf("\n");
    pShell->printf("+-%.49s-+\n", LINE_STRING);
    pShell->printf("| %-14s %-15s %-15s |\n", " ", "Input Port Filters", " ");
    pShell->printf("+-%.49s-+\n", LINE_STRING);

    pShell->printf("| %-10s | %-15s | %-5s | %-10s |\n", "Input Port", "IP Address", "Port", "Split ID");
    pShell->printf("+-%.10s-+-%.15s-+-%.5s-+-%.10s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);

    for (uint32_t i = 0; i < LineHandler::NUM_SUPPORTED_INPUT_PORTS; i++)
    {
        inputPort = i;
        numFiltersFound = 0;
  
        for (uint32_t j = 0; j < LineHandler::NUM_SUPPORTED_FILTERS_PER_INPUT_PORT; j++)
        {
            retval = pLineHandler->FilterIndexIsUsed(inputPort, j, &bSlotUsed);

            if (retval == XLNX_OK)
            {
                if (bSlotUsed)
                {
                    retval = pLineHandler->GetFilterDetails(inputPort, j, &ipAddrA, &ipAddrB, &ipAddrC, &ipAddrD, &port, &splitID);

                    if (retval == XLNX_OK)
                    {
                        snprintf(ipAddressFormatBuffer, FORMAT_BUFFER_SIZE, "%u.%u.%u.%u", ipAddrA, ipAddrB, ipAddrC, ipAddrD);

                        if (numFiltersFound == 0)
                        {
                            pShell->printf("| %-10u | %-15s | %5u | %10u |\n", inputPort, ipAddressFormatBuffer, port, splitID);
                        }
                        else
                        {
                            pShell->printf("| %-10s | %-15s | %5u | %10u |\n", "", ipAddressFormatBuffer, port, splitID);
                        }
                       

                        numFiltersFound++;
                    }
                }
            }
        }


        if (numFiltersFound == 0)
        {
            pShell->printf("| %-10u | %-15s | %-5s | %-10s |\n", inputPort, "None", "None", "None");
        }

        pShell->printf("+-%.10s-+-%.15s-+-%.5s-+-%.10s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
    }

    pShell->printf("\n");


    return retval;
}








static int LineHandler_PrintStats(Shell* pShell, LineHandler* pLineHandler)
{
    int retval = 0;
    uint32_t inputPort;
    LineHandler::Stats statsCounters;
    static const uint32_t FORMAT_BUFFER_SIZE = 64;
    char formatBuffer[FORMAT_BUFFER_SIZE + 1];



    //Read the stats...
    retval = pLineHandler->GetStats(&statsCounters);



    if (retval == XLNX_OK)
    {
        pShell->printf("\n");
        pShell->printf("+-%.25s-+-%.10s-+\n", LINE_STRING, LINE_STRING);
        pShell->printf("| %-25s | %10s | \n", "Stats Counter", "Value");
        pShell->printf("+-%.25s-+-%.10s-+\n", LINE_STRING, LINE_STRING);
        
            
            

        for (uint32_t i = 0; i < LineHandler::NUM_SUPPORTED_INPUT_PORTS; i++)
        {
            inputPort = i;

            snprintf(formatBuffer, FORMAT_BUFFER_SIZE, "Port %u Rx Words", inputPort);
            pShell->printf("| %-25s | %10u |\n", formatBuffer, statsCounters.inputPortStats[inputPort].numRxWords);

            snprintf(formatBuffer, FORMAT_BUFFER_SIZE, "Port %u Rx Meta", inputPort);
            pShell->printf("| %-25s | %10u |\n", formatBuffer, statsCounters.inputPortStats[inputPort].numRxMeta);

            snprintf(formatBuffer, FORMAT_BUFFER_SIZE, "Port %u Dropped Words", inputPort);
            pShell->printf("| %-25s | %10u |\n", formatBuffer, statsCounters.inputPortStats[inputPort].numDroppedWords);
            
            pShell->printf("+-%.25s-+-%.10s-+\n", LINE_STRING, LINE_STRING);
        }
        
       
        pShell->printf("| %-25s | %10u |\n", "Total Packets Sent", statsCounters.totalPacketsSent);
        pShell->printf("| %-25s | %10u |\n", "Total Words Sent", statsCounters.totalWordsSent);
        pShell->printf("| %-25s | %10u |\n", "Total Packets Missed", statsCounters.totalPacketsMissed);




        for (uint32_t i = 0; i < LineHandler::NUM_SUPPORTED_INPUT_PORTS; i++)
        {
            inputPort = i;

            snprintf(formatBuffer, FORMAT_BUFFER_SIZE, "Port %u Rx Packets", inputPort);
            pShell->printf("| %-25s | %10u |\n", formatBuffer, statsCounters.inputPortStats[inputPort].numRxPackets);
        }



        for (uint32_t i = 0; i < LineHandler::NUM_SUPPORTED_INPUT_PORTS; i++)
        {
            inputPort = i;

            snprintf(formatBuffer, FORMAT_BUFFER_SIZE, "Port %u Arb. Tx Packets", inputPort);
            pShell->printf("| %-25s | %10u |\n", formatBuffer, statsCounters.inputPortStats[inputPort].numArbitratedTxPackets);
        }



        for (uint32_t i = 0; i < LineHandler::NUM_SUPPORTED_INPUT_PORTS; i++)
        {
            inputPort = i;

            snprintf(formatBuffer, FORMAT_BUFFER_SIZE, "Port %u Discarded Packets", inputPort);
            pShell->printf("| %-25s | %10u |\n", formatBuffer, statsCounters.inputPortStats[inputPort].numDiscardedPackets);
        }
            

        pShell->printf("+-%.25s-+-%.10s-+\n", LINE_STRING, LINE_STRING);



        pShell->printf("| %-25s | %10u |\n", "Clock Tick Events", statsCounters.clockTickEvents);

        pShell->printf("+-%.25s-+-%.10s-+\n", LINE_STRING, LINE_STRING);

        pShell->printf("\n");
    }







    return retval;
}










static int LineHandler_SetEchoEnabled(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    bool bOKToContinue = true;
    uint32_t inputPort;
    bool bEnabled = false;

    LineHandler* pLineHandler = (LineHandler*)pObjectData;

    if (argc != 3)
    {
        pShell->printf("Usage: %s <inputport> <bool>\n", argv[0]);
        bOKToContinue = false;
    }


    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[1], &inputPort);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse input port parameter\n");
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
        retval = pLineHandler->SetEchoEnabled(inputPort, bEnabled);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] %s (0x%08X)\n", LineHandler_ErrorCodeToString(retval), retval);
        }
    }



    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }


    return retval;
}







static int LineHandler_SetEchoDestination(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    bool bOKToContinue = true;
    uint8_t ipAddrA, ipAddrB, ipAddrC, ipAddrD;
    uint16_t port;
    uint32_t inputPort;
    LineHandler* pLineHandler = (LineHandler*)pObjectData;

    if (argc != 4)
    {
        pShell->printf("Usage: %s <inputport> <ipaddr> <port>\n", argv[0]);
        bOKToContinue = false;
    }

    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[1], &inputPort);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse input port parameter\n");
        }
    }

    if (bOKToContinue)
    {
        bOKToContinue = ParseIPv4Address(pShell, argv[2], &ipAddrA, &ipAddrB, &ipAddrC, &ipAddrD);
    }

    if (bOKToContinue)
    {
        bOKToContinue = ParsePort(pShell, argv[3], &port);
    }


    if (bOKToContinue)
    {
        retval = pLineHandler->SetEchoDestination(inputPort, ipAddrA, ipAddrB, ipAddrC, ipAddrD, port);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] %s (0x%08X)\n", LineHandler_ErrorCodeToString(retval), retval);
        }
    }

    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }


    return retval;
}







static int LineHandler_SetSequenceTimer(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    bool bOKToContinue = true;
    LineHandler* pLineHandler = (LineHandler*)pObjectData;
    uint32_t microseconds;



    if (argc != 2)
    {
        pShell->printf("Usage: %s <microseconds>\n", argv[0]);
        bOKToContinue = false;
    }


    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[1], &microseconds);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse microseconds parameter\n");
        }
    }


    if (bOKToContinue)
    {
        retval = pLineHandler->SetSequenceResetTimer(microseconds);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] %s (0x%08X)\n", LineHandler_ErrorCodeToString(retval), retval);
        }
    }


    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }

    return retval;
}








static int LineHandler_ResetSequence(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    LineHandler* pLineHandler = (LineHandler*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);


    retval = pLineHandler->ResetSequenceNumber();

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        pShell->printf("[ERROR] %s (0x%08X)\n", LineHandler_ErrorCodeToString(retval), retval);
    }
    
    return retval;
}






static int LineHandler_AddFilter(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    bool bOKToContinue = true;
    uint32_t inputPort;
    uint8_t ipAddrA, ipAddrB, ipAddrC, ipAddrD;
    uint16_t port;
    uint32_t splitID;

    LineHandler* pLineHandler = (LineHandler*)pObjectData;

    if (argc != 5)
    {
        pShell->printf("Usage: %s <inputport> <ipaddr> <port> <splitID>\n", argv[0]);
        bOKToContinue = false;
    }


    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[1], &inputPort);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse input port parameter\n");
        }
    }


    if (bOKToContinue)
    {
        bOKToContinue = ParseIPv4Address(pShell, argv[2], &ipAddrA, &ipAddrB, &ipAddrC, &ipAddrD);
    }


    if (bOKToContinue)
    {
        bOKToContinue = ParsePort(pShell, argv[3], &port);
    }

    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[4], &splitID);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse splitID parameter\n");
        }
    }


    if (bOKToContinue)
    {
        retval = pLineHandler->Add(inputPort, ipAddrA, ipAddrB, ipAddrC, ipAddrD, port, splitID);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] %s (0x%08X)\n", LineHandler_ErrorCodeToString(retval), retval);
        }
    }


    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }

    return retval;
}




static int LineHandler_DeleteFilter(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    bool bOKToContinue = true;
    uint32_t inputPort;
    uint8_t ipAddrA, ipAddrB, ipAddrC, ipAddrD;
    uint16_t port;
    uint32_t splitID;

    LineHandler* pLineHandler = (LineHandler*)pObjectData;

    if (argc != 5)
    {
        pShell->printf("Usage: %s <inputport> <ipaddr> <port> <splitID>\n", argv[0]);       
        bOKToContinue = false;
    }


    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[1], &inputPort);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse input port parameter\n");
        }
    }


    if (bOKToContinue)
    {
        bOKToContinue = ParseIPv4Address(pShell, argv[2], &ipAddrA, &ipAddrB, &ipAddrC, &ipAddrD);
    }


    if (bOKToContinue)
    {
        bOKToContinue = ParsePort(pShell, argv[3], &port);
    }

    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[4], &splitID);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse splitID parameter\n");
        }
    }


    if (bOKToContinue)
    {
        retval = pLineHandler->Delete(inputPort, ipAddrA, ipAddrB, ipAddrC, ipAddrD, port, splitID);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] %s (0x%08X)\n", LineHandler_ErrorCodeToString(retval), retval);
        }
    }


    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }

    return retval;
}






static int LineHandler_DeleteAllFilters(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = 0;
    bool bOKToContinue = true;
    uint32_t inputPort;

    LineHandler* pLineHandler = (LineHandler*)pObjectData;

    if (argc != 2)
    {
        pShell->printf("Usage: %s <inputport>\n", argv[0]);   
        bOKToContinue = false;
    }


    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[1], &inputPort);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse input port parameter\n");
        }
    }



    if (bOKToContinue)
    {
        retval = pLineHandler->DeleteAll(inputPort);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] %s (0x%08X)\n", LineHandler_ErrorCodeToString(retval), retval);
        }
    }

    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }


    return retval;
}









CommandTableElement XLNX_LINE_HANDLER_COMMAND_TABLE[] =
{
   
    {"getstatus",	        LineHandler_GetStatus,			    "",					                        "Get block status"			                    },
    {/*---------------------------------------------------------------------------------------------------------------------------------------------------*/},
    {"add",                 LineHandler_AddFilter,              "<inputport> <ipaddr> <port> <splitID>",    "Adds a multicast filter for a port"            },
    {"delete",              LineHandler_DeleteFilter,           "<inputport> <ipaddr> <port> <splitID>",    "Deletes a multicast filter from a port"        },
    {"deleteall",           LineHandler_DeleteAllFilters,       "<inputport>",                              "Deletes ALL multicast filters from a port"     },
    {/*---------------------------------------------------------------------------------------------------------------------------------------------------*/},
    {"setechoenabled",      LineHandler_SetEchoEnabled,         "<inputport> <bool>",				        "Enables/disables debug traffic echo"           },
    {"setechodest",         LineHandler_SetEchoDestination,     "<inputport> <ipaddr> <port>",		        "Sets the UDP destination for debug echo"       },
    {/*---------------------------------------------------------------------------------------------------------------------------------------------------*/},
    {"setsequencetimer",    LineHandler_SetSequenceTimer,       "<microseconds>",                           "Sets the sequence reset timer"                 },
    {"resetsequence",       LineHandler_ResetSequence,          "",                                         "Reset the next expected sequence value"        }
};


const uint32_t XLNX_LINE_HANDLER_COMMAND_TABLE_LENGTH = (uint32_t)(sizeof(XLNX_LINE_HANDLER_COMMAND_TABLE) / sizeof(XLNX_LINE_HANDLER_COMMAND_TABLE[0]));
