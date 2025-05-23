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


#include "xlnx_network_capture.h"
#include "xlnx_network_capture_internal.h"
using namespace XLNX;


uint32_t NetworkCapture::StartThread()
{
    uint32_t retval = XLNX_OK;
    bool bAlreadyRunning = false;


    retval = IsThreadRunning(&bAlreadyRunning);


    if (retval == XLNX_OK)
    {
        if (bAlreadyRunning == false)
        {
            m_bKeepRunning = true;

            m_thread = std::thread(&NetworkCapture::ThreadFunc, this);
        }
    }

    return retval;
}





uint32_t NetworkCapture::StopThread(void)
{
    uint32_t retval = XLNX_OK;

    m_bKeepRunning = false;

    if (m_thread.joinable())
    {
        m_thread.join();
    }

    return retval;
}





uint32_t NetworkCapture::IsThreadRunning(bool* pbIsRunning)
{
    uint32_t retval = XLNX_OK;

    *pbIsRunning = m_thread.joinable();

    return retval;
}




uint32_t NetworkCapture::SetThreadYield(bool bEnable)
{
    uint32_t retval = XLNX_OK;

    m_bYield = bEnable;

    return retval;
}




uint32_t NetworkCapture::GetThreadYield(bool* pbEnabled)
{
    uint32_t retval = XLNX_OK;

    *pbEnabled = m_bYield;

    return retval;
}



uint32_t NetworkCapture::SetPollRate(uint32_t milliseconds)
{
    uint32_t retval = XLNX_OK;

    m_pollRateMilliseconds = milliseconds;

    return retval;
}




uint32_t NetworkCapture::GetPollRate(uint32_t* pMilliseconds)
{
    uint32_t retval = XLNX_OK;

    *pMilliseconds = m_pollRateMilliseconds;

    return retval;
}




uint32_t NetworkCapture::GetTailChunkIndex(uint32_t* pChunkIndex)
{
    uint32_t retval = XLNX_OK;
    uint32_t tailPointer;
    uint32_t byteOffset;

    retval = GetTailPointer(&tailPointer);


    if (retval == XLNX_OK)
    {
        //NOTE - the HW tail pointer increments for every 64 bytes written to card RAM.
        //       This means that if a packet is in the middle of being written to card RAM,
        //       the pointer could be mid-way through a chunk.
        //       
        //       We want to disregard any partially written chunks.  This can be taken care
        //       of by performing an INTEGER DIVISION... 

        byteOffset = tailPointer * XLNX_NETWORK_CAPTURE_TAIL_POINTER_MULTIPLIER;
        *pChunkIndex = byteOffset / CHUNK_SIZE;
    }
    

    return retval;
}






uint32_t NetworkCapture::WritePacketsToPCAPFile(uint32_t chunkStartIndex, uint32_t numChunks)
{
    uint32_t retval = XLNX_OK;
    PacketMetadata metadata;
    uint32_t* pWordBuffer;
    uint32_t chunkStartByteOffset;
    uint32_t metadataByteOffset;
    uint8_t* pPacketData;
    uint64_t timestampClockCycles;
    uint64_t timestampNanoseconds;



    pWordBuffer = (uint32_t*)m_bufferHostVirtualAddress;

    for (uint32_t i = 0; i < numChunks; i++)
    {
        chunkStartByteOffset    = ((chunkStartIndex + i) * CHUNK_SIZE);
        metadataByteOffset      = (chunkStartByteOffset + XLNX_NETWORK_CAPTURE_METADATA_OFFSET);

        metadata.packetLengthInBytes    = pWordBuffer[(metadataByteOffset / 4) + 0];
        metadata.packetNum              = pWordBuffer[(metadataByteOffset / 4) + 1];
        metadata.timestampLower         = pWordBuffer[(metadataByteOffset / 4) + 2]; 
        metadata.timestampUpper         = pWordBuffer[(metadataByteOffset / 4) + 3];

        timestampClockCycles = (((uint64_t)metadata.timestampUpper) << 32) | metadata.timestampLower;

        //The current HW timestamp counter increments every 4 clock cycles
        //so we need to multiply by 4 to get the correct time.
        timestampClockCycles = timestampClockCycles * TIMESTAMP_CLOCKS_MULTIPLIER;

        ConvertClockCyclesToNanoseconds(timestampClockCycles, &timestampNanoseconds);

        pPacketData = (uint8_t*)&pWordBuffer[chunkStartByteOffset / 4];

        retval = m_pcapWriter.WritePacket(pPacketData, metadata.packetLengthInBytes, timestampNanoseconds);

        if (retval == XLNX_OK)
        {
            m_numCapturedPackets++;
        }

        if (retval != XLNX_OK)
        {
            break; //out of loop
        }
    }



    return retval;
}




void NetworkCapture::ThreadFunc(void)
{
    uint32_t retval = XLNX_OK;
    PCAPWriter pcapWriter;
    uint32_t lastChunkIndex;
    uint32_t currentChunkIndex;
    uint32_t numChunksToTransfer;
    uint32_t numBytesToTransfer;
    uint32_t startOffset;

#ifdef XCL_EMULATION_MODE
    std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> lastPollTime = currentTime;
#endif


    m_numCapturedPackets = 0;


    //NOTE - chunk indexes refer to the "next free" location that will be written to by the HW...
    retval = GetTailChunkIndex(&lastChunkIndex);


    
    if (retval == XLNX_OK)
    {
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




            retval = GetTailChunkIndex(&currentChunkIndex);

            if (retval == XLNX_OK)
            {
                if (currentChunkIndex != lastChunkIndex)
                {
                    //we have new packets...


                    if (currentChunkIndex > lastChunkIndex)
                    {
                        numChunksToTransfer = currentChunkIndex - lastChunkIndex;


                        //NOTE - Only need to do a sync if kernel is connected to CARD-RAM
                        if (m_bUsingHostBank == false)
                        {
                            startOffset = lastChunkIndex * CHUNK_SIZE;
                            numBytesToTransfer = numChunksToTransfer * CHUNK_SIZE;

                            retval = m_pDeviceInterface->SyncBuffer(m_pBufferDescriptor, DeviceInterface::SyncDirection::FROM_DEVICE, numBytesToTransfer, startOffset);
                        }
                 

                        if (retval == XLNX_OK)
                        {
                            retval = WritePacketsToPCAPFile(lastChunkIndex, numChunksToTransfer);
                        }



                    }
                    else if (currentChunkIndex < lastChunkIndex)
                    {
                        //the buffer has wrapped around...we may need to do 2 transfers...


                        //first transfer is from last index to end of buffer....
                        numChunksToTransfer = NUM_CHUNKS - lastChunkIndex;


                        //NOTE - Only need to do a sync if kernel is connected to CARD-RAM
                        if (m_bUsingHostBank == false)
                        {
                            startOffset = lastChunkIndex * CHUNK_SIZE;
                            numBytesToTransfer = numChunksToTransfer * CHUNK_SIZE;

                            retval = m_pDeviceInterface->SyncBuffer(m_pBufferDescriptor, DeviceInterface::SyncDirection::FROM_DEVICE, numBytesToTransfer, startOffset);
                        }


                        
                        if (retval == XLNX_OK)
                        {
                            retval = WritePacketsToPCAPFile(lastChunkIndex, numChunksToTransfer);
                        }





                        if (retval == XLNX_OK)
                        {
                            //second transfer if from start of buffer to current index...

                            numChunksToTransfer = currentChunkIndex;

                            if (numBytesToTransfer > 0)
                            {

                                //NOTE - Only need to do a sync if kernel is connected to CARD-RAM
                                if (m_bUsingHostBank == false)
                                {
                                    startOffset = 0;
                                    numBytesToTransfer = numChunksToTransfer * CHUNK_SIZE;

                                    retval = m_pDeviceInterface->SyncBuffer(m_pBufferDescriptor, DeviceInterface::SyncDirection::FROM_DEVICE, numBytesToTransfer, startOffset);
                                }

                                if (retval == XLNX_OK)
                                {
                                    retval = WritePacketsToPCAPFile(0, numChunksToTransfer);
                                }
                            }
                        }

                    }



                    //finally update our position index so we know where to start next time...
                    lastChunkIndex = currentChunkIndex;
                    
                }
                
            }


            if (m_pollRateMilliseconds > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(m_pollRateMilliseconds));
            }


            if (m_bYield) //yield to play nice with other threads/processes on system....
            {
                std::this_thread::yield();
            }


        } //end while m_bKeepRunning
    }
}   





