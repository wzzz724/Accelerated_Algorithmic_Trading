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

#include "xlnx_tcp_udp_ip.h"
#include "xlnx_tcp_udp_ip_address_map.h"
#include "xlnx_tcp_udp_ip_error_codes.h"

using namespace XLNX;






//--------------
// IGMP OpCodes
//--------------
static const uint32_t IGMP_OPCODE_NOOP		= 0x00;
static const uint32_t IGMP_OPCODE_ADD		= 0x01;
static const uint32_t IGMP_OPCODE_DELETE	= 0x02;
static const uint32_t IGMP_OPCODE_READ		= 0x03;



uint32_t TCPUDPIP::SetIGMPEnabled(bool bEnabled)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckConfigurationIsAllowed();
	}

	if (retval == XLNX_OK)
	{

		mask = 0x00000001;

		if (bEnabled)
		{
			value = 0x00000001;
		}
		else
		{
			value = 0x00000000;
		}

		retval = WriteRegWithMask32(XLNX_TCP_UDP_IP_IGMP_ENABLE_REG_OFFSET, value, mask);
	}


	return retval;
}









uint32_t TCPUDPIP::GetIGMPEnabled(bool* pbEnabled)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask;

	retval = CheckIsInitialised();


	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_TCP_UDP_IP_IGMP_ENABLE_REG_OFFSET, &value);

		mask = 0x00000001;

		if (retval == XLNX_OK)
		{
			if ((value & mask) != 0)
			{
				*pbEnabled = true;
			}
			else
			{
				*pbEnabled = false;
			}
		}
	}

	return retval;
}








uint32_t TCPUDPIP::AddIPv4MulticastAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t numValidEntries;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckConfigurationIsAllowed();
	}


	if (retval == XLNX_OK)
	{
		//Check if the HW has a free slot...
		retval = GetNumValidIGMPEntries(&numValidEntries);
		
		if (retval == XLNX_OK)
		{
			if (numValidEntries >= NUM_MULTICAST_ADDRESSES_SUPPORTED)
			{
				retval = XLNX_TCP_UDP_IP_ERROR_NO_FREE_MULTICAST_ADDRESS_SLOTS;
			}
		}
	}

	


	if (retval == XLNX_OK)
	{
		PackIPAddress(a, b, c, d, &value);

		retval = WriteReg32(XLNX_TCP_UDP_IP_IGMP_MULTICAST_ADDRESS_IN_REG_OFFSET, value);
	}


	if (retval == XLNX_OK)
	{
		retval = WriteReg32(XLNX_TCP_UDP_IP_IGMP_HOST_OPCODE_REG_OFFSET, IGMP_OPCODE_ADD);
	}


	if (retval == XLNX_OK)
	{		
		retval = WriteReg32(XLNX_TCP_UDP_IP_IGMP_HOST_OP_TOGGLE_REG_OFFSET, 0x01);
	}


	if (retval == XLNX_OK)
	{
		retval = WriteReg32(XLNX_TCP_UDP_IP_IGMP_HOST_OP_TOGGLE_REG_OFFSET, 0x00);
	}


	return retval;
}










uint32_t TCPUDPIP::DeleteIPv4MulticastAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;


	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckConfigurationIsAllowed();
	}

	if (retval == XLNX_OK)
	{
		PackIPAddress(a, b, c, d, &value);

		retval = WriteReg32(XLNX_TCP_UDP_IP_IGMP_MULTICAST_ADDRESS_IN_REG_OFFSET, value);
	}



	if (retval == XLNX_OK)
	{
		retval = WriteReg32(XLNX_TCP_UDP_IP_IGMP_HOST_OPCODE_REG_OFFSET, IGMP_OPCODE_DELETE);
	}



	if (retval == XLNX_OK)
	{
		retval = WriteReg32(XLNX_TCP_UDP_IP_IGMP_HOST_OP_TOGGLE_REG_OFFSET, 0x01);
	}



	if (retval == XLNX_OK)
	{
		retval = WriteReg32(XLNX_TCP_UDP_IP_IGMP_HOST_OP_TOGGLE_REG_OFFSET, 0x00);
	}



	return retval;
}







uint32_t TCPUDPIP::ReadIGMPEntry(uint32_t index, uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		if (index >= NUM_MULTICAST_ADDRESSES_SUPPORTED)
		{
			retval = XLNX_TCP_UDP_IP_ERROR_MULTICAST_ADDRESS_INDEX_OUT_OF_RANGE;
		}
	}


	if (retval == XLNX_OK)
	{
		retval = WriteReg32(XLNX_TCP_UDP_IP_IGMP_HOST_OPCODE_REG_OFFSET, IGMP_OPCODE_READ);
	}

	if (retval == XLNX_OK)
	{
		retval = WriteReg32(XLNX_TCP_UDP_IP_IGMP_TABLE_ADDR_REG_OFFSET, (index << 2)); //HW expects this to value to be a multiple of 4...
	}

	if (retval == XLNX_OK)
	{
		retval = WriteReg32(XLNX_TCP_UDP_IP_IGMP_HOST_OP_TOGGLE_REG_OFFSET, 0x01);
	}

	if (retval == XLNX_OK)
	{
		retval = WriteReg32(XLNX_TCP_UDP_IP_IGMP_HOST_OP_TOGGLE_REG_OFFSET, 0x00);
	}

	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_TCP_UDP_IP_IGMP_MULTICAST_ADDRESS_OUT_REG_OFFSET, &value);
	}

	if (retval == XLNX_OK)
	{
		UnpackIPAddress(value, a, b, c, d);
	}

	return retval;
}










uint32_t TCPUDPIP::GetNumValidIGMPEntries(uint32_t* pNumValidEntries)
{
	uint32_t retval = XLNX_OK;
	uint32_t numValidEntries = 0;
	uint32_t i;
	uint8_t a, b, c, d;

	retval = CheckIsInitialised();


	if (retval == XLNX_OK)
	{
		for (i = 0; i < NUM_MULTICAST_ADDRESSES_SUPPORTED; i++)
		{
			retval = ReadIGMPEntry(i, &a, &b, &c, &d);

			if (retval == XLNX_OK)
			{
				//if any of the dotted quad values are non-zero, 
				//we will consider this a valid entry...
				if ((a != 0) || (b != 0) || (c != 0) || (d != 0))
				{
					numValidEntries++;
				}
			}


			if (retval != XLNX_OK)
			{
				break; //out of loop
			}
		}
	}

	*pNumValidEntries = numValidEntries;

	return retval;
}




uint32_t TCPUDPIP::SetIGMPVersion(uint32_t version)
{
	uint32_t retval = XLNX_OK;
	uint32_t mask = 0x03;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckConfigurationIsAllowed();
	}

	if ((version > IGMP_MAX_SUPPORTED_VERSION) || (version < IGMP_MIN_SUPPORTED_VERSION))
	{
		retval = XLNX_TCP_UDP_IP_ERROR_UNSUPPORTED_IGMP_VERSION;
	}

	if (retval == XLNX_OK)
	{
		retval = WriteRegWithMask32(XLNX_TCP_UDP_IP_IGMP_VER_OFFSET, version, mask);
	}

	if (retval == XLNX_OK)
	{
		retval = WriteReg32(XLNX_TCP_UDP_IP_IGMP_HOST_OPCODE_REG_OFFSET, IGMP_OPCODE_NOOP);
	}

	if (retval == XLNX_OK)
	{
		retval = WriteReg32(XLNX_TCP_UDP_IP_IGMP_HOST_OP_TOGGLE_REG_OFFSET, 0x01);
	}

	if (retval == XLNX_OK)
	{
		retval = WriteReg32(XLNX_TCP_UDP_IP_IGMP_HOST_OP_TOGGLE_REG_OFFSET, 0x00);
	}

	return retval;
}






uint32_t TCPUDPIP::GetIGMPVersion(uint32_t* version)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x03;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_TCP_UDP_IP_IGMP_VER_OFFSET, &value);

		if (retval == XLNX_OK)
		{
			*version = (value & mask);
		}
	}

	return retval;
}






void TCPUDPIP::InitialiseDefaultIGMPVersionIfNecessary(void)
{
	uint32_t retval = XLNX_OK;
	uint32_t version;

	retval = GetIGMPVersion(&version);

	if (retval == XLNX_OK)
	{
		//We only want to set the DEFAULT version if the current version is outside of our supported range.
		//e.g. a freshly programmed HW load will have a value of 0 in the register...so we DO want to set the
		//     default value in that case.
		//     However, if the software runs up on an already configured card (that has a valid version 
		//     already programmed), we DO NOT want to overwrite that.
		if ((version < IGMP_MIN_SUPPORTED_VERSION) || (version > IGMP_MAX_SUPPORTED_VERSION))
		{
			retval = SetIGMPVersion(IGMP_DEFAULT_VERSION);
		}
	}
}
