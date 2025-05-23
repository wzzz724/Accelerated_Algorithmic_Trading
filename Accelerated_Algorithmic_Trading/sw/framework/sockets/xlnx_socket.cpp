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


#include "xlnx_socket.h"



int sockInit(void)
{
    int retval = 0;

#ifdef _WIN32
    WSADATA wsaData;
    retval =  WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    return retval;
}





int sockQuit(void)
{
    int retval = 0;

#ifdef _WIN32
    retval = WSACleanup();
#endif

    return retval;
}







int sockClose(SOCKET sock)
{
    int retval = 0;

#ifdef _WIN32
    retval = shutdown(sock, SD_BOTH);

    if (retval == 0)
    {
        retval = closesocket(sock);
    }

#endif


#ifdef __linux__

    retval = shutdown(sock, SHUT_RDWR);

    if (retval == 0)
    {
        retval = close(sock);
    }

#endif

    return retval;
}

