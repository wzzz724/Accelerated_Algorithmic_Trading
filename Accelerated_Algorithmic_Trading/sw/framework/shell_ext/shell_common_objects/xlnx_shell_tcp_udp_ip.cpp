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



#include "xlnx_shell_tcp_udp_ip.h"
#include "xlnx_shell_utils.h"

#include "xlnx_tcp_udp_ip.h"
#include "xlnx_tcp_udp_ip_error_codes.h"
using namespace XLNX;





#define STR_CASE(TAG)	case(TAG):					\
						{							\
							pString = (char*) #TAG;	\
							break;					\
						}




static const char* NOT_AVAILABLE_STRING	= "N/A";


static char* TCP_UDP_IP_ErrorCodeToString(uint32_t errorCode)
{
	char* pString;

	switch (errorCode)
	{
		STR_CASE(XLNX_OK)
		STR_CASE(XLNX_TCP_UDP_IP_ERROR_NOT_INITIALISED)
		STR_CASE(XLNX_TCP_UDP_IP_ERROR_IO_FAILED)
		STR_CASE(XLNX_TCP_UDP_IP_ERROR_CU_NAME_NOT_FOUND)
		STR_CASE(XLNX_TCP_UDP_IP_ERROR_NO_FREE_MULTICAST_ADDRESS_SLOTS)
		STR_CASE(XLNX_TCP_UDP_IP_ERROR_MULTICAST_ADDRESS_INDEX_OUT_OF_RANGE)
		STR_CASE(XLNX_TCP_UDP_IP_ERROR_LISTENING_PORT_NOT_ENABLED)
		STR_CASE(XLNX_TCP_UDP_IP_ERROR_LISTENING_PORT_ALREADY_ENABLED)
		STR_CASE(XLNX_TCP_UDP_IP_ERROR_ARP_ENTRY_INDEX_OUT_OF_RANGE)
		STR_CASE(XLNX_TCP_UDP_IP_ERROR_NO_FREE_ARP_ENTRIES)
		STR_CASE(XLNX_TCP_UDP_IP_ERROR_FEATURE_NOT_SYNTHESIZED_IN_HW)
		STR_CASE(XLNX_TCP_UDP_IP_ERROR_CONFIGURATION_DISABLED_DUE_TO_HW_LIMITATIONS)
		STR_CASE(XLNX_TCP_UDP_IP_ERROR_UNSUPPORTED_IGMP_VERSION)

		default:
		{
			pString = (char*)"UKNOWN_ERROR";
			break;
		}
	}

	return pString;
}








static int TCP_UDP_IP_SetMACAddress(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	uint8_t MACAddress[6];
	TCPUDPIP* pTCPUDPIP;


	pTCPUDPIP = (TCPUDPIP*)pObjectData;


	if (argc != 2)
	{
		pShell->printf("Usage: %s <macaddr>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = ParseMACAddress(pShell, argv[1], &MACAddress[0],
														 &MACAddress[1],
														 &MACAddress[2],
														 &MACAddress[3],
														 &MACAddress[4],
														 &MACAddress[5]);

	}

	
	if (bOKToContinue)
	{
		retval = pTCPUDPIP->SetMACAddress(MACAddress[0],
										  MACAddress[1],
										  MACAddress[2],
										  MACAddress[3],
										  MACAddress[4],
										  MACAddress[5]);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] Failed to set MAC address, error = %s (0x%08X)\n", TCP_UDP_IP_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}











static int TCP_UDP_IP_SetIPv4Address(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	TCPUDPIP* pTCPUDPIP;
	uint8_t dottedQuad[4];


	pTCPUDPIP = (TCPUDPIP*)pObjectData;


	if (argc != 2)
	{
		pShell->printf("Usage: %s <ipaddr>\n", argv[0]);
		bOKToContinue = false;
	}



	if (bOKToContinue)
	{
		bOKToContinue = ParseIPv4Address(pShell, argv[1], &dottedQuad[0], &dottedQuad[1], &dottedQuad[2], &dottedQuad[3]);
	}



	if (bOKToContinue)
	{
		retval = pTCPUDPIP->SetIPv4Address(dottedQuad[0], dottedQuad[1], dottedQuad[2], dottedQuad[3]);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] Failed to set IP address, error = %s (0x%08X)\n", TCP_UDP_IP_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}






static int TCP_UDP_IP_SetSubnetMask(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	TCPUDPIP* pTCPUDPIP;
	uint8_t dottedQuad[4];


	pTCPUDPIP = (TCPUDPIP*)pObjectData;


	if (argc != 2)
	{
		pShell->printf("Usage: %s <ipaddr>\n", argv[0]);
		bOKToContinue = false;
	}



	if (bOKToContinue)
	{
		bOKToContinue = ParseIPv4Address(pShell, argv[1], &dottedQuad[0], &dottedQuad[1], &dottedQuad[2], &dottedQuad[3]);
	}



	if (bOKToContinue)
	{
		retval = pTCPUDPIP->SetSubnetMask(dottedQuad[0], dottedQuad[1], dottedQuad[2], dottedQuad[3]);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] Failed to set Subnet Mask, error = %s (0x%08X)\n", TCP_UDP_IP_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}









static int TCP_UDP_IP_SetGateway(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	TCPUDPIP* pTCPUDPIP;
	uint8_t dottedQuad[4];


	pTCPUDPIP = (TCPUDPIP*)pObjectData;


	if (argc != 2)
	{
		pShell->printf("Usage: %s <ipaddr>\n", argv[0]);
		bOKToContinue = false;
	}



	if (bOKToContinue)
	{
		bOKToContinue = ParseIPv4Address(pShell, argv[1], &dottedQuad[0], &dottedQuad[1], &dottedQuad[2], &dottedQuad[3]);
	}



	if (bOKToContinue)
	{
		retval = pTCPUDPIP->SetGatewayAddress(dottedQuad[0], dottedQuad[1], dottedQuad[2], dottedQuad[3]);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] Failed to set gateway address, error = %s (0x%08X)\n", TCP_UDP_IP_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}




static int TCP_UDP_IP_SetIGMPEnabled(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	TCPUDPIP* pTCPUDPIP;
	bool bEnabled;


	pTCPUDPIP = (TCPUDPIP*)pObjectData;

	if (argc != 2)
	{
		pShell->printf("Usage: %s <bool>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[1], &bEnabled);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to bool parameter\n");
		}
	}


	if (bOKToContinue)
	{
		retval = pTCPUDPIP->SetIGMPEnabled(bEnabled);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] Failed to set gateway address, error = %s (0x%08X)\n", TCP_UDP_IP_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}






static int TCP_UDP_IP_SetIGMPVersion(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	TCPUDPIP* pTCPUDPIP;
	uint32_t version;


	pTCPUDPIP = (TCPUDPIP*)pObjectData;

	if (argc != 2)
	{
		pShell->printf("Usage: %s <ver>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &version);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse uint32_t parameter\n");
		}
	}


	if (bOKToContinue)
	{
		retval = pTCPUDPIP->SetIGMPVersion(version);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] Failed to set IGMP version, error = %s (0x%08X)\n", TCP_UDP_IP_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}






static int TCP_UDP_IP_AddMulticastIPv4Address(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	TCPUDPIP* pTCPUDPIP;
	uint8_t dottedQuad[4];


	pTCPUDPIP = (TCPUDPIP*)pObjectData;


	if (argc != 2)
	{
		pShell->printf("Usage: %s <ipaddr>\n", argv[0]);
		bOKToContinue = false;
	}



	if (bOKToContinue)
	{
		bOKToContinue = ParseIPv4Address(pShell, argv[1], &dottedQuad[0], &dottedQuad[1], &dottedQuad[2], &dottedQuad[3]);
	}



	if (bOKToContinue)
	{

		//When we add a multicast address, the HW will send out an IGMP Join Request.  However the HW emulation framework
		//does not yet fully support this.  To prevent errors being caused in the HW emulation, we will skip the actual API call
		//when we are running in emulation mode.
#ifndef XCL_EMULATION_MODE
		retval = pTCPUDPIP->AddIPv4MulticastAddress(dottedQuad[0], dottedQuad[1], dottedQuad[2], dottedQuad[3]);
#else
		pShell->printf("[INFO] Skipping multicast address add in emulation mode\n");
#endif

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] Failed to add multicast address, error = %s (0x%08X)\n", TCP_UDP_IP_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}








static int TCP_UDP_IP_DeleteMulticastIP4vAddress(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	TCPUDPIP* pTCPUDPIP;
	uint8_t dottedQuad[4];


	pTCPUDPIP = (TCPUDPIP*)pObjectData;


	if (argc != 2)
	{
		pShell->printf("Usage: %s <ipaddr>\n", argv[0]);
		bOKToContinue = false;
	}



	if (bOKToContinue)
	{
		bOKToContinue = ParseIPv4Address(pShell, argv[1], &dottedQuad[0], &dottedQuad[1], &dottedQuad[2], &dottedQuad[3]);
	}



	if (bOKToContinue)
	{

		//When we delete a multicast address, the HW will send out an IGMP Leave Request.  However the HW emulation framework
		//does not yet fully support this.  To prevent errors being caused in the HW emulation, we will skip the actual API call
		//when we are running in emulation mode.
#ifndef XCL_EMULATION_MODE
		retval = pTCPUDPIP->DeleteIPv4MulticastAddress(dottedQuad[0], dottedQuad[1], dottedQuad[2], dottedQuad[3]);
#else
		pShell->printf("[INFO] Skipping multicast address delete in emulation mode\n");
#endif
		

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] Failed to delete multicast address, error = %s (0x%08X)\n", TCP_UDP_IP_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}


	return retval;
}


















static int TCP_UDP_IP_AddListeningPort(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	TCPUDPIP* pTCPUDPIP;
	uint16_t port;


	pTCPUDPIP = (TCPUDPIP*)pObjectData;


	if (argc != 2)
	{
		pShell->printf("Usage: %s <portnum>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = ParsePort(pShell, argv[1], &port);
	}


	if (bOKToContinue)
	{
		retval = pTCPUDPIP->AddUDPListeningPort(port);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", TCP_UDP_IP_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}


	return retval;
}









static int TCP_UDP_IP_DeleteListeningPort(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	TCPUDPIP* pTCPUDPIP;
	uint16_t port;


	pTCPUDPIP = (TCPUDPIP*)pObjectData;


	if (argc != 2)
	{
		pShell->printf("Usage: %s <portnum>\n", argv[0]);
		bOKToContinue = false;
	}




	if (bOKToContinue)
	{
		bOKToContinue = ParsePort(pShell, argv[1], &port);
	}




	if (bOKToContinue)
	{
		retval = pTCPUDPIP->DeleteUDPListeningPort(port);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", TCP_UDP_IP_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}






static int TCP_UDP_IP_DeleteAllListeningPorts(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = 0;
	bool bOKToContinue = true;
	TCPUDPIP* pTCPUDPIP;

	XLNX_UNUSED_ARG(argc);
	XLNX_UNUSED_ARG(argv);

	pTCPUDPIP = (TCPUDPIP*)pObjectData;




	if (bOKToContinue)
	{
		retval = pTCPUDPIP->DeleteAllUDPListeningPorts();

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", TCP_UDP_IP_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}


	return retval;
}








static int TCP_UDP_IP_SetICMPEnabled(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	bool bOKToContinue = true;
	TCPUDPIP* pTCPUDPIP;
	bool bEnabled = false;

	pTCPUDPIP = (TCPUDPIP*)pObjectData;

	if (argc != 2)
	{
		pShell->printf("Usage: %s <bool>\n", argv[0]);
		bOKToContinue = false;
	}

	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[1], &bEnabled);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}




	if (bOKToContinue)
	{
		retval = pTCPUDPIP->SetICMPEnabled(bEnabled);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] retval = %s (0x%08X)\n", TCP_UDP_IP_ErrorCodeToString(retval), retval);
		}
	}

	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}


	return retval;
}














static int TCP_UDP_IP_PrintIGMPTable(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	TCPUDPIP* pTCPUDPIP;
	uint32_t i;
	uint8_t a, b, c, d;
	uint32_t numConfiguredAddresses = 0;
	static const uint32_t BUFFER_SIZE = 32;
	char ipAddrStringBuffer[BUFFER_SIZE + 1];

	pTCPUDPIP = (TCPUDPIP*)pObjectData;

	XLNX_UNUSED_ARG(argc);
	XLNX_UNUSED_ARG(argv);



	pShell->printf("+-------+--------------------+\n");
	pShell->printf("| Index |  Multicast Address |\n");
	pShell->printf("+-------+--------------------+\n");

#ifndef XCL_EMULATION_MODE
	for (i = 0; i < TCPUDPIP::NUM_MULTICAST_ADDRESSES_SUPPORTED; i++)
	{
		retval = pTCPUDPIP->ReadIGMPEntry(i, &a, &b, &c, &d);

		if (retval == XLNX_OK)
		{
			if ((a != 0) || (b != 0) || (c != 0) || (d != 0))
			{
				numConfiguredAddresses++;

				snprintf(ipAddrStringBuffer, BUFFER_SIZE, "%u.%u.%u.%u", a, b, c, d);

				pShell->printf("|  %2u   | %18s |\n", i, ipAddrStringBuffer);
			}
		}

		if (retval != XLNX_OK)
		{
			break; //out of loop
		}
	}


	if (numConfiguredAddresses == 0)
	{
		pShell->printf("|  No Addresses Configured   |\n");
	}
#else
	pShell->printf("| Skipping in emulation mode |\n");
#endif

	pShell->printf("+-------+--------------------+\n");


	return retval;
}






static int TCP_UDP_IP_PrintStats(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	TCPUDPIP* pTCPUDPIP;
	TCPUDPIP::Stats statsCounters;

	XLNX_UNUSED_ARG(argc);
	XLNX_UNUSED_ARG(argv);

	pTCPUDPIP = (TCPUDPIP*)pObjectData;


	//Read the stats from HW...
	retval = pTCPUDPIP->GetStats(&statsCounters);


	if (retval == XLNX_OK)
	{
		pShell->printf("\n");
		pShell->printf("+-------+----------------------------+------------+\n");
		pShell->printf("| Block | Stats Counter              |    Value   | \n");
		pShell->printf("+-------+----------------------------+------------+\n");
		pShell->printf("| IGMP  | Invalid Checksum           | %10u |\n", statsCounters.igmp.numInvalidChecksum);
		pShell->printf("|       | IGMP Queries               | %10u |\n", statsCounters.igmp.numIGMPQueries);
		pShell->printf("|       | Invalid Queries            | %10u |\n", statsCounters.igmp.numInvalidQueries);
		pShell->printf("+-------+----------------------------+------------+\n");
		pShell->printf("| ARP   | Requests Sent              | %10u |\n", statsCounters.arp.numRequestsSent);
		pShell->printf("|       | Replies Sent               | %10u |\n", statsCounters.arp.numRepliesSent);
		pShell->printf("|       | Requests Received          | %10u |\n", statsCounters.arp.numRequestsReceived);
		pShell->printf("|       | Replies Received           | %10u |\n", statsCounters.arp.numRepliesReceived);
		pShell->printf("|       | Requests Sent Lost         | %10u |\n", statsCounters.arp.numRequestsSentLost);
		pShell->printf("+-------+----------------------------+------------+\n");
		pShell->printf("| ICMP  | Echo Requests Received     | %10u |\n", statsCounters.icmp.numEchoRequestsReceived);
		pShell->printf("|       | Echo Replies Sent          | %10u |\n", statsCounters.icmp.numEchoRepliesSent);
		pShell->printf("+-------+----------------------------+------------+\n");
		pShell->printf("| IPH   | In Header Errors           | %10u |\n", statsCounters.ipHandler.numInHeaderErrors);
		pShell->printf("|       | In Delivers                | %10u |\n", statsCounters.ipHandler.numInDelivers);
		pShell->printf("|       | In Unknown Protocols       | %10u |\n", statsCounters.ipHandler.numInUnknownProtocols);
		pShell->printf("|       | In Address Errors          | %10u |\n", statsCounters.ipHandler.numInAddressErrors);
		pShell->printf("|       | In Received                | %10u |\n", statsCounters.ipHandler.numInReceives);
		pShell->printf("+-------+----------------------------+------------+\n");
		pShell->printf("| MIE   | IPv4 Packets Sent          | %10u |\n", statsCounters.macIPEncoder.numIPv4PacketsSent);
		pShell->printf("|       | Packets Dropped            | %10u |\n", statsCounters.macIPEncoder.numPacketsDropped);
		pShell->printf("+-------+----------------------------+------------+\n");
		
		if (statsCounters.udp.bStatsValid)
		{
			pShell->printf("| UDP   | Datagrams Tx               | %10u |\n", statsCounters.udp.numTxDatagrams);
			pShell->printf("|       | Datagrams Rx               | %10u |\n", statsCounters.udp.numRxDatagrams);
			pShell->printf("|       | Datagrams Rx Invalid Port  | %10u |\n", statsCounters.udp.numRxDatagramsInvalidPort);
		}
		else
		{
			pShell->printf("| UDP   | Datagrams Tx               | %10s |\n", NOT_AVAILABLE_STRING);
			pShell->printf("|       | Datagrams Rx               | %10s |\n", NOT_AVAILABLE_STRING);
			pShell->printf("|       | Datagrams Rx Invalid Port  | %10s |\n", NOT_AVAILABLE_STRING);
		}
		
		pShell->printf("+-------+----------------------------+------------+\n");


		if (statsCounters.tcp.bStatsValid)
		{
			pShell->printf("| TCP   | In Segments                | %10u |\n", statsCounters.tcp.numInSegments);
			pShell->printf("|       | In Errors                  | %10u |\n", statsCounters.tcp.numInErrors);
			pShell->printf("|       | Out Segments               | %10u |\n", statsCounters.tcp.numOutSegments);
			pShell->printf("|       | Retransmitted Segments     | %10u |\n", statsCounters.tcp.numRetransmittedSegments);
			pShell->printf("|       | Active Opens               | %10u |\n", statsCounters.tcp.numActiveOpens);
			pShell->printf("|       | Passive Opens              | %10u |\n", statsCounters.tcp.numPassiveOpens);
			pShell->printf("|       | Attempt Fails              | %10u |\n", statsCounters.tcp.numAttemptFails);
			pShell->printf("|       | Establish Resets           | %10u |\n", statsCounters.tcp.numEstablishResets);
			pShell->printf("|       | Current Established        | %10u |\n", statsCounters.tcp.numCurrentEstablished);
		}
		else
		{
			pShell->printf("| TCP   | In Segments                | %10s |\n", NOT_AVAILABLE_STRING);
			pShell->printf("|       | In Errors                  | %10s |\n", NOT_AVAILABLE_STRING);
			pShell->printf("|       | Out Segments               | %10s |\n", NOT_AVAILABLE_STRING);
			pShell->printf("|       | Retransmitted Segments     | %10s |\n", NOT_AVAILABLE_STRING);
			pShell->printf("|       | Active Opens               | %10s |\n", NOT_AVAILABLE_STRING);
			pShell->printf("|       | Passive Opens              | %10s |\n", NOT_AVAILABLE_STRING);
			pShell->printf("|       | Attempt Fails              | %10s |\n", NOT_AVAILABLE_STRING);
			pShell->printf("|       | Establish Resets           | %10s |\n", NOT_AVAILABLE_STRING);
			pShell->printf("|       | Current Established        | %10s |\n", NOT_AVAILABLE_STRING);
		}
		
		pShell->printf("+-------+----------------------------+------------+\n");
	}
	


	if(retval != XLNX_OK)
	{
		pShell->printf("[ERROR] retval = %s (0x%08X)\n", TCP_UDP_IP_ErrorCodeToString(retval), retval);
	}


	return retval;
}









static int TCP_UDP_IP_PrintARPTable(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	TCPUDPIP* pTCPUDPIP;
	TCPUDPIP::ARPEntry arpEntry;
	bool bEntryValid = false;
	uint32_t i;
	static const uint32_t BUFFER_SIZE = 32;
	char formatBuffer[BUFFER_SIZE + 1];
	uint32_t numValidEntries = 0;

	XLNX_UNUSED_ARG(argc);
	XLNX_UNUSED_ARG(argv);

	pTCPUDPIP = (TCPUDPIP*)pObjectData;


	pShell->printf("+-------+-----------------+-------------------+\n");
	pShell->printf("| Index |   IP Address    |    MAC Address    |\n");
	pShell->printf("+-------+-----------------+-------------------+\n");

	for (i = 0; i < TCPUDPIP::NUM_ARP_ENTRIES_SUPPORTED; i++)
	{
		retval = pTCPUDPIP->ReadARPEntry(i, &arpEntry, &bEntryValid);
		
		if (retval == XLNX_OK)
		{
			if (bEntryValid)
			{
				numValidEntries++;

				//need to format IP address into a seperate buffer.
				//the string width can vary (depending on the address values)
				//its easier to format first into a string, then pad that string to the correct width later
				snprintf(formatBuffer, BUFFER_SIZE, "%u.%u.%u.%u", arpEntry.IPAddress.a,
													 arpEntry.IPAddress.b,
													 arpEntry.IPAddress.c,
													 arpEntry.IPAddress.d);

				pShell->printf("|  %3u  | %15s | %02x:%02x:%02x:%02x:%02x:%02x |\n", i, formatBuffer,
																						arpEntry.MACAddress.a,
																						arpEntry.MACAddress.b,
																						arpEntry.MACAddress.c,
																						arpEntry.MACAddress.d,
																						arpEntry.MACAddress.e,
																						arpEntry.MACAddress.f);
			}
			
		}
		else
		{
			break; //out of loop
		}
	}

	if (numValidEntries == 0)
	{
		pShell->printf("No Valid ARP Entries Found!\n");
	}


	pShell->printf("+-------+-----------------+-------------------+\n");
	

	return retval;
}








static int TCP_UDP_IP_AddARPEntry(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	TCPUDPIP* pTCPUDPIP;
	bool bOKToContinue = true;
	TCPUDPIP::ARPEntry arpEntry;

	pTCPUDPIP = (TCPUDPIP*)pObjectData;

	if (argc != 3)
	{
		pShell->printf("Usage: %s <ipaddr> <macaddr>\n", argv[0]);
		bOKToContinue = false;
	}


	if (bOKToContinue)
	{
		bOKToContinue = ParseIPv4Address(pShell, argv[1], &arpEntry.IPAddress.a,
														  &arpEntry.IPAddress.b, 
														  &arpEntry.IPAddress.c, 
														  &arpEntry.IPAddress.d);
	}


	if (bOKToContinue)
	{
		bOKToContinue = ParseMACAddress(pShell, argv[2], &arpEntry.MACAddress.a,
														 &arpEntry.MACAddress.b, 
														 &arpEntry.MACAddress.c, 
														 &arpEntry.MACAddress.d, 
														 &arpEntry.MACAddress.e, 
														 &arpEntry.MACAddress.f);
	}



	if (bOKToContinue)
	{
		retval = pTCPUDPIP->AddARPEntry(&arpEntry);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] Failed add ARP entry, error = %s (0x%08X)\n", TCP_UDP_IP_ErrorCodeToString(retval), retval);
		}
	}



	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}










static int TCP_UDP_IP_DeleteARPEntry(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	TCPUDPIP* pTCPUDPIP;
	bool bOKToContinue = true;
	uint32_t index;

	pTCPUDPIP = (TCPUDPIP*)pObjectData;

	if (argc != 2)
	{
		pShell->printf("Usage: %s <index>\n", argv[0]);
		bOKToContinue = false;
	}



	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseUInt32(argv[1], &index);
	}


	if (bOKToContinue)
	{
		retval = pTCPUDPIP->DeleteARPEntry(index);

		if (retval == XLNX_OK)
		{
			pShell->printf("OK\n");
		}
		else
		{
			bOKToContinue = false;
			pShell->printf("[ERROR] Failed to delete ARP entry, error = %s (0x%08X)\n", TCP_UDP_IP_ErrorCodeToString(retval), retval);
		}
	}


	if (bOKToContinue == false)
	{
		retval = Shell::COMMAND_PARSING_ERROR;
	}

	return retval;
}




static int TCP_UDP_IP_DeleteAllARPEntries(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	TCPUDPIP* pTCPUDPIP;

	XLNX_UNUSED_ARG(argc);
	XLNX_UNUSED_ARG(argv);

	pTCPUDPIP = (TCPUDPIP*)pObjectData;

	retval = pTCPUDPIP->DeleteAllARPEntries();

	if (retval == XLNX_OK)
	{
		pShell->printf("OK\n");
	}
	else
	{
		pShell->printf("[ERROR] Failed to delete ARP entries, error = %s (0x%08X)\n", TCP_UDP_IP_ErrorCodeToString(retval), retval);
	}

	return retval;
}







static int TCP_UDP_IP_GetStatus(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	bool bIsInitialised = false;
	uint32_t cuIndex;
	uint64_t cuAddress;
	bool bUDPSynthesized;
	bool bTCPSynthesized;
	uint8_t MACAddress[6];
	uint8_t IPv4Address[4];
	TCPUDPIP* pTCPUDPIP;
	uint32_t numEnabledListeningPorts = 0;
	bool bICMPEnabled = false;
	bool bIGMPEnabled = false;
	uint32_t IGMPVersion = 0;
	static const uint32_t BUFFER_SIZE = 32;
	char ipAddressFormatBuffer[BUFFER_SIZE + 1];

	XLNX_UNUSED_ARG(argc);
	XLNX_UNUSED_ARG(argv);

	pTCPUDPIP = (TCPUDPIP*)pObjectData;


	pTCPUDPIP->IsInitialised(&bIsInitialised);
	if(bIsInitialised == false)
	{
		retval = XLNX_TCP_UDP_IP_ERROR_NOT_INITIALISED;
	}




	if (retval == XLNX_OK)
	{
		pShell->printf("+--------------------+----------------------+\n");
	}
	
	

	if (retval == XLNX_OK)
	{
		retval = pTCPUDPIP->GetCUIndex(&cuIndex);
		if (retval == XLNX_OK)
		{
			pShell->printf("| CU Index           | %20u |\n", cuIndex);
		}
	}



	if (retval == XLNX_OK)
	{
		retval = pTCPUDPIP->GetCUAddress(&cuAddress);
		if (retval == XLNX_OK)
		{
			pShell->printf("| CU Address         |   0x%016" PRIX64 " |\n", cuAddress);
		}
	}
		

	if (retval == XLNX_OK)
	{
		pShell->printf("+--------------------+----------------------+\n");
	}


	if (retval == XLNX_OK)
	{
		pTCPUDPIP->IsTCPSynthesized(&bTCPSynthesized);
		pShell->printf("| TCP Synthesized    | %20s |\n", pShell->boolToString(bTCPSynthesized));
	}

	if (retval == XLNX_OK)
	{
		pTCPUDPIP->IsUDPSynthesized(&bUDPSynthesized);
		pShell->printf("| UDP Synthesized    | %20s |\n", pShell->boolToString(bUDPSynthesized));
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("+--------------------+----------------------+\n");
	}



	if(retval == XLNX_OK)
	{ 
		retval = pTCPUDPIP->GetMACAddress(&MACAddress[0], &MACAddress[1], &MACAddress[2], &MACAddress[3], &MACAddress[4], &MACAddress[5]);
		if (retval == XLNX_OK)
		{
			pShell->printf("| MAC Address        |    %02X:%02X:%02X:%02X:%02X:%02X |\n", MACAddress[0],
																					      MACAddress[1],
																					      MACAddress[2],
																					      MACAddress[3],
																					      MACAddress[4],
																					      MACAddress[5]);
			
		}
	}
		



	if (retval == XLNX_OK)
	{
		retval = pTCPUDPIP->GetIPv4Address(&IPv4Address[0], &IPv4Address[1], &IPv4Address[2], &IPv4Address[3]);
		if (retval == XLNX_OK)
		{
			snprintf(ipAddressFormatBuffer, BUFFER_SIZE, "%u.%u.%u.%u", IPv4Address[0],
																		IPv4Address[1],
																		IPv4Address[2],
																		IPv4Address[3]);

			pShell->printf("| IPv4 Address       | %20s |\n", ipAddressFormatBuffer);
		}															 
	}




	if (retval == XLNX_OK)
	{
		retval = pTCPUDPIP->GetSubnetMask(&IPv4Address[0], &IPv4Address[1], &IPv4Address[2], &IPv4Address[3]);

		if (retval == XLNX_OK)
		{
			snprintf(ipAddressFormatBuffer, BUFFER_SIZE, "%u.%u.%u.%u", IPv4Address[0],
																		IPv4Address[1],
																		IPv4Address[2],
																		IPv4Address[3]);



			pShell->printf("| Subnet Mask        | %20s |\n", ipAddressFormatBuffer);
		}
	}







	if (retval == XLNX_OK)
	{
		retval = pTCPUDPIP->GetGatewayAddress(&IPv4Address[0], &IPv4Address[1], &IPv4Address[2], &IPv4Address[3]);

		if (retval == XLNX_OK)
		{
			snprintf(ipAddressFormatBuffer, BUFFER_SIZE, "%u.%u.%u.%u", IPv4Address[0],
																		IPv4Address[1],
																		IPv4Address[2],
																		IPv4Address[3]);



			pShell->printf("| Gateway Address    | %20s |\n", ipAddressFormatBuffer);
		}
	}


	if (retval == XLNX_OK)
	{
		pShell->printf("+--------------------+----------------------+\n");
	}




	if (retval == XLNX_OK)
	{
		retval = pTCPUDPIP->GetICMPEnabled(&bICMPEnabled);

		if (retval == XLNX_OK)
		{		
			pShell->printf("| ICMP Enabled       | %20s |\n", pShell->boolToString(bICMPEnabled));
		}
	}




	if (retval == XLNX_OK)
	{
		pShell->printf("+--------------------+----------------------+\n");
	}



	if (retval == XLNX_OK)
	{
		retval = pTCPUDPIP->GetIGMPEnabled(&bIGMPEnabled);

		if (retval == XLNX_OK)
		{
			pShell->printf("| IGMP Enabled       | %20s |\n", pShell->boolToString(bIGMPEnabled));
		}
	}


	if (retval == XLNX_OK)
	{
		retval = pTCPUDPIP->GetIGMPVersion(&IGMPVersion);

		if (retval == XLNX_OK)
		{
			pShell->printf("| IGMP Version       | %20u |\n", IGMPVersion);
		}
	}



	if (retval == XLNX_OK)
	{
		pShell->printf("+--------------------+----------------------+\n");
		pShell->printf("\n\n");
	}
	







	if (retval == XLNX_OK)
	{
		retval = TCP_UDP_IP_PrintIGMPTable(pShell, 0, NULL, pObjectData);
	}












	if (retval == XLNX_OK)
	{
		if (bUDPSynthesized)
		{
			//NOTE - DYNAMIC ALLOCATION of array - its just a bit too big to put on the stack...
			uint16_t* enabledPorts = nullptr;
			
			enabledPorts = new uint16_t[TCPUDPIP::NUM_LISTENING_PORTS_SUPPORTED];

			if (enabledPorts == nullptr)
			{
				pShell->printf("[ERROR] Failed to allocate port map buffer\n");
				return -1;
			}


#ifndef XCL_EMULATION_MODE
			//This call causes over 2000 register reads.  In emulation mode, this takes a LOOOONNGG time
			//so we will skip it and inform the user...
			retval = pTCPUDPIP->GetUDPListeningPorts(enabledPorts, &numEnabledListeningPorts);
#endif
			if (retval == XLNX_OK)
			{
				pShell->printf("\n\n");

				pShell->printf("+---------------------+\n");
				pShell->printf("| UDP Listening Ports |\n");
				pShell->printf("+---------------------+\n");

#ifndef XCL_EMULATION_MODE
				if (numEnabledListeningPorts > 0)
				{
					for (uint32_t i = 0; i < numEnabledListeningPorts; i++)
					{
						pShell->printf("| %-19u |\n", enabledPorts[i]);
					}
				}
				else
				{
					pShell->printf("| %-19s |\n", "No Ports Configured");
				}
#else		
				pShell->printf("Skipping in emulation mode\n");
#endif

				pShell->printf("+---------------------+\n");



			}


			//Free up our dynamically allocated buffer...
			if (enabledPorts != nullptr)
			{
				delete[] enabledPorts;
				enabledPorts = nullptr;
			}
		}
	}







	if (retval != XLNX_OK)
	{
		pShell->printf("[ERROR] retval = %s (0x%08X)\n", TCP_UDP_IP_ErrorCodeToString(retval), retval);
	}



	return retval;
}









static int TCP_UDP_UP_SetConfigAllowed(Shell* pShell, int argc, char* argv[], void* pObjectData)
{
	int retval = XLNX_OK;
	TCPUDPIP* pTCPUDPIP;
	bool bOKToContinue = true;
	bool bAllow = true;

	pTCPUDPIP = (TCPUDPIP*)pObjectData;


	if (argc != 2)
	{
		pShell->printf("Usage: %s <bool>\n", argv[0]);
		bOKToContinue = false;
	}



	if (bOKToContinue)
	{
		bOKToContinue = pShell->parseBool(argv[1], &bAllow);
		if (bOKToContinue == false)
		{
			pShell->printf("[ERROR] Failed to parse bool parameter\n");
		}
	}



	if (bOKToContinue)
	{
		pTCPUDPIP->SetConfigurationAllowed(bAllow);
		pShell->printf("OK\n");
	}

	return retval;
}













CommandTableElement XLNX_TCP_UDP_IP_COMMAND_TABLE[] =
{
	{"setmacaddr",			TCP_UDP_IP_SetMACAddress,					"<macaddr>",			"Set MAC address"							},
	{"setipaddr",			TCP_UDP_IP_SetIPv4Address,					"<ipaddr>",				"Set IPv4 address"							},
	{"setsubnetmask",		TCP_UDP_IP_SetSubnetMask,					"<ipmask>",				"Set subnet mask"							},
	{"setgateway",			TCP_UDP_IP_SetGateway,						"<ipaddr>",				"Set gateway IPv4 address"					},
	{/*-----------------------------------------------------------------------------------------------------------------------------------*/},
	{"setigmp",				TCP_UDP_IP_SetIGMPEnabled,					"<bool>",				"Enable/disable IGMP"						},
	{"setigmpver",			TCP_UDP_IP_SetIGMPVersion,					"<ver>",				"Set the IGMP version (2 or 3)"				},
	{"igmpprint",			TCP_UDP_IP_PrintIGMPTable,					"",						"Print the contents of the IGMP table"		},
	{"addmcast",			TCP_UDP_IP_AddMulticastIPv4Address,			"<ipaddr>",				"Add a multicast IP address"				},
	{"deletemcast",			TCP_UDP_IP_DeleteMulticastIP4vAddress,		"<ipaddr>",				"Delete a multicast IP address"				},
	{/*-----------------------------------------------------------------------------------------------------------------------------------*/},
	{"seticmp",				TCP_UDP_IP_SetICMPEnabled,					"<bool>",				"Enable/disable ICMP"						},
	{/*-----------------------------------------------------------------------------------------------------------------------------------*/},
	{"addport",				TCP_UDP_IP_AddListeningPort,				"<portnum>",			"Add a UDP listening port"					},
	{"deleteport",			TCP_UDP_IP_DeleteListeningPort,				"<portnum>",			"Delete a UDP listening port"				},
	{"deleteallports",		TCP_UDP_IP_DeleteAllListeningPorts,			"",						"Deletes ALL UDP listening ports"			},
	{/*-----------------------------------------------------------------------------------------------------------------------------------*/},
	{"arpprint",			TCP_UDP_IP_PrintARPTable,					"",						"Print the contents of the ARP table"		},
	{"arpadd",				TCP_UDP_IP_AddARPEntry,						"<ipaddr> <macaddr>",	"Add an ARP entry"							},
	{"arpdelete",			TCP_UDP_IP_DeleteARPEntry,					"<index>",				"Delete the ARP entry at specified index"	},
	{"arpdeleteall",		TCP_UDP_IP_DeleteAllARPEntries,				"",						"Delete ALL ARP entries"					},
	{/*-----------------------------------------------------------------------------------------------------------------------------------*/},
	{"printstats",			TCP_UDP_IP_PrintStats,						"",						"Prints stats counters",					},
	{"getstatus",			TCP_UDP_IP_GetStatus,						"",						"Get info on status of block"				},
	{"setconfigallowed",	TCP_UDP_UP_SetConfigAllowed,				"<bool>",				"Toggle config writes allowed"				}
};



const uint32_t XLNX_TCP_UDP_IP_COMMAND_TABLE_LENGTH = (uint32_t)(sizeof(XLNX_TCP_UDP_IP_COMMAND_TABLE) / sizeof(XLNX_TCP_UDP_IP_COMMAND_TABLE[0]));

