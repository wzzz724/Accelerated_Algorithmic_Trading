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


#ifndef XLNX_NETWORK_TAP_H
#define XLNX_NETWORK_TAP_H


#include <cstdint>
#include <cstdbool>
#include <thread>

#include "xlnx_device_interface.h"

#include "xlnx_network_tap_error_codes.h"





namespace XLNX
{




class NetworkTap
{

public:
    NetworkTap();
    virtual ~NetworkTap();


public:
    uint32_t Initialise(DeviceInterface* pDeviceInterface, const char* cuName);
    uint32_t Uninitialise(void);




public:
    uint32_t Start(void);
    uint32_t Stop(void);
    uint32_t IsRunning(bool* pbIsRunning);




public:
    void IsInitialised(bool* pbIsInitialised);

    uint32_t GetCUIndex(uint32_t* pCUIndex);
    uint32_t GetCUAddress(uint64_t* pCUAddress);



protected:
    uint32_t CheckIsInitialised(void);
   






protected:
    uint32_t ReadReg32(uint64_t offset, uint32_t* value);
    uint32_t WriteReg32(uint64_t offset, uint32_t value);
    uint32_t WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask);
    uint32_t BlockReadReg32(uint64_t offset, uint32_t* buffer, uint32_t numWords);






protected:
    uint32_t m_initialisedMagicNumber;
    uint64_t m_cuAddress;
    uint32_t m_cuIndex;
    DeviceInterface* m_pDeviceInterface;



}; //class NetworkTap



} //namespace XLNX



#endif //XLNX_NETWORK_TAP_H