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

#include <cstring> //for memcpy..

#include "xlnx_aat.h"
#include "xlnx_aat_error_codes.h"
using namespace XLNX;





static const char* ETHERNET_CU_NAME					= "ethernet_krnl_axis_x4:eth0";
static const char* ALT_ETHERNET_CU_NAME				= "ethernet_krnl_axis_x2:eth0";


static const char* UDP_IP_CU_NAME					= "udp_ip_krnl:udp_ip0";	//ingress comms block is always UDP

//The following are to cope with the case where we have line arbitration in the HW build...i.e. multiple ingress comms blocks
static const char* UDP_IP_0_CU_NAME					= "udp_ip_krnl:udp_ip0";
static const char* UDP_IP_1_CU_NAME					= "udp_ip_krnl:udp_ip1";

static const char* FEED_HANDLER_CU_NAME				= "feedHandlerTop:feedHandlerTop";

static const char* ORDER_BOOK_CU_NAME				= "orderBookTop:orderBookTop";

static const char* ORDER_BOOK_DATA_MOVER_CU_NAME	= "orderBookDataMoverTop:orderBookDataMoverTop";

static const char* PRICING_ENGINE_CU_NAME			= "pricingEngineTop:pricingEngineTop";

static const char* ORDER_ENTRY_CU_NAME				= "orderEntryTop:orderEntryTop";		//original name
static const char* ORDER_ENTRY_UDP_CU_NAME			= "orderEntryUdpTop:orderEntryUdpTop";	//new name when attached to UDPIP block
static const char* ORDER_ENTRY_TCP_CU_NAME			= "orderEntryTcpTop:orderEntryTcpTop";	//new name when attached to TCPIP block

static const char* TCP_IP_CU_NAME					= "tcp_ip_krnl:tcp_ip0";	//egress comms block can either be TCP or UDP depending on how the HW is built...
static const char* UDP_IP_CU_NAME_2					= "udp_ip_krnl:udp_ip2";

static const char* CLOCK_TICK_GENERATOR_CU_NAME		= "clockTickGeneratorTop:clockTickGeneratorTop";

static const char* LINE_HANDLER_CU_NAME				= "lineHandlerTop:lineHandlerTop";

static const char* NETWORK_CAPTURE_CU_NAME          = "networkCaptureTop:networkCaptureTop"; 
static const char* NETWORK_TAP_CU_NAME              = "networkTap:networkTap";


//
//Accelerated Algorithmic Trading
//

AAT::AAT()
{
    m_subBlockErrorCode = XLNX_OK;
}





AAT::~AAT()
{


}






uint32_t AAT::Initialise(DeviceInterface* pDeviceInterface)
{
    uint32_t retval = XLNX_OK;

    m_subBlockErrorCode = XLNX_OK;


    //TODO probably want to stop initialising other blocks if one of them fail to initialise.
    //but currently we have different loads that contain different combinations of blocks.
    //So it is currently easier to try to attempt to initialise them all...

    retval = ethernet.Initialise(pDeviceInterface, ETHERNET_CU_NAME);

    if (retval == XLNX_ETHERNET_ERROR_CU_NAME_NOT_FOUND)
    {
        //try the alternate name
        retval = ethernet.Initialise(pDeviceInterface, ALT_ETHERNET_CU_NAME);
    }



    //first try the normal (single line) ingress kernel name..
    retval = ingressTCPUDPIP0.Initialise(pDeviceInterface, UDP_IP_CU_NAME);


    //if it didn't work...we may have a build with line arbitration in it
    //in which case we will have 2 ingress UDP blocks...which have different names
    if (retval == XLNX_TCP_UDP_IP_ERROR_CU_NAME_NOT_FOUND)
    {
        //try the first (0) multi-line ingress kernel name
        retval = ingressTCPUDPIP0.Initialise(pDeviceInterface, UDP_IP_0_CU_NAME);
    }


    //always try the second (1) multi-line ingress kernel name...
    //it will FAIL if we don't have a HW build with line arbitration in it.
    retval = ingressTCPUDPIP1.Initialise(pDeviceInterface, UDP_IP_1_CU_NAME);





    retval = feedHandler.Initialise(pDeviceInterface, FEED_HANDLER_CU_NAME);




    retval = orderBook.Initialise(pDeviceInterface, ORDER_BOOK_CU_NAME);



    retval = dataMover.Initialise(pDeviceInterface, ORDER_BOOK_DATA_MOVER_CU_NAME);




    retval = pricingEngine.Initialise(pDeviceInterface, PRICING_ENGINE_CU_NAME);







    //first we will try the original name...
    retval = orderEntry.Initialise(pDeviceInterface, ORDER_ENTRY_CU_NAME);

    //...if it didn't work try the new UDP name...
    if (retval == XLNX_ORDER_ENTRY_ERROR_CU_NAME_NOT_FOUND)
    {
        retval = orderEntry.Initialise(pDeviceInterface, ORDER_ENTRY_UDP_CU_NAME);
    }

    //...and if it still didn't work try the new TCP name...
    if (retval == XLNX_ORDER_ENTRY_ERROR_CU_NAME_NOT_FOUND)
    {
        retval = orderEntry.Initialise(pDeviceInterface, ORDER_ENTRY_TCP_CU_NAME);
    }






    //first try the TCP kernel name...
    retval = egressTCPUDPIP.Initialise(pDeviceInterface, TCP_IP_CU_NAME);

    //...if it didn't try the UDP kernel name
    if (retval == XLNX_TCP_UDP_IP_ERROR_CU_NAME_NOT_FOUND)
    {
        retval = egressTCPUDPIP.Initialise(pDeviceInterface, UDP_IP_CU_NAME_2);
    }




    retval = clockTickGenerator.Initialise(pDeviceInterface, CLOCK_TICK_GENERATOR_CU_NAME);



    retval = lineHandler.Initialise(pDeviceInterface, LINE_HANDLER_CU_NAME);



    retval = networkCapture.Initialise(pDeviceInterface, NETWORK_CAPTURE_CU_NAME);


    retval = networkTap.Initialise(pDeviceInterface, NETWORK_TAP_CU_NAME);


    //Now we will set the MAC address registers in the IP blocks using the MAC addresses from the 
    //cards NVRAM (which is set in the factory)
    SetMACAddressesFromNVRAM(pDeviceInterface);


    return XLNX_OK;
}







uint32_t AAT::Uninitialise(void)
{
    uint32_t retval = XLNX_OK;

    //Currently the data mover kernel and the network capture kernel are the only ones that uses any DDR/HBM memory...
    retval = dataMover.Uninitialise();

    retval = StopCapture();

    return retval;
}






uint32_t AAT::SetMACAddressesFromNVRAM(DeviceInterface* pDeviceInterface)
{
    uint32_t retval = XLNX_OK;
    bool bEthernetObjectInitialised;
    bool bIngressObject0Initialised;
    bool bIngressObject1Initialised;
    bool bEgressObjectInitialised;
    bool bIngressMAC0Set = false;
    bool bIngressMAC1Set = false;
    bool bEgressMACSet = false;
    bool bSpareMACSet = false;
    MACAddresses* pMACAddresses;

    

    //In this function we retrieve the MAC addresses from the cards NVRAM and use them to set the 
    //MAC address registers in the relevant IP blocks...


    //Retrieve the addresses programmed in the cards NVRAM...
    pMACAddresses = pDeviceInterface->GetMACAddresses();
    

    //If we're running on real HW, the following function will DISABLE the secondary ingress interface
    //if the card has insufficient MAC addresses in its NVRAM
    //However, in EMULATION MODE, we want to skip this check and allow the user to configure
    //the interfaces any way they want.
#ifndef XCL_EMULATION_MODE
    CheckForSufficientMACAddresses(pMACAddresses);
#endif

    

    ethernet.IsInitialised(&bEthernetObjectInitialised);

    ingressTCPUDPIP0.IsInitialised(&bIngressObject0Initialised);

    ingressTCPUDPIP1.IsInitialised(&bIngressObject1Initialised);

    egressTCPUDPIP.IsInitialised(&bEgressObjectInitialised);



    for (uint32_t i = 0; i < MACAddresses::NUM_SUPPORTED_MAC_ADDRESSES; i++)
    {
        if (pMACAddresses->isValid[i])
        {	
            //Use first valid address for the INGRESS interface...

            if (bIngressMAC0Set == false)
            {
                if (bEthernetObjectInitialised)
                {
                    ethernet.SetMACFilterAddress(0, pMACAddresses->addr[i][0],
                                                    pMACAddresses->addr[i][1],
                                                    pMACAddresses->addr[i][2],
                                                    pMACAddresses->addr[i][3],
                                                    pMACAddresses->addr[i][4],
                                                    pMACAddresses->addr[i][5]);
                }


                if (bIngressObject0Initialised)
                {
                    ingressTCPUDPIP0.SetMACAddress(pMACAddresses->addr[i][0],
                                                   pMACAddresses->addr[i][1],
                                                   pMACAddresses->addr[i][2],
                                                   pMACAddresses->addr[i][3],
                                                   pMACAddresses->addr[i][4],
                                                   pMACAddresses->addr[i][5]);

                }
                    
                bIngressMAC0Set = true;
                continue;
            }


        

            //Use the second valid address for the EGRESS interface...
            if (bEgressMACSet == false)
            {
                if (bEthernetObjectInitialised)
                {
                    ethernet.SetMACFilterAddress(1, pMACAddresses->addr[i][0],
                                                    pMACAddresses->addr[i][1],
                                                    pMACAddresses->addr[i][2],
                                                    pMACAddresses->addr[i][3],
                                                    pMACAddresses->addr[i][4],
                                                    pMACAddresses->addr[i][5]);
                }
                

                if (bEgressObjectInitialised)
                { 
                    egressTCPUDPIP.SetMACAddress(pMACAddresses->addr[i][0],
                                                 pMACAddresses->addr[i][1],
                                                 pMACAddresses->addr[i][2],
                                                 pMACAddresses->addr[i][3],
                                                 pMACAddresses->addr[i][4],
                                                 pMACAddresses->addr[i][5]);
                }

                bEgressMACSet = true;
                continue;
            }




            //Use the third address for the other INGRESS interface (used with line arbitration)
            if (bIngressMAC1Set == false)
            {
                if (bEthernetObjectInitialised)
                {					
                    ethernet.SetMACFilterAddress(2, pMACAddresses->addr[i][0],
                                                    pMACAddresses->addr[i][1],
                                                    pMACAddresses->addr[i][2],
                                                    pMACAddresses->addr[i][3],
                                                    pMACAddresses->addr[i][4],
                                                    pMACAddresses->addr[i][5]);		
                }



                if (bIngressObject0Initialised)
                {
                    ingressTCPUDPIP1.SetMACAddress(pMACAddresses->addr[i][0],
                                                   pMACAddresses->addr[i][1],
                                                   pMACAddresses->addr[i][2],
                                                   pMACAddresses->addr[i][3],
                                                   pMACAddresses->addr[i][4],
                                                   pMACAddresses->addr[i][5]);

                }

                bIngressMAC1Set = true;
                continue;

            }




            //just set the MAC address on the 4th ethernet channel for consistency - it is not actually connected to anything in HW
            if (bSpareMACSet == false)
            {
                if (bEthernetObjectInitialised)
                {					
                    ethernet.SetMACFilterAddress(3, pMACAddresses->addr[i][0],
                                                    pMACAddresses->addr[i][1],
                                                    pMACAddresses->addr[i][2],
                                                    pMACAddresses->addr[i][3],
                                                    pMACAddresses->addr[i][4],
                                                    pMACAddresses->addr[i][5]);		
                }

                bSpareMACSet = true;
                continue;
            }
            
            
        }
    }
    



    return retval;
}





void AAT::CheckForSufficientMACAddresses(MACAddresses* pMACAddresses)
{
    uint32_t retval = XLNX_OK;
    uint32_t numValidAddresses = 0;
    bool bEthernetInitialised = false;
    bool bUDPIP1Initialised = false;
    uint32_t numSupportedEthernetChannels = 0;

    for (uint32_t i = 0; i < MACAddresses::NUM_SUPPORTED_MAC_ADDRESSES; i++)
    {
        if (pMACAddresses->isValid[i])
        {
            numValidAddresses++;
        }
    }


    ethernet.IsInitialised(&bEthernetInitialised);
    if (bEthernetInitialised)
    {
        retval = ethernet.GetNumSupportedChannels(&numSupportedEthernetChannels);
        if (retval == XLNX_OK)
        {
            for (uint32_t i = 0; i < numSupportedEthernetChannels; i++)
            {
                if (i < numValidAddresses)
                {
                    ethernet.SetChannelConfigurationAllowed(i, true);
                }
                else
                {
                    printf("[WARNING] Disabling configuration of Ethernet channel %u due to insufficient MAC addresses\n", i);
                    //Clear any default values...
                    ethernet.SetMACFilterAddress(i, 0, 0, 0, 0, 0, 0);

                    //..disable ANY output from GTs...
                    ethernet.SetTxInhibit(i, true);

                    //..and finally disable any config writes.
                    ethernet.SetChannelConfigurationAllowed(i, false);
                }
            }
        }
    }


    


    ingressTCPUDPIP1.IsInitialised(&bUDPIP1Initialised);
    if (bUDPIP1Initialised)
    {
        if (numValidAddresses >= 3)
        {
            ingressTCPUDPIP1.SetConfigurationAllowed(true);
        }
        else
        {
            printf("[WARNING] Disabling configuration of second ingress UDPIP block due to insufficient MAC addresses\n");
            
            //Clear the default address values...
            ingressTCPUDPIP1.SetIPv4Address(0, 0, 0, 0);
            ingressTCPUDPIP1.SetSubnetMask(0, 0, 0, 0);
            ingressTCPUDPIP1.SetGatewayAddress(0, 0, 0, 0);
            ingressTCPUDPIP1.SetMACAddress(0, 0, 0, 0, 0, 0);

            //..and finally disable any config writes...
            ingressTCPUDPIP1.SetConfigurationAllowed(false);
        }
    }


}






uint32_t AAT::StartDataMover(void)
{
    uint32_t retval = XLNX_OK;

    retval = dataMover.StartHWKernel();
    
    //we only want to turn on the data mover output from the order book if the data mover HW kernel started OK...
    if (retval == XLNX_OK)
    {
        retval = orderBook.SetDataMoverOutput(true);
    }


    return retval;
}









uint32_t AAT::StartCapture(char* filepath)
{
    uint32_t retval = XLNX_OK;


    //NOTE - the order that the kernels are started is important here!
    //To prevent HW FIFOs from filling up, we need to start the capture kernel before we turn on the network tap.


    retval = networkCapture.Start(filepath);
    if (retval != XLNX_OK)
    {
        SetSubBlockError(retval);
        retval = XLNX_AAT_ERROR_NETWORK_CAPTURE_ERROR;
    }





    if (retval == XLNX_OK)
    {
        retval = networkTap.Start();
        if (retval != XLNX_OK)
        {
            SetSubBlockError(retval);
            retval = XLNX_AAT_ERROR_NETWORK_TAP_ERROR;
        }
    }




    return retval;
}








uint32_t AAT::StopCapture(void)
{
    uint32_t retval = XLNX_OK;

    //NOTE - the order that the kernels are stopped  is important here!
    //To prevent HW FIFOs from filling up, we need to turn off the network tap before stopping the capture kernel...
    //i.e. this is the REVERSE ORDER to how we started things up.

    retval = networkTap.Stop();
    if (retval != XLNX_OK)
    {
        SetSubBlockError(retval);
        retval = XLNX_AAT_ERROR_NETWORK_TAP_ERROR;
    }



    if (retval == XLNX_OK)
    {
        retval = networkCapture.Stop();
        if (retval != XLNX_OK)
        {
            SetSubBlockError(retval);
            retval = XLNX_AAT_ERROR_NETWORK_CAPTURE_ERROR;
        }
    }


    return retval;
}




uint32_t AAT::IsCaptureRunning(bool* pbIsRunning)
{
    uint32_t retval = XLNX_OK;

    retval = networkCapture.IsRunning(pbIsRunning);

    if (retval != XLNX_OK)
    {
        SetSubBlockError(retval);
        retval = XLNX_AAT_ERROR_NETWORK_CAPTURE_ERROR;
    }

    return retval;
}




void AAT::SetSubBlockError(uint32_t errorCode)
{
    m_subBlockErrorCode = errorCode;
}






void AAT::GetSubBlockError(uint32_t* pErrorCode)
{
    *pErrorCode = m_subBlockErrorCode;
}

