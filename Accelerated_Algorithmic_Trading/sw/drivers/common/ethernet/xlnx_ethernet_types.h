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



#ifndef XLNX_ETHERNET_TYPES_H
#define XLNX_ETHERNET_TYPES_H


namespace XLNX
{

enum class GTClockSelect
{
    STATIC_1,
    OUTCLKPCS,
    OUTCLKPMA,
    PLLREFCLK_DIV1,
    PLLREFCLK_DIV2,
    PROGDIVCLK
};



enum class GTEqualizationMode
{
    DFE,
    LPM
};



enum class GTLoopback
{
    NO_LOOPACK,	//<-- Normal Operation

    NEAR_END_PCS_LOOPBACK,
    NEAR_END_PMA_LOOPBACK,
    FAR_END_PMA_LOOPBACK,
    FAR_END_PCS_LOOPBACK,

};


enum class GTPolarity
{
    NORMAL,
    INVERTED
};






enum class GTTestPattern
{
   NO_PATTERN, //<-- Normal Operation

   PRBS_7,
   PRBS_9,
   PRBS_15,
   PRBS_23,
   PRBS_31
};




enum class GTRxBufStatus
{
    NOMINAL = 0,
    FILL_UNDER_CLK_COR_MIN_LAT = 1,
    FILL_OVER_CLK_COR_MAX_LAT = 2,
    RX_ELASTIC_BUFFER_UNDERFLOW = 5,
    RX_ELASTIC_BUFFER_OVERFLOW = 6,
};


} //end namespace XLNX


#endif // XLNX_ETHERNET_TYPES_H

