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


#ifndef XLNX_UDP_SERVER_H
#define XLNX_UDP_SERVER_H

#include <stdint.h>

#include <thread>

#include "xlnx_socket.h"

namespace XLNX
{

class UDPServer
{

public:
	UDPServer();
	virtual ~UDPServer();



public:
	void Start(uint16_t listeningPort);
	void StartAsThread(uint16_t listeningPort);
	void Stop(void);


public:
    typedef void (*ReceiveCallbackType)(void* pDataObject, uint8_t* pBuffer, uint32_t dataLength);
    void SetReceiveCallback(ReceiveCallbackType callback, void* pDataObject);

public:
	static void HexDumpPacket(uint32_t packetIndex, uint8_t* pBuffer, uint32_t dataLength);


protected:
	SOCKET m_socket;
	bool m_bRunning;

	static const uint32_t MAX_RECEIVE_LENGTH = 1024;
	uint8_t m_recvBuffer[MAX_RECEIVE_LENGTH];

	uint32_t m_packetIndex;

	std::thread m_thread;
	bool m_bRunningAsThread;


protected:
    ReceiveCallbackType m_receiveCallback;
    void* m_pReceiveCallbackDataObject;

};



} //end namespace XLNX


#endif //end XLNX_UDP_SERVER_H



