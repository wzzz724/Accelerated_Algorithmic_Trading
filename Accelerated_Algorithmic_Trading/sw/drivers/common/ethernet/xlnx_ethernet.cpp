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


#include "xlnx_ethernet.h"
#include "xlnx_ethernet_address_map.h"
#include "xlnx_ethernet_error_codes.h"
#include "xlnx_ethernet_types.h"

using namespace XLNX;



static const uint32_t IS_INITIALISED_MAGIC_NUMBER = 0x93BAC34C;





Ethernet::Ethernet()
{
	m_pDeviceInterface			= nullptr;
	m_cuIndex					= 0xFFFFFFFF;
	m_cuAddress					= 0;
	m_initialisedMagicNumber	= 0;
}




Ethernet::~Ethernet()
{

}







uint32_t Ethernet::Initialise(DeviceInterface* pDeviceInterface, const char* cuName)
{
	uint32_t retval = XLNX_OK;

	m_initialisedMagicNumber = 0;	//will be set to magic number if we successfully initialise...

	m_pDeviceInterface = pDeviceInterface;


	retval = m_pDeviceInterface->GetCUAddress(cuName, &m_cuAddress);

	if (retval != XLNX_OK)
	{
		retval = XLNX_ETHERNET_ERROR_CU_NAME_NOT_FOUND;
	}


	if (retval == XLNX_OK)
	{	
		retval = m_pDeviceInterface->GetCUIndex(cuName, &m_cuIndex);

		if (retval != XLNX_OK)
		{
			retval = XLNX_ETHERNET_ERROR_CU_NAME_NOT_FOUND;
		}
	}


	if (retval == XLNX_OK)
	{
		retval = InitialiseNumSupportedChannels();
	}


	if (retval == XLNX_OK)
	{
		for (uint32_t i = 0; i < MAX_SUPPORTED_CHANNELS; i++)
		{
			SetChannelConfigurationAllowed(i, true); //default to true - will only be set to false in certain circumstances.
		}
	}


	if (retval == XLNX_OK)
	{
		m_initialisedMagicNumber = IS_INITIALISED_MAGIC_NUMBER;
	}

	return retval;
}






void Ethernet::IsInitialised(bool* pbIsInitialised)
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






uint32_t Ethernet::CheckIsInitialised(void)
{
	uint32_t retval = XLNX_OK;

	if (m_initialisedMagicNumber != IS_INITIALISED_MAGIC_NUMBER)
	{
		retval = XLNX_ETHERNET_ERROR_NOT_INITIALISED;
	}

	return retval;
}




//The following function is where we figure out out many channels the underlying HW block as been built with...
uint32_t Ethernet::InitialiseNumSupportedChannels(void)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;

	retval = ReadReg32(XLNX_ETHERNET_KERNEL_RESET_CONTROL_OFFSET, &value);

	if (retval == XLNX_OK)
	{
		if (value & (1 << 8))
		{
			m_numSupportedChannels = 4;
		}
		else
		{
			m_numSupportedChannels = 2;
		}
	}

	return retval;
}





uint32_t Ethernet::GetNumSupportedChannels(uint32_t* pNumChannels)
{
	uint32_t retval = XLNX_OK;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		*pNumChannels = m_numSupportedChannels;
	}

	return retval;
}





uint32_t Ethernet::GetCUIndex(uint32_t* pCUIndex)
{
	uint32_t retval = XLNX_OK;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		*pCUIndex = m_cuIndex;
	}

	return retval;
}




uint32_t Ethernet::GetCUAddress(uint64_t* pCUAddress)
{
	uint32_t retval = XLNX_OK;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		*pCUAddress = m_cuAddress;
	}

	return retval;
}




void Ethernet::SetChannelConfigurationAllowed(uint32_t channel, bool bAllowed)
{
	if (channel < MAX_SUPPORTED_CHANNELS)
	{
		m_bChannelConfigurationAllowed[channel] = bAllowed;
	}
}


void Ethernet::GetChannelConfigurationAllowed(uint32_t channel, bool* pbAllowed)
{
	if (channel < MAX_SUPPORTED_CHANNELS)
	{
		*pbAllowed = m_bChannelConfigurationAllowed[channel];
	}
	else
	{
		*pbAllowed = false;
	}
}










uint32_t Ethernet::CheckChannel(uint32_t channel)
{
	uint32_t retval = XLNX_OK;

	if (channel >= m_numSupportedChannels)
	{
		retval = XLNX_ETHERNET_ERROR_CHANNEL_OUT_OF_RANGE;
	}

	return retval;
}







uint32_t Ethernet::CheckChannelConfigurationAllowed(uint32_t channel)
{
	uint32_t retval = XLNX_OK;
	bool bAllowed = true;

	GetChannelConfigurationAllowed(channel, &bAllowed);

	if (bAllowed == false)
	{
		retval = XLNX_ETHERNET_ERROR_CHANNEL_CONFIGURATION_DISABLED_DUE_TO_HW_LIMITATIONS;
	}
	

	return retval;
}























uint32_t Ethernet::ReadReg32(uint64_t offset, uint32_t* value)
{
	uint32_t retval = XLNX_OK;
	uint64_t address = m_cuAddress + offset;

	retval = m_pDeviceInterface->ReadReg32(address, value);

	if (retval != XLNX_OK)
	{
		retval = XLNX_ETHERNET_ERROR_IO_FAILED;
	}

	return retval;
}






uint32_t Ethernet::WriteReg32(uint64_t offset, uint32_t value)
{
	uint32_t retval = XLNX_OK;
	uint64_t address = m_cuAddress + offset;

	retval = m_pDeviceInterface->WriteReg32(address, value);

	if (retval != XLNX_OK)
	{
		retval = XLNX_ETHERNET_ERROR_IO_FAILED;
	}

	return retval;
}







uint32_t Ethernet::WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask)
{
	uint32_t retval = XLNX_OK;
	uint64_t address = m_cuAddress + offset;

	retval = m_pDeviceInterface->WriteRegWithMask32(address, value, mask);

	if (retval != XLNX_OK)
	{
		retval = XLNX_ETHERNET_ERROR_IO_FAILED;
	}

	return retval;
}











uint32_t Ethernet::SetKernelReset(bool bInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t shift = 0;
	uint32_t mask = 0x01;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		if (bInReset)
		{
			value = 1;
		}
		else
		{
			value = 0;
		}
		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_KERNEL_RESET_CONTROL_OFFSET, value, mask);
	}

	return retval;
}


uint32_t Ethernet::GetKernelReset(bool* pbInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t shift = 0;
	uint32_t mask = 0x01;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_KERNEL_RESET_CONTROL_OFFSET, &value);
	}

	if (retval == XLNX_OK)
	{	
		*pbInReset = (bool)((value >> shift) & mask);
	}

	return retval;
}

