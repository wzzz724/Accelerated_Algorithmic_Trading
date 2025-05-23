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


#ifndef XLNX_AAT_H
#define XLNX_AAT_H


#include "xlnx_device_interface.h"

#include "xlnx_ethernet.h"
#include "xlnx_tcp_udp_ip.h"
#include "xlnx_feed_handler.h"
#include "xlnx_order_book.h"
#include "xlnx_order_book_data_mover.h"
#include "xlnx_pricing_engine.h"
#include "xlnx_order_entry.h"
#include "xlnx_clock_tick_generator.h"
#include "xlnx_line_handler.h"

#include "xlnx_network_capture.h"
#include "xlnx_network_tap.h"


namespace XLNX
{

class AAT //Accelerated Algorithmic Trading
{

public:
	AAT();
	virtual ~AAT();


public:
	uint32_t Initialise(DeviceInterface* pDeviceInterface);
    uint32_t Uninitialise(void);




public:
    uint32_t StartDataMover(void);





public:
	uint32_t StartCapture(char* filepath);
	uint32_t StopCapture(void);
	uint32_t IsCaptureRunning(bool* pbIsRunning);



public:
	void GetSubBlockError(uint32_t* pErrorCode);
protected:
	void SetSubBlockError(uint32_t errorCode);



protected:
	uint32_t SetMACAddressesFromNVRAM(DeviceInterface* pDeviceInterface);

	//With the addition of line arbitration functionality, the ATRD design now needs 3 MAC addresses (2 for ingress, 1 for egress).
	//However, some older cards (mainly U200/U250) only have 2 addresses programmed into their NVRAM.
	//The following function will check the number of available MAC addresses, and will DISABLE CONFIGURATION of the relevant
	//TCPUDPIP block and the relevant Ethernet channel.
	void CheckForSufficientMACAddresses(MACAddresses* pMACAddresses);






public:
	Ethernet		    ethernet;
	TCPUDPIP		    ingressTCPUDPIP0;
	TCPUDPIP			ingressTCPUDPIP1;
	TCPUDPIP		    egressTCPUDPIP;
	FeedHandler		    feedHandler;
	OrderBook		    orderBook;
    OrderBookDataMover  dataMover;
    PricingEngine	    pricingEngine;
    OrderEntry		    orderEntry;
    ClockTickGenerator  clockTickGenerator;
	LineHandler			lineHandler;

	//For debug use only
	NetworkCapture      networkCapture;
	NetworkTap			networkTap;




protected:
	uint32_t            m_subBlockErrorCode;

};



}




#endif //XLNX_AAT_H

