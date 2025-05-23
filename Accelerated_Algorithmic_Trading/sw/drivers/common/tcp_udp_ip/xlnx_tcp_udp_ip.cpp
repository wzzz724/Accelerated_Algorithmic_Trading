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

#include <string.h>

#include "xlnx_tcp_udp_ip.h"
#include "xlnx_tcp_udp_ip_address_map.h"
#include "xlnx_tcp_udp_ip_error_codes.h"

using namespace XLNX;




static const uint32_t IS_INITIALISED_MAGIC_NUMBER = 0x373CFA10;



static const char* UDP_ONLY_KERNEL_CLASS_NAME			= "udp_ip_krnl";
static const char* TCP_ONLY_KERNEL_CLASS_NAME			= "tcp_ip_krnl";
static const char* TCP_UDP_COMBINED_KERNEL_CLASS_NAME	= "tcp_udp_ip_krnl";





TCPUDPIP::TCPUDPIP()
{
	m_pDeviceInterface = nullptr;
	m_cuIndex = 0xFFFFFFFF;
	m_cuAddress = 0;
	m_initialisedMagicNumber = 0;

	m_bTCPSynthesized = false;
	m_bUDPSynthesized = false;

}




TCPUDPIP::~TCPUDPIP()
{

}




uint32_t TCPUDPIP::Initialise(DeviceInterface* pDeviceInterface, const char* cuName)
{
	uint32_t retval = XLNX_OK;

	m_initialisedMagicNumber = 0;	//will be set to magic number if we successfully initialise...

	m_pDeviceInterface = pDeviceInterface;

	retval = m_pDeviceInterface->GetCUAddress(cuName, &m_cuAddress);	
	if (retval != XLNX_OK)
	{
		retval = XLNX_TCP_UDP_IP_ERROR_CU_NAME_NOT_FOUND;
	}


	if (retval == XLNX_OK)
	{
		retval = m_pDeviceInterface->GetCUIndex(cuName, &m_cuIndex);
		if (retval != XLNX_OK)
		{
			retval = XLNX_TCP_UDP_IP_ERROR_CU_NAME_NOT_FOUND;
		}
	}

	if (retval == XLNX_OK)
	{
		//We need to figure out the synthesis options that this instance has been built with.
		//Currently there are 3 ways it can be built:
		//(1) UDP Only
		//(2) TCP Only
		//(3) UDP and TCP

		if (strncmp(cuName, UDP_ONLY_KERNEL_CLASS_NAME, strlen(UDP_ONLY_KERNEL_CLASS_NAME)) == 0)
		{
			m_bTCPSynthesized = false;
			m_bUDPSynthesized = true;
		}
		else if (strncmp(cuName, TCP_ONLY_KERNEL_CLASS_NAME, strlen(TCP_ONLY_KERNEL_CLASS_NAME)) == 0)
		{
			m_bTCPSynthesized = true;
			m_bUDPSynthesized = false;
		}
		else if (strncmp(cuName, TCP_UDP_COMBINED_KERNEL_CLASS_NAME, strlen(TCP_UDP_COMBINED_KERNEL_CLASS_NAME)) == 0)
		{
			m_bTCPSynthesized = true;
			m_bUDPSynthesized = true;
		}
	}



	if (retval == XLNX_OK)
	{
		//default configuration ENABLED...it will only be disabled in special circumstances.
		SetConfigurationAllowed(true);
	}

	

	if (retval == XLNX_OK)
	{
		m_initialisedMagicNumber = IS_INITIALISED_MAGIC_NUMBER;
	}
	


	//The following need to be AFTER the magic number initialisation..

	if (retval == XLNX_OK)
	{
		InitialiseDefaultIGMPVersionIfNecessary();
	}


	return retval;
}









uint32_t TCPUDPIP::CheckIsInitialised(void)
{
	uint32_t retval = XLNX_OK;

	if (m_initialisedMagicNumber != IS_INITIALISED_MAGIC_NUMBER)
	{
		retval = XLNX_TCP_UDP_IP_ERROR_NOT_INITIALISED;
	}

	return retval;
}





uint32_t TCPUDPIP::CheckTCPIsSynthesized(void)
{
	uint32_t retval = XLNX_OK;
	
	if (m_bTCPSynthesized == false)
	{
		retval = XLNX_TCP_UDP_IP_ERROR_FEATURE_NOT_SYNTHESIZED_IN_HW;
	}


	return retval;
}




uint32_t TCPUDPIP::CheckUDPIsSynthesized(void)
{
	uint32_t retval = XLNX_OK;

	if (m_bUDPSynthesized == false)
	{
		retval = XLNX_TCP_UDP_IP_ERROR_FEATURE_NOT_SYNTHESIZED_IN_HW;
	}

	return retval;
}



uint32_t TCPUDPIP::CheckConfigurationIsAllowed(void)
{
	uint32_t retval = XLNX_OK;

	if (m_bConfigurationAllowed == false)
	{
		retval = XLNX_TCP_UDP_IP_ERROR_CONFIGURATION_DISABLED_DUE_TO_HW_LIMITATIONS;
	}

	return retval;
}


void TCPUDPIP::IsInitialised(bool* pbIsInitialised)
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




void TCPUDPIP::IsTCPSynthesized(bool* bIsSynthesized)
{
	if (CheckTCPIsSynthesized() == XLNX_OK)
	{
		*bIsSynthesized = true;
	}
	else
	{
		*bIsSynthesized = false;
	}
}



void TCPUDPIP::IsUDPSynthesized(bool* bIsSynthesized)
{
	if (CheckUDPIsSynthesized() == XLNX_OK)
	{
		*bIsSynthesized = true;
	}
	else
	{
		*bIsSynthesized = false;
	}
}





uint32_t TCPUDPIP::ReadReg32(uint64_t offset, uint32_t* value)
{
	uint32_t retval = XLNX_OK;
	uint64_t address = m_cuAddress + offset;

	retval = m_pDeviceInterface->ReadReg32(address, value);

	if (retval != XLNX_OK)
	{
		retval = XLNX_TCP_UDP_IP_ERROR_IO_FAILED;
	}

	return retval;
}










uint32_t TCPUDPIP::WriteReg32(uint64_t offset, uint32_t value)
{
	uint32_t retval = XLNX_OK;
	uint64_t address = m_cuAddress + offset;

	retval = m_pDeviceInterface->WriteReg32(address, value);

	if (retval != XLNX_OK)
	{
		retval = XLNX_TCP_UDP_IP_ERROR_IO_FAILED;
	}

	return retval;
}









uint32_t TCPUDPIP::WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask)
{
	uint32_t retval = XLNX_OK;
	uint64_t address = m_cuAddress + offset;

	retval = m_pDeviceInterface->WriteRegWithMask32(address, value, mask);

	if (retval != XLNX_OK)
	{
		retval = XLNX_TCP_UDP_IP_ERROR_IO_FAILED;
	}

	return retval;
}





uint32_t TCPUDPIP::BlockReadReg32(uint64_t offset, uint32_t* buffer, uint32_t numWords)
{
	uint32_t retval = XLNX_OK;
	uint64_t address = m_cuAddress + offset;

	retval = m_pDeviceInterface->BlockReadReg32(address, buffer, numWords);

	if (retval != XLNX_OK)
	{
		retval = XLNX_TCP_UDP_IP_ERROR_IO_FAILED;
	}

	return retval;
}







uint32_t TCPUDPIP::GetCUIndex(uint32_t* pCUIndex)
{
	uint32_t retval = XLNX_OK;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		*pCUIndex = m_cuIndex;
	}

	return retval;
}





uint32_t TCPUDPIP::GetCUAddress(uint64_t* pCUAddress)
{
	uint32_t retval = XLNX_OK;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		*pCUAddress = m_cuAddress;
	}

	return retval;
}










uint32_t TCPUDPIP::SetMACAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f)
{
	uint32_t retval = XLNX_OK;
	uint32_t value1 = 0;
	uint32_t value2 = 0;
	uint64_t offset1 = 0;
	uint64_t offset2 = 0;


	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckConfigurationIsAllowed();
	}

	if (retval == XLNX_OK)
	{
		offset1 = XLNX_TCP_UDP_IP_MAC_ADDRESS_CONFIG1_REG_OFFSET;
		offset2 = XLNX_TCP_UDP_IP_MAC_ADDRESS_CONFIG2_REG_OFFSET;


		value1 = 0;
		value2 = 0;

		value1 |= c << 24;
		value1 |= d << 16;
		value1 |= e << 8;
		value1 |= f << 0;

		value2 |= a << 8;
		value2 |= b << 0;
	}



	if (retval == XLNX_OK)
	{
		retval = WriteReg32(offset1, value1);
	
	}

	if (retval == XLNX_OK)
	{
		retval = WriteReg32(offset2, value2);
	}

	return retval;
}






uint32_t TCPUDPIP::GetMACAddress(uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d, uint8_t* e, uint8_t* f)
{
	uint32_t retval = XLNX_OK;

	uint32_t value1 = 0;
	uint32_t value2 = 0;
	uint64_t offset1 = 0;
	uint64_t offset2 = 0;


	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		offset1 = XLNX_TCP_UDP_IP_MAC_ADDRESS_CONFIG1_REG_OFFSET;
		offset2 = XLNX_TCP_UDP_IP_MAC_ADDRESS_CONFIG2_REG_OFFSET;
	}

	if (retval == XLNX_OK)
	{
		retval = ReadReg32(offset1, &value1);
	}


	if (retval == XLNX_OK)
	{
		retval = ReadReg32(offset2, &value2);
	}



	if (retval == XLNX_OK)
	{
		*c = (value1 >> 24) & 0xFF;
		*d = (value1 >> 16) & 0xFF;
		*e = (value1 >> 8) & 0xFF;
		*f = (value1 >> 0) & 0xFF;

		*a = (value2 >> 8) & 0xFF;
		*b = (value2 >> 0) & 0xFF;
	}

	return retval;
}








uint32_t TCPUDPIP::SetIPv4Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint64_t offset;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckConfigurationIsAllowed();
	}

	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_IPv4_ADDRESS_CONFIG_REG_OFFSET;

		PackIPAddress(a, b, c, d, &value);


		retval = WriteReg32(offset, value);
	}


	return retval;
}







uint32_t TCPUDPIP::GetIPv4Address(uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d)
{
	uint32_t retval = XLNX_OK;
	uint32_t value = 0;
	uint64_t offset;

	retval = CheckIsInitialised();


	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_IPv4_ADDRESS_CONFIG_REG_OFFSET;

		retval = ReadReg32(offset, &value);
	}


	if (retval == XLNX_OK)
	{
		UnpackIPAddress(value, a, b, c, d);
	}

	return retval;
}











uint32_t TCPUDPIP::SetSubnetMask(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint64_t offset;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckConfigurationIsAllowed();
	}

	if (retval == XLNX_OK)
	{
		offset =  XLNX_TCP_UDP_IP_SUBNET_MASK_CONFIG_REG_OFFSET;

		PackIPAddress(a, b, c, d, &value);


		retval = WriteReg32(offset, value);
	}


	return retval;
}








uint32_t TCPUDPIP::GetSubnetMask(uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d)
{
	uint32_t retval = XLNX_OK;
	uint32_t value = 0;
	uint64_t offset;

	retval = CheckIsInitialised();


	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_SUBNET_MASK_CONFIG_REG_OFFSET;

		retval = ReadReg32(offset, &value);
	}


	if (retval == XLNX_OK)
	{
		UnpackIPAddress(value, a, b, c, d);
	}

	return retval;
}







uint32_t TCPUDPIP::SetGatewayAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint64_t offset;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckConfigurationIsAllowed();
	}

	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_GATEWAY_ADDRESS_CONFIG_REG_OFFSET;

		PackIPAddress(a, b, c, d, &value);


		retval = WriteReg32(offset, value);
	}


	return retval;
}








uint32_t TCPUDPIP::GetGatewayAddress(uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d)
{
	uint32_t retval = XLNX_OK;
	uint32_t value = 0;
	uint64_t offset;

	retval = CheckIsInitialised();


	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_GATEWAY_ADDRESS_CONFIG_REG_OFFSET;

		retval = ReadReg32(offset, &value);
	}


	if (retval == XLNX_OK)
	{
		UnpackIPAddress(value, a, b, c, d);
	}

	return retval;
}





void TCPUDPIP::SetConfigurationAllowed(bool bAllowed)
{
	m_bConfigurationAllowed = bAllowed;
}



void TCPUDPIP::GetConfigurationAllowed(bool* pbAllowed)
{
	*pbAllowed = m_bConfigurationAllowed;
}





