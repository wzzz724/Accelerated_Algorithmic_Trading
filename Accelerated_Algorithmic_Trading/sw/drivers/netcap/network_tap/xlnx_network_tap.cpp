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


#include "xlnx_network_tap.h"
#include "xlnx_network_tap_address_map.h"
#include "xlnx_network_tap_internal.h"

using namespace XLNX;


NetworkTap::NetworkTap()
{
    m_pDeviceInterface          = nullptr;
    m_cuAddress                 = 0;
    m_cuIndex                   = 0;
    m_initialisedMagicNumber    = 0;


}



NetworkTap::~NetworkTap()
{
    Stop();
}






uint32_t NetworkTap::Initialise(DeviceInterface* pDeviceInterface, const char* cuName)
{
    uint32_t retval = XLNX_OK;
   


    m_initialisedMagicNumber = 0;	//will be set to magic number if we successfully initialise...

    m_pDeviceInterface = pDeviceInterface;


   
    retval = m_pDeviceInterface->GetCUAddress(cuName, &m_cuAddress);

    if (retval != XLNX_OK)
    {
        retval = XLNX_NETWORK_TAP_ERROR_CU_NAME_NOT_FOUND;
    }



    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->GetCUIndex(cuName, &m_cuIndex);

        if (retval != XLNX_OK)
        {
            retval = XLNX_NETWORK_TAP_ERROR_CU_NAME_NOT_FOUND;
        }
    }


    if (retval == XLNX_OK)
    {
        m_initialisedMagicNumber = XLNX_NETWORK_TAP_INITIALISED_MAGIC_NUMBER;
    }

    return retval;
}




uint32_t NetworkTap::Uninitialise(void)
{
    uint32_t retval = XLNX_OK;

    retval = Stop();

    return retval;
}





uint32_t NetworkTap::Start(void)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;
    uint32_t value;
    uint32_t mask;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        address = m_cuAddress + XLNX_NETWORK_TAP_ENABLE_PROCESSING_OFFSET;

        value = 1 << 16;
        mask = value;


        retval = m_pDeviceInterface->WriteRegWithMask32(address, value, mask);
    }

    return retval;
}





uint32_t NetworkTap::Stop(void)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;
    uint32_t value;
    uint32_t mask;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        address = m_cuAddress + XLNX_NETWORK_TAP_ENABLE_PROCESSING_OFFSET;

        value = 0;
        mask = 1 << 16;


        retval = m_pDeviceInterface->WriteRegWithMask32(address, value, mask);
    }

    return retval;
}






uint32_t NetworkTap::IsRunning(bool* pbIsRunning)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;
    uint32_t value;


    address = m_cuAddress + XLNX_NETWORK_TAP_ENABLE_PROCESSING_OFFSET;

    retval = m_pDeviceInterface->ReadReg32(address, &value);

    if (retval == XLNX_OK)
    {
        *pbIsRunning = (bool)(value & (1 << 16));
    }


    return retval;
}







void NetworkTap::IsInitialised(bool* pbIsInitialised)
{
    if (CheckIsInitialised() == XLNX_OK)
    {
        *pbIsInitialised = true;
    }
    else
    {
        *pbIsInitialised = false;
    }
}







uint32_t NetworkTap::GetCUIndex(uint32_t* pCUIndex)
{
    uint32_t retval = XLNX_OK;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        *pCUIndex = m_cuIndex;
    }

    return retval;
}




uint32_t NetworkTap::GetCUAddress(uint64_t* pCUAddress)
{
    uint32_t retval = XLNX_OK;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        *pCUAddress = m_cuAddress;
    }

    return retval;
}






uint32_t NetworkTap::ReadReg32(uint64_t offset, uint32_t* value)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->ReadReg32(address, value);




    if (retval != XLNX_OK)
    {
        retval = XLNX_NETWORK_TAP_ERROR_IO_FAILED;
    }

    return retval;
}






uint32_t NetworkTap::WriteReg32(uint64_t offset, uint32_t value)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->WriteReg32(address, value);


    if (retval != XLNX_OK)
    {
        retval = XLNX_NETWORK_TAP_ERROR_IO_FAILED;
    }

    return retval;
}




uint32_t NetworkTap::WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->WriteRegWithMask32(address, value, mask);

    if (retval != XLNX_OK)
    {
        retval = XLNX_NETWORK_TAP_ERROR_IO_FAILED;
    }

    return retval;
}






uint32_t NetworkTap::BlockReadReg32(uint64_t offset, uint32_t* buffer, uint32_t numWords)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->BlockReadReg32(address, buffer, numWords);


    if (retval != XLNX_OK)
    {
        retval = XLNX_NETWORK_TAP_ERROR_IO_FAILED;
    }

    return retval;
}




