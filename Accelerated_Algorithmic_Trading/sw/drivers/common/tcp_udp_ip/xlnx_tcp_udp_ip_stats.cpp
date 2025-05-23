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






uint32_t TCPUDPIP::GetIGMPStats(IGMPStats* pStats)
{
	uint32_t retval = XLNX_OK;
	uint64_t offset;
	uint32_t buffer[XLNX_TCP_UDP_IP_NUM_IGMP_STATS_REGISTERS];


	retval = CheckIsInitialised();

	
	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_NUM_IGMP_STATS_INVALID_CSUM;
		retval = ReadReg32(offset, &buffer[0]);
	}

	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_NUM_IGMP_STATS_IGMPV3_QUERY;
		retval = ReadReg32(offset, &buffer[1]);
	}

	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_NUM_IGMP_STATS_INVALID_QUERY;
		retval = ReadReg32(offset, &buffer[2]);
	}
	if (retval == XLNX_OK)
	{
		//pull out the stats counters into meaningful fields...
		pStats->numInvalidChecksum	= buffer[0];
		pStats->numIGMPQueries	    = buffer[1];
		pStats->numInvalidQueries	= buffer[2];
	}

	return retval;
}







uint32_t TCPUDPIP::GetARPStats(ARPStats* pStats)
{
	uint32_t retval = XLNX_OK;
	uint64_t offset;
	uint32_t buffer[XLNX_TCP_UDP_IP_NUM_ARP_STATS_REGISTERS];


	retval = CheckIsInitialised();


	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_ARP_STATS_REQ_SENT;
		retval = ReadReg32(offset, &buffer[0]);
	}

	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_ARP_STATS_REPLIES_SENT;
		retval = ReadReg32(offset, &buffer[1]);
	}

	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_ARP_STATS_REQ_RECEIVED;
		retval = ReadReg32(offset, &buffer[2]);
	}

	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_ARP_STATS_REPLIES_RECEIVED;
		retval = ReadReg32(offset, &buffer[3]);
	}

	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_ARP_STATS_REQ_SENT_LOST;
		retval = ReadReg32(offset, &buffer[4]);
	}

	if (retval == XLNX_OK)
	{
		//pull out the stats counters into meaningful fields...
		pStats->numRequestsSent		= buffer[0];
		pStats->numRepliesSent		= buffer[1];
		pStats->numRequestsReceived = buffer[2];
		pStats->numRepliesReceived	= buffer[3];
		pStats->numRequestsSentLost = buffer[4];
	}

	return retval;
}





uint32_t TCPUDPIP::GetICMPStats(ICMPStats* pStats)
{
	uint32_t retval = XLNX_OK;
	uint64_t offset;
	uint32_t buffer[XLNX_TCP_UDP_IP_NUM_ICMP_STATS_REGISTERS];


	retval = CheckIsInitialised();


	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_ICMP_STATS_ECHO_REQUESTS_RECEIVED;
		retval = ReadReg32(offset, &buffer[0]);
	}

	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_ICMP_STATS_ECHO_REPLIES_SENT;
		retval = ReadReg32(offset, &buffer[1]);
	}

	if (retval == XLNX_OK)
	{
		//pull out the stats counters into meaningful fields...
		pStats->numEchoRequestsReceived = buffer[0];
		pStats->numEchoRepliesSent		= buffer[1];
	}

	return retval;
}






uint32_t TCPUDPIP::GetIPHandlerStats(IPHandlerStats* pStats)
{
	uint32_t retval = XLNX_OK;
	uint64_t offset;
	uint32_t buffer[XLNX_TCP_UDP_IP_NUM_IP_HANDLER_STATS_REGISTERS];


	retval = CheckIsInitialised();


	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_IP_HANDLER_STATS_IPINHDRERRORS;
		retval = ReadReg32(offset, &buffer[0]);
	}

	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_IP_HANDLER_STATS_IPINDELIVERS;
		retval = ReadReg32(offset, &buffer[1]);
	}

	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_IP_HANDLER_STATS_IPINUNKNOWNPROTOS;
		retval = ReadReg32(offset, &buffer[2]);
	}

	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_IP_HANDLER_STATS_IPINADDRERRORS;
		retval = ReadReg32(offset, &buffer[3]);
	}

	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_IP_HANDLER_STATS_IPINRECEIVES;
		retval = ReadReg32(offset, &buffer[4]);
	}


	if (retval == XLNX_OK)
	{
		//pull out the stats counters into meaningful fields...
		pStats->numInHeaderErrors		= buffer[0];
		pStats->numInDelivers			= buffer[1];
		pStats->numInUnknownProtocols	= buffer[2];
		pStats->numInAddressErrors		= buffer[3];
		pStats->numInReceives			= buffer[4];
	}

	return retval;
}






uint32_t TCPUDPIP::GetMACIPEncoderStats(MACIPEncoderStats* pStats)
{
	uint32_t retval = XLNX_OK;
	uint64_t offset;
	uint32_t buffer[XLNX_TCP_UDP_IP_NUM_MAC_IP_ENCODER_STATS_REGISTERS];


	retval = CheckIsInitialised();


	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_MAC_IP_ENCODER_STATS_IPV4_PACKETS_SENT;
		retval = ReadReg32(offset, &buffer[0]);
	}

	if (retval == XLNX_OK)
	{
		offset = XLNX_TCP_UDP_IP_MAC_IP_ENCODER_STATS_PACKETS_DROPPED;
		retval = ReadReg32(offset, &buffer[1]);
	}


	if (retval == XLNX_OK)
	{
		//pull out the stats counters into meaningful fields...
		pStats->numIPv4PacketsSent	= buffer[0];
		pStats->numPacketsDropped	= buffer[1];
	}

	return retval;
}








uint32_t TCPUDPIP::GetUDPStats(UDPStats* pStats)
{
	uint32_t retval = XLNX_OK;
	uint64_t offset;
	uint32_t buffer[XLNX_TCP_UDP_IP_NUM_UDP_STATS_REGISTERS];
	bool bStatsValid = false;


	retval = CheckIsInitialised();


	if (retval == XLNX_OK)
	{
		//Only want to read registers if they have been synthesized in HW...
		IsUDPSynthesized(&bStatsValid);
		
		if (bStatsValid)
		{
			if (retval == XLNX_OK)
			{
				offset = XLNX_TCP_UDP_IP_UDP_STATS_DATAGRAMS_TRANSMITTED;
				retval = ReadReg32(offset, &buffer[0]);
			}

			if (retval == XLNX_OK)
			{
				offset = XLNX_TCP_UDP_IP_UDP_STATS_DATAGRAMS_RECEIVED;
				retval = ReadReg32(offset, &buffer[1]);
			}

			if (retval == XLNX_OK)
			{
				offset = XLNX_TCP_UDP_IP_UDP_STATS_DATAGRAMS_RECEIVED_INVALID_PORT;
				retval = ReadReg32(offset, &buffer[2]);
			}
		}
		else
		{
			//just zero out the buffer...
			memset(buffer, 0, sizeof(buffer));
		}
	}


	if (retval == XLNX_OK)
	{
		//Set the flag to inidicate if the counter values are valid...
		pStats->bStatsValid = bStatsValid;


		//pull out the stats counters into meaningful fields...
		pStats->numTxDatagrams				= buffer[0];
		pStats->numRxDatagrams				= buffer[1];
		pStats->numRxDatagramsInvalidPort	= buffer[2];
	}

	return retval;
}








uint32_t TCPUDPIP::GetTCPStats(TCPStats* pStats)
{
	uint32_t retval = XLNX_OK;
	uint64_t offset;
	uint32_t buffer[XLNX_TCP_UDP_IP_NUM_TCP_STATS_REGISTERS];
	bool bStatsValid = false;


	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		//Only want to read registers if they have been synthesized in HW...
		IsTCPSynthesized(&bStatsValid);

		if (bStatsValid)
		{
			if (retval == XLNX_OK)
			{
				offset = XLNX_TCP_UDP_IP_TCP_STATS_TCPINSEGS;
				retval = ReadReg32(offset, &buffer[0]);
			}

			if (retval == XLNX_OK)
			{
				offset = XLNX_TCP_UDP_IP_TCP_STATS_TCPINERRS;
				retval = ReadReg32(offset, &buffer[1]);
			}

			if (retval == XLNX_OK)
			{
				offset = XLNX_TCP_UDP_IP_TCP_STATS_TCPOUTSEGS;
				retval = ReadReg32(offset, &buffer[2]);
			}

			if (retval == XLNX_OK)
			{
				offset = XLNX_TCP_UDP_IP_TCP_STATS_TCPRETRANSSEGS;
				retval = ReadReg32(offset, &buffer[3]);
			}

			if (retval == XLNX_OK)
			{
				offset = XLNX_TCP_UDP_IP_TCP_STATS_TCPACTIVEOPENS;
				retval = ReadReg32(offset, &buffer[4]);
			}

			if (retval == XLNX_OK)
			{
				offset = XLNX_TCP_UDP_IP_TCP_STATS_TCPPASSIVEOPENS;
				retval = ReadReg32(offset, &buffer[5]);
			}

			if (retval == XLNX_OK)
			{
				offset = XLNX_TCP_UDP_IP_TCP_STATS_TCPATTEMPTFAILS;
				retval = ReadReg32(offset, &buffer[6]);
			}

			if (retval == XLNX_OK)
			{
				offset = XLNX_TCP_UDP_IP_TCP_STATS_TCPESTABRESETS;
				retval = ReadReg32(offset, &buffer[7]);
			}

			if (retval == XLNX_OK)
			{
				offset = XLNX_TCP_UDP_IP_TCP_STATS_TCPCURRESTAB;
				retval = ReadReg32(offset, &buffer[8]);
			}
		}
		else
		{
			//just zero out the buffer...
			memset(buffer, 0, sizeof(buffer));
		}
	}


	if (retval == XLNX_OK)
	{
		//Set the flag to inidicate if the counter values are valid...
		pStats->bStatsValid = bStatsValid;

		//pull out the stats counters into meaningful fields...

		pStats->numInSegments				= buffer[0];
		pStats->numInErrors					= buffer[1];
		pStats->numOutSegments				= buffer[2];
		pStats->numRetransmittedSegments	= buffer[3];
		pStats->numActiveOpens				= buffer[4];
		pStats->numPassiveOpens				= buffer[5];
		pStats->numAttemptFails				= buffer[6];
		pStats->numEstablishResets			= buffer[7];
		pStats->numCurrentEstablished		= buffer[8];
	}

	return retval;
}













uint32_t TCPUDPIP::GetStats(Stats* pStats)
{
	uint32_t retval = XLNX_OK;
	

	retval = CheckIsInitialised();


	if (retval == XLNX_OK)
	{
		retval = GetIGMPStats(&(pStats->igmp));
	}


	if (retval == XLNX_OK)
	{
		retval = GetARPStats(&(pStats->arp));
	}


	if (retval == XLNX_OK)
	{
		retval = GetICMPStats(&(pStats->icmp));
	}


	if (retval == XLNX_OK)
	{
		retval = GetIPHandlerStats(&(pStats->ipHandler));
	}


	if (retval == XLNX_OK)
	{
		retval = GetMACIPEncoderStats(&(pStats->macIPEncoder));
	}



	if (retval == XLNX_OK)
	{
		retval = GetUDPStats(&(pStats->udp));
	}


	
	if (retval == XLNX_OK)
	{
		retval = GetTCPStats(&(pStats->tcp));
	}

	
	return retval;
}



