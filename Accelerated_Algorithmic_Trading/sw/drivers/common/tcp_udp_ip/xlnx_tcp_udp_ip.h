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

#ifndef XLNX_TCP_UDP_IP_H
#define XLNX_TCP_UDP_IP_H


#include <cstdint>


#include "xlnx_device_interface.h"

#include "xlnx_tcp_udp_ip_error_codes.h"


namespace XLNX
{







class TCPUDPIP
{

public:
	TCPUDPIP();
	virtual ~TCPUDPIP();






public: //General 

	uint32_t Initialise(DeviceInterface* pDeviceInterface, const char* cuName);

	uint32_t SetMACAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f);
	uint32_t GetMACAddress(uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d, uint8_t* e, uint8_t* f);

	uint32_t SetIPv4Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
	uint32_t GetIPv4Address(uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d);

	uint32_t SetSubnetMask(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
	uint32_t GetSubnetMask(uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d);

	uint32_t SetGatewayAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
	uint32_t GetGatewayAddress(uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d);







public: //IGMP 

	uint32_t SetIGMPEnabled(bool bEnabled);
	uint32_t GetIGMPEnabled(bool* pbEnabled);

	static const uint32_t NUM_MULTICAST_ADDRESSES_SUPPORTED = 4;

	uint32_t AddIPv4MulticastAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
	uint32_t DeleteIPv4MulticastAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

	//Internally the HW block maintain a table of multicast addresses.  The following function
	//reads the specified index in the table.  NOTE - the table is a SPARSE TABLE, meaning
	//there can be empty entries between valid entries.
	//Empty entries give back a multicast address of 0.0.0.0
	uint32_t GetNumValidIGMPEntries(uint32_t* pNumValidEntries);
	uint32_t ReadIGMPEntry(uint32_t index, uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d);

	static const uint32_t IGMP_MIN_SUPPORTED_VERSION = 2;
	static const uint32_t IGMP_MAX_SUPPORTED_VERSION = 3;

	static const uint32_t IGMP_DEFAULT_VERSION = IGMP_MAX_SUPPORTED_VERSION;

	uint32_t GetIGMPVersion(uint32_t *version);
	uint32_t SetIGMPVersion(uint32_t version);



public: //ARP
	static const uint32_t NUM_ARP_ENTRIES_SUPPORTED = 256;

	typedef struct
	{
		struct
		{
			uint8_t a;
			uint8_t b;
			uint8_t c;
			uint8_t d;

		}IPAddress;

		struct 
		{
			uint8_t a;
			uint8_t b;
			uint8_t c;
			uint8_t d;
			uint8_t e;
			uint8_t f;

		}MACAddress;

	}ARPEntry;

	uint32_t ReadARPEntry(uint32_t index, ARPEntry* pARPEntry, bool* pbEntryValid);

	uint32_t AddARPEntry(ARPEntry* pARPEntry);
	uint32_t DeleteARPEntry(uint32_t index);
	uint32_t DeleteAllARPEntries(void);








public: //ICMP
	uint32_t SetICMPEnabled(bool bEnabled);
	uint32_t GetICMPEnabled(bool* pbEnabled);
	







public: //UDP Handler
	uint32_t AddUDPListeningPort(uint16_t port);
	uint32_t DeleteUDPListeningPort(uint16_t port);
	uint32_t DeleteAllUDPListeningPorts(void);

	uint32_t IsUDPListeningPortEnabled(uint16_t port, bool* pbEnabled);

	static const uint32_t NUM_LISTENING_PORTS_SUPPORTED = 65536;
	uint32_t GetUDPListeningPorts(uint16_t enabledPorts[NUM_LISTENING_PORTS_SUPPORTED], uint32_t* pNumEnabledPorts);



public: //Stats

	typedef struct
	{
		uint32_t numInvalidChecksum;
		uint32_t numIGMPQueries;
		uint32_t numInvalidQueries;
	}IGMPStats;


	typedef struct
	{
		uint32_t numRequestsSent;
		uint32_t numRepliesSent;
		uint32_t numRequestsReceived;
		uint32_t numRepliesReceived;
		uint32_t numRequestsSentLost;
	}ARPStats;

	typedef struct
	{
		uint32_t numEchoRequestsReceived;
		uint32_t numEchoRepliesSent;
	}ICMPStats;

	typedef struct
	{
		uint32_t numInHeaderErrors;
		uint32_t numInDelivers;
		uint32_t numInUnknownProtocols;
		uint32_t numInAddressErrors;
		uint32_t numInReceives;
	}IPHandlerStats;

	typedef struct
	{
		uint32_t numIPv4PacketsSent;
		uint32_t numPacketsDropped;
	}MACIPEncoderStats;


	typedef struct 
	{
		bool bStatsValid;
		uint32_t numTxDatagrams;
		uint32_t numRxDatagrams;
		uint32_t numRxDatagramsInvalidPort;
	}UDPStats;

	typedef struct
	{
		bool bStatsValid;
		uint32_t numInSegments;
		uint32_t numInErrors;
		uint32_t numOutSegments;
		uint32_t numRetransmittedSegments;
		uint32_t numActiveOpens;
		uint32_t numPassiveOpens;
		uint32_t numAttemptFails;
		uint32_t numEstablishResets;
		uint32_t numCurrentEstablished;
	}TCPStats;



	typedef struct
	{
		IGMPStats igmp;
		ARPStats arp;
		ICMPStats icmp;
		IPHandlerStats ipHandler;
		MACIPEncoderStats macIPEncoder;
		UDPStats udp;
		TCPStats tcp;

	}Stats;
	


	uint32_t GetStats(Stats* pStats);






public:
	void IsInitialised(bool* pbIsInitialised);


	//An instance of the HW block can be built in one of a few ways:
	//(1) UDP Only
	//(2) TCP Only
	//(3) UDP and TCP 
	//The following functions can be used to determine what has been built
	void IsTCPSynthesized(bool* bIsSynthesized);
	void IsUDPSynthesized(bool* bIsSynthesized);

	uint32_t GetCUIndex(uint32_t* pCUIndex);
	uint32_t GetCUAddress(uint64_t* pCUAddress);


public:
	//The following function is used to enable/disable ANY CONFIGURATION of the block.
	//This was introduced to allow an instance of the block to be disabled due to HW limitations on the underlying card
	//e.g. insufficient MAC addresses 
	void SetConfigurationAllowed(bool bAllowed);
	void GetConfigurationAllowed(bool* pbAllowed);





protected:
	uint32_t CheckIsInitialised(void);
	uint32_t CheckTCPIsSynthesized(void);
	uint32_t CheckUDPIsSynthesized(void);
	uint32_t CheckConfigurationIsAllowed(void);


protected: //IO
	uint32_t ReadReg32(uint64_t offset, uint32_t* value);
	uint32_t WriteReg32(uint64_t offset, uint32_t value);
	uint32_t WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask);
	uint32_t BlockReadReg32(uint64_t offset, uint32_t* buffer, uint32_t numWords);
	

	
	
protected:
	uint32_t m_initialisedMagicNumber;

	uint64_t m_cuAddress;
	uint32_t m_cuIndex;

	DeviceInterface* m_pDeviceInterface;

	bool m_bTCPSynthesized;
	bool m_bUDPSynthesized;


protected: 
	bool m_bConfigurationAllowed;




protected: //UDP

	void CalculateUDPPortCAMOffsets(uint16_t port, uint32_t* pWordIndex, uint32_t* pBitIndex);

	
protected: //IGMP
	void InitialiseDefaultIGMPVersionIfNecessary(void);


protected: //Stats
	uint32_t GetIGMPStats(IGMPStats* pStats);
	uint32_t GetARPStats(ARPStats* pStats);
	uint32_t GetICMPStats(ICMPStats* pStats);
	uint32_t GetIPHandlerStats(IPHandlerStats* pStats);
	uint32_t GetMACIPEncoderStats(MACIPEncoderStats* pStats);
	uint32_t GetUDPStats(UDPStats* pStats);
	uint32_t GetTCPStats(TCPStats* pStats);





protected: //Utils
	static void PackIPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint32_t* pRegValue);
	static void UnpackIPAddress(uint32_t regValue, uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d);
	static void PackMACAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f, uint32_t* pUpperRegValue, uint32_t* pLowerRegValue);
	static void UnpackMACAddress(uint32_t upperRegValue, uint32_t lowerRegValue, uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d, uint8_t* e, uint8_t* f);





}; //class TCPUDPIP


} //namespace XLNX




#endif //XLNX_TCP_UDP_IP_H

