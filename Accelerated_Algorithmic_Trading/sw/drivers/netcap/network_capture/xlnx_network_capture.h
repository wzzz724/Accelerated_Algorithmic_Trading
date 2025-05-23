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


#ifndef XLNX_NETWORK_CAPTURE_H
#define XLNX_NETWORK_CAPTURE_H


#include <cstdint>
#include <cstdbool>
#include <thread>

#include "xlnx_device_interface.h"

#include "xlnx_network_capture_error_codes.h"
#include "xlnx_network_capture_pcap_writer.h"




namespace XLNX
{




class NetworkCapture
{

public:
    NetworkCapture();
    virtual ~NetworkCapture();


public:
    uint32_t Initialise(DeviceInterface* pDeviceInterface, const char* cuName);
    uint32_t Uninitialise(void);


public:
    
    uint32_t IsHWKernelRunning(bool* pbIsRunning);


public:
    uint32_t GetTailPointer(uint32_t* pTailPointer);


public:
    uint32_t Start(char* filepath);
    uint32_t Stop(void);
    uint32_t IsRunning(bool* pbIsRunning);
    uint32_t GetNumCapturedPackets(uint32_t* pNumPackets);
   

public: //Thread Control

    //The following function can be used to control if the processing thread
    //yields execution to other thread/processes on the system
    uint32_t SetThreadYield(bool bEnable);
    uint32_t GetThreadYield(bool* pbEnabled);
    uint32_t SetPollRate(uint32_t milliseconds);
    uint32_t GetPollRate(uint32_t* pMilliseconds);


    //The following function can be used to control the a delay that gets applied to the polling thread
       //when running in HW emulation mode.  Because HW emulation runs much slower than real HW, by introducing
       //a delay, we can reduce the load on the simulation.
    static const uint32_t HW_EMU_POLL_DELAY_DEFAULT_SECONDS = 30;
    void SetHWEmulationPollDelay(uint32_t delaySeconds);
    void GetHWEmulationPollDelay(uint32_t* pDelaySeconds);


public:
    void IsInitialised(bool* pbIsInitialised);

    uint32_t GetCUIndex(uint32_t* pCUIndex);
    uint32_t GetCUAddress(uint64_t* pCUAddress);
    uint32_t GetBufferCUMemTopologyIndex(uint32_t* pMemTopologyIndex);



protected:
    uint32_t CheckIsInitialised(void);
    uint32_t CheckIsRunning(void);
    uint32_t CheckIsNotRunning(void);




public:
    uint32_t SetupBufferIfNecessary(void);
    void* GetBufferHostVirtualAddress(void);
  
    static const uint64_t INVALID_HW_BUFFER_ADDRESS = 0xFFFFFFFF;
    uint64_t GetBufferHWAddress(void);

    uint32_t StartHWKernel(void);




public: //Thread
   
    







protected: //Thread
    uint32_t StartThread();
    uint32_t StopThread(void);
    uint32_t IsThreadRunning(bool* pbIsRunning);
    void ThreadFunc(void);
    uint32_t GetTailChunkIndex(uint32_t* pChunkIndex);
    uint32_t WritePacketsToPCAPFile(uint32_t chunkStartIndex, uint32_t numChunks);







protected:
    uint32_t ReadReg32(uint64_t offset, uint32_t* value);
    uint32_t WriteReg32(uint64_t offset, uint32_t value);
    uint32_t WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask);
    uint32_t BlockReadReg32(uint64_t offset, uint32_t* buffer, uint32_t numWords);



protected:
    uint32_t ConvertClockCyclesToNanoseconds(uint64_t clockCycles, uint64_t* pNanoseconds);


protected:
    static const uint32_t KERNEL_MEMORY_ARGUMENT_BUFFER_INDEX = 2;

    static const uint32_t CHUNK_SIZE = 0x800;   //A chunk contains packet data + metadata

    static const uint32_t NUM_CHUNKS = 64;      // This needs to match the ring size defined in HW
                                                //TODO probably needs to be larger...

    static const uint32_t BUFFER_SIZE = CHUNK_SIZE * NUM_CHUNKS;


    //The current HW timestamp counter increments every 4 clock cycles
    //so we need to multiply by 4 to get the correct time.
    static const uint32_t TIMESTAMP_CLOCKS_MULTIPLIER = 4;


    uint32_t SetupBufferObjects(void);
    uint32_t CleanupBufferObjects(void);


    uint32_t SetupBufferObjectsFromHostBank(void);
    uint32_t SetupBufferObjectsFromCardRAM(void);
    uint32_t SetupBufferDetails(void);

    uint32_t CleanupBufferObjectsFromHostBank(void);
    uint32_t CleanupBufferObjectsFromCardRAM(void);

    uint32_t SetHWBufferAddress(uint64_t address);




protected:
    uint32_t m_initialisedMagicNumber;
    uint64_t m_cuAddress;
    uint32_t m_cuIndex;
    DeviceInterface* m_pDeviceInterface;



protected:
    bool m_bUsingHostBank;
    bool m_bNeedToSetupBuffer = true;
    uint32_t m_bufferMemTopologyIndex;
    uint32_t m_hostBankMemTopologyIndex;
    BufferDescriptor* m_pBufferDescriptor;
    void* m_bufferHostVirtualAddress;
    uint64_t m_bufferHWAddress;


protected:
    uint32_t m_clockFrequencyMHz;


protected:
    PCAPWriter m_pcapWriter;
    std::thread m_thread;
    bool m_bKeepRunning;
    bool m_bYield;
    uint32_t m_numCapturedPackets;
    uint32_t m_pollRateMilliseconds;

    uint32_t m_hwEmulationPollDelaySeconds;


}; //class NetworkCapture



} //namespace XLNX



#endif //XLNX_NETWORK_CAPTURE_H