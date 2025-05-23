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

using namespace XLNX;



void TCPUDPIP::PackIPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint32_t* pRegValue)
{
    uint32_t value;

    value = 0;
    value |= a << 24;
    value |= b << 16;
    value |= c << 8;
    value |= d << 0;


    *pRegValue = value;

}





void TCPUDPIP::UnpackIPAddress(uint32_t regValue, uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d)
{ 
    *a = (regValue >> 24) & 0xFF;
    *b = (regValue >> 16) & 0xFF;
    *c = (regValue >> 8) & 0xFF;
    *d = (regValue >> 0) & 0xFF;
}







void TCPUDPIP::PackMACAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f, uint32_t* pUpperRegValue, uint32_t* pLowerRegValue)
{
    uint32_t value;

    value = 0;
    value |= a << 8;
    value |= b << 0;
   
    *pUpperRegValue = value;




    value = 0;
    value |= c << 24;
    value |= d << 16;
    value |= e << 8;
    value |= f << 0;

    *pLowerRegValue = value;

}




void TCPUDPIP::UnpackMACAddress(uint32_t upperRegValue, uint32_t lowerRegValue, uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d, uint8_t* e, uint8_t* f)
{
   
    *a = (upperRegValue >> 8) & 0xFF;
    *b = (upperRegValue >> 0) & 0xFF;

    *c = (lowerRegValue >> 24) & 0xFF;
    *d = (lowerRegValue >> 16) & 0xFF;
    *e = (lowerRegValue >> 8) & 0xFF;
    *f = (lowerRegValue >> 0) & 0xFF;
}







