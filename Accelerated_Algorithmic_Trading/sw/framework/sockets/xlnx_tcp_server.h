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


#ifndef XLNX_TCP_SERVER_H
#define XLNX_TCP_SERVER_H




#include <stdint.h>
#include <thread>

#include "xlnx_socket.h"

namespace XLNX
{

class TCPServer
{

public:
    TCPServer();
    virtual ~TCPServer();

public:

    typedef void (*ConnectionEstablishedCallbackType)(SOCKET sock);

    void Start(uint16_t listeningPort, ConnectionEstablishedCallbackType callback);
    void StartAsThread(uint16_t listeningPort, ConnectionEstablishedCallbackType callback);
    void Stop(void);


public:
    static const uint32_t MAX_CONNECTIONS = 10;

protected:
    void InternalConnectionEstablishedCallback(SOCKET sock);

protected:

    bool m_bRunning;
    SOCKET m_listenSocket;

    ConnectionEstablishedCallbackType m_callback;

    std::thread m_thread;
    bool m_bRunningAsThread;


};





} //end namespace XLNX






#endif //XLNX_TCP_SERVER_H


