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

using namespace XLNX;








void TCPUDPIP::CalculateUDPPortCAMOffsets(uint16_t port, uint32_t* pWordIndex, uint32_t* pBitIndex)
{
	*pWordIndex = port / 32;
	*pBitIndex = port % 32;
}







uint32_t TCPUDPIP::AddUDPListeningPort(uint16_t port)
{
	uint32_t retval = XLNX_OK;
	uint64_t offset = 0;
	uint32_t value = 0;
	uint32_t wordIndex = 0;
	uint32_t bitIndex = 0;



	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckUDPIsSynthesized();
	}

	if (retval == XLNX_OK)
	{
		retval = CheckConfigurationIsAllowed();
	}
	

	if (retval == XLNX_OK)
	{
		CalculateUDPPortCAMOffsets(port, &wordIndex, &bitIndex);
	}


	if (retval == XLNX_OK)
	{

		//First check to see if the specified port has already been added....
		offset = XLNX_TCP_UDP_IP_UDP_LISTEN_PORT_CAM_OFFSET + (wordIndex * 4);

		retval = ReadReg32(offset, &value);		
	}


	if (retval == XLNX_OK)
	{
		if ((value & (1 << bitIndex)) != 0)
		{
			retval = XLNX_TCP_UDP_IP_ERROR_LISTENING_PORT_ALREADY_ENABLED;
		}
	}


	if (retval == XLNX_OK)
	{
		//Set the appropriate bit in the word and write all 32-bits back down to HW...

		value |= (1 << bitIndex);

		retval = WriteReg32(offset, value);
	}

	return retval;
}








uint32_t TCPUDPIP::DeleteUDPListeningPort(uint16_t port)
{
	uint32_t retval = XLNX_OK;
	uint64_t offset = 0;
	uint32_t value = 0;
	uint32_t wordIndex = 0;
	uint32_t bitIndex = 0;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckUDPIsSynthesized();
	}

	if (retval == XLNX_OK)
	{
		retval = CheckConfigurationIsAllowed();
	}


	if (retval == XLNX_OK)
	{
		CalculateUDPPortCAMOffsets(port, &wordIndex, &bitIndex);
	}


	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_UDP_LISTEN_PORT_CAM_OFFSET + (wordIndex * 4);

		retval = ReadReg32(offset, &value);	
	}

	if (retval == XLNX_OK)
	{
		if ((value & (1 << bitIndex)) == 0)
		{
			retval = XLNX_TCP_UDP_IP_ERROR_LISTENING_PORT_NOT_ENABLED;
		}
	}

	if (retval == XLNX_OK)
	{
		value &= ~(1 << bitIndex);

		retval = WriteReg32(offset, value);
	}

	

	return retval;
}






uint32_t TCPUDPIP::DeleteAllUDPListeningPorts(void)
{
	uint32_t retval = XLNX_OK;
	uint64_t offset;
	uint32_t numWords;
	uint32_t i;


	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckUDPIsSynthesized();
	}

	if (retval == XLNX_OK)
	{
		retval = CheckConfigurationIsAllowed();
	}


	if (retval == XLNX_OK)
	{
		numWords = NUM_LISTENING_PORTS_SUPPORTED / 32;

		for (i = 0; i < numWords; i++)
		{
			offset = XLNX_TCP_UDP_IP_UDP_LISTEN_PORT_CAM_OFFSET + (i * 4);

			retval = WriteReg32(offset, 0x00000000);
			
			if (retval != XLNX_OK)
			{
				break; //out of loop
			}
		}
	}
	

	return retval;
}







uint32_t TCPUDPIP::IsUDPListeningPortEnabled(uint16_t port, bool* pbEnabled)
{
	uint32_t retval = XLNX_OK;
	uint64_t offset = 0;
	uint32_t value = 0;
	uint32_t wordIndex = 0;
	uint32_t bitIndex = 0;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckUDPIsSynthesized();
	}


	if (retval == XLNX_OK)
	{
		CalculateUDPPortCAMOffsets(port, &wordIndex, &bitIndex);
	}


	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_UDP_LISTEN_PORT_CAM_OFFSET + (wordIndex * 4);

		retval = ReadReg32(offset, &value);
	}

	if (retval == XLNX_OK)
	{
		if ((value & (1 << bitIndex)) != 0)
		{
			*pbEnabled = true;
		}
		else
		{
			*pbEnabled = false;
		}
	}


	return retval;
}







uint32_t TCPUDPIP::GetUDPListeningPorts(uint16_t enabledPorts[NUM_LISTENING_PORTS_SUPPORTED], uint32_t* pNumEnabledPorts)
{
	uint32_t retval = XLNX_OK;
	uint64_t offset;
	uint32_t value;
	uint32_t numWords;
	uint32_t i;
	uint32_t j;
	uint32_t port;
	uint32_t numEnabledPorts = 0;


	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckUDPIsSynthesized();
	}


	if (retval == XLNX_OK)
	{
		numWords = NUM_LISTENING_PORTS_SUPPORTED / 32;


		port = 0;

		for (i = 0; i < numWords; i++)
		{
			offset = XLNX_TCP_UDP_IP_UDP_LISTEN_PORT_CAM_OFFSET + (i * 4);

			retval = ReadReg32(offset, &value);

			if (retval == XLNX_OK)
			{
				for (j = 0; j < 32; j++)
				{
					if ((value & (1 << j)) != 0)
					{
						enabledPorts[numEnabledPorts] = port;
						numEnabledPorts++;
					}			

					port++;
				}
			}

		}

		*pNumEnabledPorts = numEnabledPorts;
	}


	return retval;
}

