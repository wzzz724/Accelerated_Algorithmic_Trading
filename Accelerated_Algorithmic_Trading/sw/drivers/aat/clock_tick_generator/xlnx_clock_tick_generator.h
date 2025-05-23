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

#ifndef XLNX_CLOCK_TICK_GENERATOR_H
#define XLNX_CLOCK_TICK_GENERATOR_H

#include <cstdint>

#include "xlnx_device_interface.h"

#include "xlnx_clock_tick_generator_error_codes.h"


namespace XLNX
{

class ClockTickGenerator
{

public:
    ClockTickGenerator();
    virtual ~ClockTickGenerator();


public:
    uint32_t Initialise(DeviceInterface* pDeviceInterface, const char* cuName);



public:
    static const uint32_t NUM_SUPPORTED_TICK_STREAMS = 4;


public:
    uint32_t SetTickEnabled(uint32_t streamIndex, bool bEnabled);
    uint32_t GetTickEnabled(uint32_t streamIndex, bool* pbEnabled);

    uint32_t SetTickInterval(uint32_t streamIndex, uint32_t microseconds);
    uint32_t GetTickInterval(uint32_t streamIndex, uint32_t* pMicroseconds);

   


public:

    typedef struct 
    {
        uint32_t tickCount; //free-running counter

        uint32_t numTickEventsFired[NUM_SUPPORTED_TICK_STREAMS];

    }Stats;

    uint32_t GetStats(Stats* pStats);




public:
    void IsInitialised(bool* pbIsInitialised);
    uint32_t GetCUIndex(uint32_t* pCUIndex);
    uint32_t GetCUAddress(uint64_t* pCUAddress);
    uint32_t GetClockFrequency(uint32_t* pFrequencyMHz);
    uint32_t GetTickIntervalClockCycles(uint32_t streamIndex, uint32_t* pClockCycles);

public:
    uint32_t CheckIsInitialised(void);
    uint32_t CheckStreamIndex(uint32_t streamIndex);



protected: //IO
    uint32_t ReadReg32(uint64_t offset, uint32_t* value);
    uint32_t WriteReg32(uint64_t offset, uint32_t value);
    uint32_t WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask);
    uint32_t BlockReadReg32(uint64_t offset, uint32_t* buffer, uint32_t numWords);

    uint32_t BlockReadMem32(uint64_t address, uint32_t* buffer, uint32_t numWords);



protected: 
    uint32_t ConvertMicrosecondsToClockCycles(uint32_t microseconds, uint32_t* pClockCycles);
    uint32_t ConvertClockCyclesToMicroseconds(uint32_t clockCycles, uint32_t* pMicroseconds);


protected:
    uint32_t m_initialisedMagicNumber;

    uint32_t m_cuIndex;
    uint64_t m_cuAddress;

    DeviceInterface* m_pDeviceInterface;

    uint32_t m_clockFrequencyMHz;

}; //class ClockTickGenerator


} //namespace XLNX





#endif //XLNX_CLOCK_TICK_GENERATOR_H


