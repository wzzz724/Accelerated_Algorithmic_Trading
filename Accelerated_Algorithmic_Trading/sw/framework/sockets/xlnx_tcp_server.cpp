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
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <thread>


#include "xlnx_socket.h"
#include "xlnx_tcp_server.h"
using namespace XLNX;








TCPServer::TCPServer()
{
    m_bRunning = false;

    m_listenSocket = INVALID_SOCKET;

    m_bRunningAsThread = false;

    m_callback = nullptr;

}





TCPServer::~TCPServer()
{
    if (m_bRunning)
    {
        Stop();
    }
}






void TCPServer::Start(uint16_t listeningPort, ConnectionEstablishedCallbackType callback)
{
    struct sockaddr_in serv_addr;
    int retval = 0;
    bool bOKToContinue = true;

    SOCKET connectionSocket;




    if (!m_bRunning)
    {
        m_callback = callback;

        printf("Starting server...listening on port %u\n", listeningPort);
        m_bRunning = true;



        retval = sockInit();
        if (retval != 0)
        {
            printf("[ERROR] Failed to initialise socket library\n");
            bOKToContinue = false;
        }





        if (bOKToContinue)
        {
            m_listenSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (m_listenSocket == INVALID_SOCKET)
            {
                printf("[ERROR] Failed to create listening socket\n");
                bOKToContinue = false;
            }
        }








        if (bOKToContinue)
        {
            int enable = 1;
            retval = setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));
            if (retval != 0)
            {
                printf("[ERROR] Failed to set SO_REUSEADDR socket option, errno = %u\n", errno);
                bOKToContinue = false;
            }
        }








        if (bOKToContinue)
        {
            memset(&serv_addr, '0', sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            serv_addr.sin_port = htons(listeningPort);

            retval = bind(m_listenSocket, (struct sockaddr*) & serv_addr, sizeof(serv_addr));
            if (retval != 0)
            {
                printf("[ERROR] Socket bind call failed, errno = %u\n", errno);
                bOKToContinue = false;
            }
        }




        if (bOKToContinue)
        {
            retval = listen(m_listenSocket, MAX_CONNECTIONS);
            if (retval != 0)
            {
                printf("[ERROR] Socket listen call failed, errno = %u\n", errno);
                bOKToContinue = false;
            }
        }



        if (bOKToContinue)
        {
            while (m_bRunning)
            {
                connectionSocket = accept(m_listenSocket, (struct sockaddr*)NULL, NULL);

                if (connectionSocket != INVALID_SOCKET)
                {
                    std::thread t = std::thread(&TCPServer::InternalConnectionEstablishedCallback, this, connectionSocket);
                    t.detach();
                }
            }
        }
    }
}




void TCPServer::StartAsThread(uint16_t listeningPort, ConnectionEstablishedCallbackType callback)
{
    if (!m_bRunning)
    {
        m_thread = std::thread(&TCPServer::Start, this, listeningPort, callback);
        m_bRunningAsThread = true;
    }

}






void TCPServer::Stop(void)
{
    if (m_bRunning)
    {
        printf("Stopping Server...\n");
        m_bRunning = false;

        if (m_listenSocket != INVALID_SOCKET)
        {
            sockClose(m_listenSocket);
            m_listenSocket = INVALID_SOCKET;
        }


        sockQuit();


        if (m_bRunningAsThread)
        {
            m_thread.join();
            m_bRunningAsThread = false;
        }


        m_callback = nullptr;
    }




}





void TCPServer::InternalConnectionEstablishedCallback(SOCKET sock)
{
    if (m_callback != nullptr)
    {
        (*m_callback)(sock);
    }


    //once this callback returns, we will assume the user has finished with the socket
    //so we can close it...
    sockClose(sock);

}




