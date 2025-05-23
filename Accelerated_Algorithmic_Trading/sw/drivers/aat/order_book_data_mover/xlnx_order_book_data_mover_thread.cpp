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

#include <cstring> //for memset

#include "xlnx_order_book_data_mover.h"
#include "xlnx_order_book_data_mover_address_map.h"
#include "xlnx_order_book_data_mover_error_codes.h"
#include "xlnx_order_book_data_mover_host_pricing_interface.h"

#include "xlnx_hw_device_interface.h"

using namespace XLNX;







static void UnpackResponse(orderBookResponse_t* dst, uint8_t* src)
{
    memcpy(&dst->timestamp,         &src[121],  7); //NOTE - HW timestamp is currently only 56-bits wide... 
    memcpy(&dst->symbolIndex,       &src[120],  1);
    memcpy(&dst->bidCount[0],       &src[100],  20);
    memcpy(&dst->bidPrice[0],       &src[80],   20);
    memcpy(&dst->bidQuantity[0],    &src[60],   20);
    memcpy(&dst->askCount[0],       &src[40],   20);
    memcpy(&dst->askPrice[0],       &src[20],   20);
    memcpy(&dst->askQuantity[0],    &src[0],    20);
}





static void PackOperation(uint8_t* dst, orderEntryOperation_t* src)
{
    memcpy(&dst[0],     &src->direction,    1);
    memcpy(&dst[1],     &src->price,        4);
    memcpy(&dst[5],     &src->quantity,     4);
    memcpy(&dst[9],     &src->orderId,      4);
    memcpy(&dst[13],    &src->symbolIndex,  1);
    memcpy(&dst[14],    &src->opCode,       1);
    memcpy(&dst[15],    &src->timestamp,    8);
}








uint32_t OrderBookDataMover::StartProcessingThread(void)
{
	uint32_t retval = XLNX_OK;
    bool bAlreadyRunning = false;

    
    retval = IsProcessingThreadRunning(&bAlreadyRunning);
    

    if (retval == XLNX_OK)
    {
        if (bAlreadyRunning == false)
        {
            m_bKeepRunning = true;
            m_previousTailIndex = 0;

            m_processingThread = std::thread(&OrderBookDataMover::ThreadFunc, this);
        }
    }

	

	return retval;
}






uint32_t OrderBookDataMover::StopProcessingThread(void)
{
	uint32_t retval = XLNX_OK;

	m_bKeepRunning = false;

	if (m_processingThread.joinable())
	{
		m_processingThread.join();
	}

	return retval;
}








uint32_t OrderBookDataMover::IsProcessingThreadRunning(bool* pbIsRunning)
{
    uint32_t retval = XLNX_OK;

    *pbIsRunning = m_processingThread.joinable();
   
    return retval;
}





uint32_t OrderBookDataMover::SetThreadYield(bool bEnable)
{
    uint32_t retval = XLNX_OK;

    m_bYield = bEnable;

    return retval;
}




uint32_t OrderBookDataMover::GetThreadYield(bool* pbEnabled)
{
    uint32_t retval = XLNX_OK;

    *pbEnabled = m_bYield;

    return retval;
}





void OrderBookDataMover::ThreadFunc(void)
{
    uint32_t retval = XLNX_OK;
    orderBookResponse_t response;
    orderEntryOperation_t operation;
    bool bOperationValid;

    uint32_t newHeadIndex;
    uint32_t newTailIndex;
    uint32_t numElementsToProcess;

    uintptr_t pBuffer;
    uint32_t startIndex;
    uint32_t elementIndex;
    uint8_t* pElement = nullptr;

#ifdef XCL_EMULATION_MODE
    std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> lastPollTime = currentTime;
#endif



    memset(&m_threadStats, 0, sizeof(m_threadStats));

	while (m_bKeepRunning)
	{

        // The following code only runs in HW emulation mode. Its purpose is to reduce the rate at which the thread polls
        // the HW for new data in order to try to reduce the load on the simulator. Since the poll delay could be several
        // tens of seconds (default = 30 seconds), we do not want to use a normal "sleep" as this would block the thread
        // for this duration - this would cause a delay when the process is exiting (when the sim completes).
#ifdef XCL_EMULATION_MODE
        if (m_hwEmulationPollDelaySeconds > 0)
        {
            currentTime = std::chrono::system_clock::now();

            uint64_t diffSeconds = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastPollTime).count();

            if (diffSeconds >= m_hwEmulationPollDelaySeconds)
            {
                lastPollTime = currentTime;
            }       
            else
            {
                //jump back to the top of the loop
                continue;
            }
        }
#endif



        //DMA up any new data since last time....
        retval = SyncReadBuffer();

        if (retval == XLNX_OK)
        {
            retval = GetSWRingReadBufferIndexes(&newHeadIndex, &newTailIndex);
        }


        if (retval == XLNX_OK)
        {
            if (newTailIndex == m_previousTailIndex)
            {
                //nothing to do...
                numElementsToProcess = 0;
            }
            else
            {
                if (m_previousTailIndex < newTailIndex)
                {
                    numElementsToProcess = newTailIndex - m_previousTailIndex;
                }
                else
                {
                    //assume buffer has wrapped around...
                    numElementsToProcess = OrderBookDataMover::RING_SIZE - m_previousTailIndex;
                    numElementsToProcess += newTailIndex;
                }
            }




            if( numElementsToProcess > 0)
            {
                startIndex = m_previousTailIndex;
                pBuffer = (uintptr_t)GetReadBufferHostVirtualAddress();


                //loop round, processing each of the new elements....
                for (uint32_t i = 0; i < numElementsToProcess; i++)
                {
                    elementIndex = startIndex + i;

                    if (elementIndex >= OrderBookDataMover::RING_SIZE)
                    {
                        elementIndex = elementIndex - OrderBookDataMover::RING_SIZE;
                    }

                    pElement = (uint8_t*)(pBuffer + (elementIndex * OrderBookDataMover::READ_ELEMENT_SIZE));


                    m_threadStats.numRxPackets++;

                    /* put through pricing engine */
                    UnpackResponse(&response, pElement);
                    bOperationValid = ProcessPricingData(&response, &operation);


                    // If the pricing engine determined that a new operation must be issued....
                    if (bOperationValid)
                    {          
                        WriteData(&operation);
                        m_threadStats.numTxPackets++;
                    }
                }

                //finally update our cached tail index...
                m_previousTailIndex = newTailIndex;

            }
    
        }


        if (m_bYield) //yield to play nice with other threads/processes on system....
        {
            std::this_thread::yield();

        }
   
	} //end while (m_bKeepRunning)
}







bool OrderBookDataMover::ProcessPricingData(orderBookResponse_t* response, orderEntryOperation_t* operation)
{
    bool bOperationValid = false;

    if (m_pPricingInterface != nullptr)
    {
        bOperationValid = m_pPricingInterface->PricingProcess(response, operation);
    }

    return bOperationValid;
}







uint32_t OrderBookDataMover::WriteData(orderEntryOperation_t* src)
{
    uint32_t retval = XLNX_OK;
    uint32_t newHeadIndex;
    uint32_t newTailIndex;
    uint32_t newestElementIndex;
    uint32_t* pNewestElement;
    uintptr_t pBuffer;

    retval = SetupBuffersIfNecessary();

    if (retval == XLNX_OK)
    {
        //Tail points to the next "free" element that will be written.
        retval = GetSWRingWriteBufferIndexes(&newHeadIndex, &newTailIndex);
    }

    if (retval == XLNX_OK)
    {
        newestElementIndex = newTailIndex;

        pBuffer = (uintptr_t)GetWriteBufferHostVirtualAddress();
        pNewestElement = (uint32_t*)(pBuffer + (newestElementIndex * OrderBookDataMover::WRITE_ELEMENT_SIZE));

        PackOperation((uint8_t*)pNewestElement, src);


        //Since we have written an element into the SW ring, we need to move the index on....
        newestElementIndex++;
        if (newestElementIndex >= OrderBookDataMover::RING_SIZE)
        {
            newestElementIndex = 0;
        }

        SetSWWriteTailIndex(newestElementIndex);

        //sync the SW ring with the HW ring...
        retval = SyncWriteBuffer();
    }

    return retval;

}








uint32_t OrderBookDataMover::GetThreadStats(ThreadStats* pStats)
{
    memcpy(pStats, &m_threadStats, sizeof(m_threadStats));

    return XLNX_OK;
}



void OrderBookDataMover::SetVerboseTracing(bool bEnabled)
{
   
    if (m_pPricingInterface != nullptr)
    {
        m_pPricingInterface->SetVerboseTracing(bEnabled);
    }
  
}

