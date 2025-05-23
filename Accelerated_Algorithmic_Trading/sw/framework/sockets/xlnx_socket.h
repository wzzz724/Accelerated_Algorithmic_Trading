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



#ifndef XLNX_SOCKET_H
#define XLNX_SOCKET_H


#ifdef _WIN32

#include <winsock2.h>
#include <Ws2tcpip.h>

#endif




#ifdef __linux__

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>  /* Needed for getaddrinfo() and freeaddrinfo() */
#include <unistd.h> /* Needed for close() */

#endif









#ifdef __linux__

typedef int SOCKET;
#define INVALID_SOCKET  (-1)

#endif











int sockInit(void);
int sockQuit(void);

int sockClose(SOCKET sock);

















#endif //XLNX_SOCKET_H
