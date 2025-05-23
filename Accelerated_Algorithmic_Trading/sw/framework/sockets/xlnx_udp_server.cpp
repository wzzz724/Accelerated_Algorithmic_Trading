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


#include <stdio.h>
#include <string.h>

#include "xlnx_socket.h"
#include "xlnx_udp_server.h"
using namespace XLNX;


UDPServer::UDPServer()
{
	m_socket = INVALID_SOCKET;
	m_bRunning = false;
	m_packetIndex = 0;
	m_bRunningAsThread = false;
	
	m_receiveCallback = nullptr;
	m_pReceiveCallbackDataObject = nullptr;
}


UDPServer::~UDPServer()
{
	if (m_bRunning)
	{
		Stop();
	}
}




void UDPServer::Start(uint16_t listeningPort)
{
	bool bOKToContinue = true;
	int retval = 0;
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLength;

	clientAddrLength = sizeof(clientAddr);



	if (m_bRunning == false)
	{
		retval = sockInit();
		if (retval != 0)
		{
			printf("[ERROR] Failed to initialise socket library\n");
			bOKToContinue = false;
		}


		if (bOKToContinue)
		{
			m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (m_socket == INVALID_SOCKET)
			{
				printf("[ERROR] Failed to create socket\n");
				bOKToContinue = false;
			}
		}


		if (bOKToContinue)
		{
			memset(&serverAddr, '0', sizeof(serverAddr));
			serverAddr.sin_family = AF_INET;
			serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
			serverAddr.sin_port = htons(listeningPort);

			retval = bind(m_socket, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
			if (retval != 0)
			{
				printf("[ERROR] Socket bind call failed, errno = %u\n", errno);
				bOKToContinue = false;
			}
		}




		if (bOKToContinue)
		{
			m_bRunning = true;
			printf("Starting server...listening on port %u\n", listeningPort);

			while (m_bRunning)
			{
				//try to receive some data, this is a blocking call
				retval = recvfrom(m_socket, (char*)m_recvBuffer, MAX_RECEIVE_LENGTH, 0, (struct sockaddr*) &clientAddr, &clientAddrLength);
				if(retval > 0)
				{
					if (m_receiveCallback != nullptr)
					{
						(*m_receiveCallback)(m_pReceiveCallbackDataObject, m_recvBuffer, retval);
					}
					else
					{
						HexDumpPacket(m_packetIndex, m_recvBuffer, retval);
						
					}

					m_packetIndex++;
					
				}
				else
				{
					printf("[ERROR] Receive Error: 0x%08X\n", retval);
				}

			}

		}

	}

}


void UDPServer::StartAsThread(uint16_t listeningPort)
{
	if (!m_bRunning)
	{
		m_thread = std::thread(&UDPServer::Start, this, listeningPort);
		m_bRunningAsThread = true;
	}

}






void UDPServer::Stop(void)
{
	if (m_bRunning)
	{
		printf("Stopping server...\n");
		m_bRunning = false;

		if (m_socket != INVALID_SOCKET)
		{
			sockClose(m_socket);
			m_socket = INVALID_SOCKET;
		}


		sockQuit();

		if (m_bRunningAsThread)
		{
			m_thread.join();
			m_bRunningAsThread = false;
		}

	}
}












static const char* LINE_STRING = "------------------------------------------------------------------------------";


void UDPServer::HexDumpPacket(uint32_t packetIndex, uint8_t* pBuffer, uint32_t dataLength)
{
	const uint32_t numBytesPerRow = 16;
	uint32_t numRows;
	uint32_t i;
	uint32_t j;
	uint32_t bufferIndex;
	uint32_t numBytesToProcess;
	static const uint32_t BUFFER_SIZE = 256;
	char asciiFormatBuffer[BUFFER_SIZE + 1];
	char hexFormatBuffer[BUFFER_SIZE + 1];
	uint32_t bufferOffset;

	numRows = dataLength / numBytesPerRow;
	if (dataLength % numBytesPerRow != 0)
	{
		numRows++;
	}

	printf("\n");
	printf("+-%.18s-+-%.47s-+-%.16s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);
	printf("| %-18s | %-47s | %-16s |\n", "Packet Index", "Hex Data", "ASCII");
	printf("+-%.18s-+-%.47s-+-%.16s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);


	for (i = 0; i < numRows; i++)
	{
		bufferIndex = (i * numBytesPerRow);

		numBytesToProcess = dataLength - bufferIndex;
		if (numBytesToProcess > numBytesPerRow)
		{
			numBytesToProcess = numBytesPerRow;
		}


		bufferOffset = 0;
		for (j = 0; j < numBytesToProcess; j++)
		{
			if (j != 0)
			{
				bufferOffset += snprintf(&hexFormatBuffer[bufferOffset],BUFFER_SIZE, " ");
			}

			bufferOffset += snprintf(&hexFormatBuffer[bufferOffset], BUFFER_SIZE, "%02X", pBuffer[bufferIndex + j]);
		}




		bufferOffset = 0;
		for (j = 0; j < numBytesToProcess; j++)
		{
			uint8_t byte;
			char c;

			byte = pBuffer[bufferIndex + j];

			if (isprint(byte))
			{
				c = (char)byte;
			}
			else
			{
				c = '.';
			}

			bufferOffset += snprintf(&asciiFormatBuffer[bufferOffset], BUFFER_SIZE, "%c", c);
		}





		if (i == 0)
		{
			printf("| %18u | %-47s | %-16s |\n", packetIndex, hexFormatBuffer, asciiFormatBuffer);
		}
		else
		{
			printf("| %18s | %-47s | %-16s |\n", "", hexFormatBuffer, asciiFormatBuffer);
		}

	}


	printf("+-%.18s-+-%.47s-+-%.16s-+\n", LINE_STRING, LINE_STRING, LINE_STRING);





}



void UDPServer::SetReceiveCallback(ReceiveCallbackType callback, void* pDataObject)
{
	m_receiveCallback = callback;
	m_pReceiveCallbackDataObject = pDataObject;
}

