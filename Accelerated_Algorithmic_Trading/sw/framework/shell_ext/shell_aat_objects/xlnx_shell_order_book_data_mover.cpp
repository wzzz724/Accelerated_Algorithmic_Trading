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

#include "xlnx_shell_order_book_data_mover.h"
#include "xlnx_shell_utils.h"

#include "xlnx_order_book_data_mover.h"
#include "xlnx_order_book_data_mover_error_codes.h"
using namespace XLNX;








#define STR_CASE(TAG)	case(TAG):					\
						{							\
							pString = (char*) #TAG;	\
							break;					\
						}


static const char* LINE_STRING = "--------------------------------------------------------------------------------------------------";


static char* OrderBookDataMover_ErrorCodeToString(uint32_t errorCode)
{
	char* pString;

	switch (errorCode)
	{
		STR_CASE(XLNX_OK)
		STR_CASE(XLNX_ORDER_BOOK_DATA_MOVER_ERROR_NOT_INITIALISED)
		STR_CASE(XLNX_ORDER_BOOK_DATA_MOVER_ERROR_IO_FAILED)
		STR_CASE(XLNX_ORDER_BOOK_DATA_MOVER_ERROR_INVALID_PARAMETER)
		STR_CASE(XLNX_ORDER_BOOK_DATA_MOVER_ERROR_CU_NAME_NOT_FOUND)
		STR_CASE(XLNX_ORDER_BOOK_DATA_MOVER_ERROR_CU_INDEX_NOT_FOUND)
        STR_CASE(XLNX_ORDER_BOOK_DATA_MOVER_ERROR_FAILED_TO_ALLOCATE_BUFFER_OBJECT)
        STR_CASE(XLNX_ORDER_BOOK_DATA_MOVER_ERROR_FAILED_TO_MAP_BUFFER_OBJECT)
        STR_CASE(XLNX_ORDER_BOOK_DATA_MOVER_ERROR_FAILED_TO_SYNC_BUFFER_OBJECT)

	    default:
		{
			pString = (char*)"UKNOWN_ERROR";
			break;
		}
	}

	return pString;
}











static int OrderBookDataMover_GetStatus(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    OrderBookDataMover* pDataMover = (OrderBookDataMover*)pObjectData;
    bool bIsInitialised = false;
    uint32_t cuIndex = 0;
    uint64_t cuAddress;
    uint32_t cuMemTopologyIndex;
    bool bIsRunning;
    bool bYielding;
    OrderBookDataMover::Stats stats;
    OrderBookDataMover::ThreadStats threadStats;

    uint32_t headIndex;
    uint32_t tailIndex;

    uint32_t throttleRate;
    OrderBookDataMover::ThrottleStats throttleStats;

    OrderBookDataMover::DMAStats dmaH2CStats;
    OrderBookDataMover::DMAStats dmaC2HStats;

    uint32_t numElements;

    uint32_t hwEmuPollDelay;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);


    pDataMover->IsInitialised(&bIsInitialised);

    if (bIsInitialised == false)
    {
        retval = XLNX_ORDER_BOOK_DATA_MOVER_ERROR_NOT_INITIALISED;
    }



    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }








    if (retval == XLNX_OK)
    {
        retval = pDataMover->GetCUIndex(&cuIndex);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20u |\n", "CU Index", cuIndex);
        }
    }




    if (retval == XLNX_OK)
    {
        retval = pDataMover->GetCUAddress(&cuAddress);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s |   0x%016" PRIX64 " |\n", "CU Address", cuAddress);
        }
    }



    if (retval == XLNX_OK)
    {
        retval = pDataMover->GetReadBufferCUMemTopologyIndex(&cuMemTopologyIndex);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20u |\n", "Read Buffer CU Mem Topology Index", cuMemTopologyIndex);
        }
    }


    if (retval == XLNX_OK)
    {
        retval = pDataMover->GetWriteBufferCUMemTopologyIndex(&cuMemTopologyIndex);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20u |\n", "Write Buffer CU Mem Topology Index", cuMemTopologyIndex);
        }
    }





    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }






    if (retval == XLNX_OK)
    {
        void* pHostBuffer = pDataMover->GetReadBufferHostVirtualAddress();

        if (pHostBuffer != nullptr)
        {
            pShell->printf("| %-35s |   0x%016" PRIXPTR " |\n", "Read Buffer Host Virtual Address", (uintptr_t)pHostBuffer);
        }
        else
        {
            pShell->printf("| %-35s | %20s |\n", "Read Buffer Host Virtual Address", "Deferred Until Sync");
        }
    }




    

    if(retval == XLNX_OK)
    {
    
        uint64_t hwBufferAddress = pDataMover->GetReadBufferHWAddress();

        if (hwBufferAddress != OrderBookDataMover::INVALID_HW_BUFFER_ADDRESS)
        {
            pShell->printf("| %-35s |   0x%016" PRIX64 " |\n", "Read Buffer HW Address", hwBufferAddress);
        }
        else
        {
            pShell->printf("| %-35s | %20s |\n", "Read Buffer HW Address", "Deferred Until Sync");
        }  
    }


    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }


   

    if (retval == XLNX_OK)
    {
        void* pHostBuffer = pDataMover->GetWriteBufferHostVirtualAddress();

        if (pHostBuffer != nullptr)
        {
            pShell->printf("| %-35s |   0x%016" PRIXPTR " |\n", "Write Buffer Host Virtual Address", (uintptr_t)pHostBuffer);
        }
        else
        {
            pShell->printf("| %-35s | %20s |\n", "Write Buffer Host Virtual Address", "Deferred Until Sync");
        }
    }


   


    if(retval == XLNX_OK)
    {
        
        uint64_t hwBufferAddress = pDataMover->GetWriteBufferHWAddress();

        if (hwBufferAddress != OrderBookDataMover::INVALID_HW_BUFFER_ADDRESS)
        {
            pShell->printf("| %-35s |   0x%016" PRIX64 " |\n", "Write Buffer HW Address", hwBufferAddress);
        }
        else
        {
            pShell->printf("| %-35s | %20s |\n", "Write Buffer HW Address", "Deferred Until Sync");
        }   
    }




    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }


    if (retval == XLNX_OK)
    {
        retval = pDataMover->GetHWRingReadBufferIndexes(&headIndex, &tailIndex);
        if (retval == XLNX_OK)
        {  
            pShell->printf("| %-35s | %20u |\n", "HW Ring Read Buffer Head", headIndex);  
            pShell->printf("| %-35s | %20u |\n", "HW Ring Read Buffer Tail", tailIndex);
        }
    }


    if (retval == XLNX_OK)
    {
        retval = pDataMover->GetHWRingWriteBufferIndexes(&headIndex, &tailIndex);
        if (retval == XLNX_OK)
        {  
            pShell->printf("| %-35s | %20u |\n", "HW Ring Write Buffer Head", headIndex);  
            pShell->printf("| %-35s | %20u |\n", "HW Ring Write Buffer Tail", tailIndex);
        }
    }


    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }

    if (retval == XLNX_OK)
    {
        retval = pDataMover->GetSWRingReadBufferIndexes(&headIndex, &tailIndex);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20u |\n", "SW Ring Read Buffer Head", headIndex);
            pShell->printf("| %-35s | %20u |\n", "SW Ring Read Buffer Tail", tailIndex);
        }
    }

    if (retval == XLNX_OK)
    {
        retval = pDataMover->GetSWRingWriteBufferIndexes(&headIndex, &tailIndex);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20u |\n", "SW Ring Write Buffer Head", headIndex);
            pShell->printf("| %-35s | %20u |\n", "SW Ring Write Buffer Tail", tailIndex);
        }
    }

    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }




    if (retval == XLNX_OK)
    {
        retval = pDataMover->IsHWKernelRunning(&bIsRunning);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20s |\n", "HW Kernel Is Running", pShell->boolToString(bIsRunning));
        }
    }


    if (retval == XLNX_OK)
    {
        retval = pDataMover->IsProcessingThreadRunning(&bIsRunning);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20s |\n", "SW Thread Is Running", pShell->boolToString(bIsRunning));
        }
    }

    if (retval == XLNX_OK)
    {
        retval = pDataMover->GetThreadYield(&bYielding);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20s |\n", "SW Thread Yielding Enabled", pShell->boolToString(bYielding));
        }
    }

    if (retval == XLNX_OK)
    {
        pDataMover->GetHWEmulationPollDelay(&hwEmuPollDelay);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20u |\n", "HW Emulation Poll Delay (secs)", hwEmuPollDelay);
        }
    }



    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);     
    }



    if (retval == XLNX_OK)
    {
        retval = pDataMover->GetStats(&stats);
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20u |\n", "Tx Response Count", stats.txResponseCount);
            pShell->printf("| %-35s | %20u |\n", "Rx Operation Count", stats.rxOp);
        }
    }


    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }


    if (retval == XLNX_OK)
    {
        retval = pDataMover->GetThreadStats(&threadStats);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20u |\n", "Thread Rx Packets", threadStats.numRxPackets);
            pShell->printf("| %-35s | %20u |\n", "Thread Tx Packets", threadStats.numTxPackets);
        }
    }



    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }





    if (retval == XLNX_OK)
    {
        retval = pDataMover->GetThrottleRate(&throttleRate);

        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20u |\n", "Throttle Rate (Clock Cycles)", throttleRate);
        }
    }


    if (retval == XLNX_OK)
    {
        retval = pDataMover->GetThrottleStats(&throttleStats);
        
        if (retval == XLNX_OK)
        {
            pShell->printf("| %-35s | %20u |\n", "Throttle Counter", throttleStats.throttleCounter);
            pShell->printf("| %-35s | %20u |\n", "Throttle Events", throttleStats.throttleEvents);
        }
    }



    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }

    if (retval == XLNX_OK)
    {
        pDataMover->GetDMAStats(&dmaH2CStats, &dmaC2HStats);

                
        pShell->printf("| %-35s | %20u |\n", "H2C Total Sync Operations",            dmaH2CStats.totalSyncOperations);
        pShell->printf("| %-35s | %20u |\n", "H2C Buffer Wrap Arounds",              dmaH2CStats.bufferWrapArounds);
        pShell->printf("| %-35s | %20u |\n", "H2C Transfer High Tide Bytes",         dmaH2CStats.transferHighTide);
        pShell->printf("| %-35s | %20" PRIu64 " |\n", "H2C Total Bytes Transferred", dmaH2CStats.totalBytesTransferred);

        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);

        pShell->printf("| %-35s | %20u |\n", "C2H Total Sync Operations",            dmaC2HStats.totalSyncOperations);
        pShell->printf("| %-35s | %20u |\n", "C2H Buffer Wrap Arounds",              dmaC2HStats.bufferWrapArounds);
        pShell->printf("| %-35s | %20u |\n", "C2H Transfer High Tide Bytes",         dmaC2HStats.transferHighTide);
        pShell->printf("| %-35s | %20" PRIu64 " |\n", "C2H Total Bytes Transferred", dmaC2HStats.totalBytesTransferred);
        
    }


    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
    }



    if (retval == XLNX_OK)
    {
        pDataMover->GetDMAChunkSize(&numElements);

        pShell->printf("| %-35s | %20u |\n", "DMA Chunk Size (elements, 0=ALL)", numElements);
    }





    if (retval == XLNX_OK)
    {
        pShell->printf("+-%.35s-+-%.20s-+\n", LINE_STRING, LINE_STRING);
        pShell->printf("\n\n");
    }



    if (retval != XLNX_OK)
    {
        pShell->printf("[ERROR] retval = %s (0x%08X)\n", OrderBookDataMover_ErrorCodeToString(retval), retval);
    }


    return retval;
}







static int OrderBookDataMover_Start(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    OrderBookDataMover* pDataMover = (OrderBookDataMover*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    retval = pDataMover->StartHWKernel();

    if (retval == XLNX_OK)
    {
        pShell->printf("OK\n");
    }
    else
    {
        pShell->printf("[ERROR] retval = %s (0x%08X)\n", OrderBookDataMover_ErrorCodeToString(retval), retval);
    }

    return retval;
}





static int OrderBookDataMover_ReadBuffer(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    OrderBookDataMover* pDataMover = (OrderBookDataMover*)pObjectData;
    uintptr_t pBuffer;
    uint32_t newHeadIndex;
    uint32_t newTailIndex;
    uint32_t newestElementIndex;
    uint32_t* pNewestElement;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);




    if (retval == XLNX_OK)
    {
        //DMA up any new data since last time....
        retval = pDataMover->SyncReadBuffer();

    }
    

    if (retval == XLNX_OK)
    {
        retval = pDataMover->GetSWRingReadBufferIndexes(&newHeadIndex, &newTailIndex);
    }


    if (retval == XLNX_OK)
    {
        pBuffer = (uintptr_t) pDataMover->GetReadBufferHostVirtualAddress();
    
        //Tail points to the next "free" element that will be written.
        //Therefore the newest data will be one element before that....

        if (newTailIndex > 0)
        {
            newestElementIndex = newTailIndex - 1;
        }
        else
        {
            newestElementIndex = OrderBookDataMover::RING_SIZE - 1;
        }

        pShell->printf("newestElementIndex = %u\n", newestElementIndex);
        pShell->printf("\n");



        pNewestElement = (uint32_t*)(pBuffer + (newestElementIndex * OrderBookDataMover::READ_ELEMENT_SIZE));

        for (uint32_t i = 0; i < OrderBookDataMover::READ_ELEMENT_SIZE / sizeof(uint32_t); i++)
        {
            pShell->printf("pNewestElement[%u] = 0x%08X\n", i, pNewestElement[i]);
        }
    }


    if (retval != XLNX_OK)
    {
        pShell->printf("[ERROR] retval = %s (0x%08X)\n", OrderBookDataMover_ErrorCodeToString(retval), retval);
    }

    return retval;
}








static int OrderBookDataMover_Timing(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    OrderBookDataMover* pDataMover = (OrderBookDataMover*)pObjectData;
    const uint32_t NUM_SECONDS_TO_RUN = 10;
    bool bThreadRunning = false;

    OrderBookDataMover::ThreadStats statsStart;
    OrderBookDataMover::ThreadStats statsEnd;

    double max;
    double min;
    double sum;
    uint32_t cnt;
    double avg;
    uint32_t cyclesPre;
    uint32_t cyclesPost;



    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

  


    retval = pDataMover->IsProcessingThreadRunning(&bThreadRunning);


    if (retval == XLNX_OK)
    {
        if (bThreadRunning == false)
        {
            pShell->printf("[ERROR] Pricing thread is not running...\n");
        }
    }



    if (bThreadRunning)
    {
        pShell->printf("Running for %u seconds...please wait...\n", NUM_SECONDS_TO_RUN);

        if (retval == XLNX_OK)
        {
            retval = pDataMover->StartLatencyCounters();
        }

        if (retval == XLNX_OK)
        {
            retval = pDataMover->GetThreadStats(&statsStart);
        }


        if (retval == XLNX_OK)
        {
            std::this_thread::sleep_for(chrono::seconds(NUM_SECONDS_TO_RUN));
        }

        if (retval == XLNX_OK)
        {
            retval = pDataMover->StopLatencyCounters();
        }


        if (retval == XLNX_OK)
        {
            retval = pDataMover->GetThreadStats(&statsEnd);
        }



     


        if (retval == XLNX_OK)
        {
            retval = pDataMover->GetLatencyStats(NUM_SECONDS_TO_RUN, &max, &min, &sum, &cnt, &cyclesPre, &cyclesPost);
        }

        if (retval == XLNX_OK)
        {
            if (cnt > 0)
            {
                avg = sum / (double)cnt;
            }
            else
            {
                avg = 0.0;
            }

            pShell->printf("RTT (us): max = %.2f, min = %.2f, sum = %.2f, cnt = %u, avg = %.2f, cyclesPre = %u, cyclesPost = %u\n", max, min, sum, cnt, avg, cyclesPre, cyclesPost);
            pShell->printf("num rx pkts = %u\n", statsEnd.numRxPackets - statsStart.numRxPackets);
            pShell->printf("num tx pkts = %u\n", statsEnd.numTxPackets - statsStart.numTxPackets);
            pShell->printf("OK\n");
        }

        if (retval != XLNX_OK)
        {
            pShell->printf("[ERROR] retval = %s (0x%08X)\n", OrderBookDataMover_ErrorCodeToString(retval), retval);
        }
    }

   
   


   
    

    return retval;
}





static int OrderBookDataMover_VerboseOn(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    OrderBookDataMover* pDataMover = (OrderBookDataMover*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    pDataMover->SetVerboseTracing(true);

    pShell->printf("OK\n");

    return retval;
}






static int OrderBookDataMover_VerboseOff(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    OrderBookDataMover* pDataMover = (OrderBookDataMover*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    pDataMover->SetVerboseTracing(false);

    pShell->printf("OK\n");

    return retval;
}





static int OrderBookDataMover_ThreadStart(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    OrderBookDataMover* pDataMover = (OrderBookDataMover*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    pDataMover->StartProcessingThread();

    pShell->printf("OK\n");

    return retval;
}




static int OrderBookDataMover_ThreadStop(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    OrderBookDataMover* pDataMover = (OrderBookDataMover*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    pDataMover->StopProcessingThread();

    pShell->printf("OK\n");

    return retval;
}






static int OrderBookDataMover_ThreadYield(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    OrderBookDataMover* pDataMover = (OrderBookDataMover*)pObjectData;
    bool bOKToContinue = true;
    bool bYield = false;

    if (argc != 2)
    {
        pShell->printf("Usage: %s <bool>\n", argv[0]);
        bOKToContinue = false;
    }


    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseBool(argv[1], &bYield);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse bool parameter\n");
        }
    }

    if (bOKToContinue)
    {
        retval = pDataMover->SetThreadYield(bYield);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] retval = %s (0x%08X)\n", OrderBookDataMover_ErrorCodeToString(retval), retval);
        }
    }


    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }

    return retval;
}







static int OrderBookDataMover_SetThrottle(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    OrderBookDataMover* pDataMover = (OrderBookDataMover*)pObjectData;
    bool bOKToContinue = true;
    uint32_t throttleRateClockCycles;

    if (argc != 2)
    {
        pShell->printf("Usage: %s <clkcycles>\n", argv[0]);
        bOKToContinue = false;
    }


    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[1], &throttleRateClockCycles);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse throttle rate (clock cycles) parameter\n");
        }
    }



    if (bOKToContinue)
    {
        retval = pDataMover->SetThrottleRate(throttleRateClockCycles);

        if (retval == XLNX_OK)
        {
            pShell->printf("OK\n");
        }
        else
        {
            pShell->printf("[ERROR] retval = %s (0x%08X)\n", OrderBookDataMover_ErrorCodeToString(retval), retval);
        }
    }

    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }

    return retval;
}






static int OrderBookDataMover_ResetDMAStats(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    OrderBookDataMover* pDataMover = (OrderBookDataMover*)pObjectData;

    XLNX_UNUSED_ARG(argc);
    XLNX_UNUSED_ARG(argv);

    pDataMover->ResetDMAStats();

    pShell->printf("OK\n");

    return retval;
}


static int OrderBookDataMover_SetDMAChunkSize(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    OrderBookDataMover* pDataMover = (OrderBookDataMover*)pObjectData;
    bool bOKToContinue = true;
    uint32_t numElements;

    if (argc != 2)
    {
        pShell->printf("Usage: %s <numelements>\n", argv[0]);
        bOKToContinue = false;
    }


    if (bOKToContinue)
    {
        bOKToContinue = pShell->parseUInt32(argv[1], &numElements);
        if (bOKToContinue == false)
        {
            pShell->printf("[ERROR] Failed to parse numelements parameter\n");
        }
    }

    if (bOKToContinue)
    {
        pDataMover->SetDMAChunkSize(numElements);
        pShell->printf("OK\n");
    }


    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }

    return retval;
}








static int OrderBookDataMover_SetHWEmuPollDelay(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
    int retval = XLNX_OK;
    OrderBookDataMover* pDataMover = (OrderBookDataMover*)pObjectData;
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
        pDataMover->SetHWEmulationPollDelay(delaySeconds);
        pShell->printf("OK\n");
    }


    if (bOKToContinue == false)
    {
        retval = Shell::COMMAND_PARSING_ERROR;
    }

    return retval;
}





CommandTableElement XLNX_ORDER_BOOK_DATA_MOVER_COMMAND_TABLE[] =
{
    {"getstatus",	        OrderBookDataMover_GetStatus,	        "",			                "Get block status"	                            },
    {"start",               OrderBookDataMover_Start,               "",                         "Starts the block running"                      },
    {"readbuffer",          OrderBookDataMover_ReadBuffer,          "",                         "Syncs and reads the memory mapped buffer"      },
    {"timing",              OrderBookDataMover_Timing,              "",                         "Round trip timing test"                        },
    {"verboseon",           OrderBookDataMover_VerboseOn,           "",                         "Turn on verbose debug output"                  },
    {"verboseoff",          OrderBookDataMover_VerboseOff,          "",                         "Turn off verbose debug output"                 },
    {"threadstart",         OrderBookDataMover_ThreadStart,         "",                         "Start the pricing engine thread"               },
    {"threadstop",          OrderBookDataMover_ThreadStop,          "",                         "Stop the princing engine thread"               },
    {"threadyield",         OrderBookDataMover_ThreadYield,         "<bool>",                   "Controls thread yielding to other threads"     },
    {"setthrottle",         OrderBookDataMover_SetThrottle,         "<clkcycles>",              "Set the HW throttle rate on reading H2C ring"  },
    {"resetdmastats",       OrderBookDataMover_ResetDMAStats,       "",                         "Reset DMA stats counters"                      },
    {"setdmachunksize",     OrderBookDataMover_SetDMAChunkSize,     "<numelements>",            "Sets the number of elements DMA'd at a time"   },
    {"sethwemupolldelay",   OrderBookDataMover_SetHWEmuPollDelay,   "<seconds>",                "Sets a poll delay - only used in HW emulation" }  
};


const uint32_t XLNX_ORDER_BOOK_DATA_MOVER_COMMAND_TABLE_LENGTH = (uint32_t)(sizeof(XLNX_ORDER_BOOK_DATA_MOVER_COMMAND_TABLE) / sizeof(XLNX_ORDER_BOOK_DATA_MOVER_COMMAND_TABLE[0]));

