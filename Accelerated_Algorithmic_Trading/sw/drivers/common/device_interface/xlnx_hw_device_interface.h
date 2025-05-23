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

#ifndef XLNX_HW_DEVICE_INTERFACE_H
#define XLNX_HW_DEVICE_INTERFACE_H

#include "xlnx_device_interface.h"

#ifdef _WIN32
#include "windows/uuid.h"
#else
#include <uuid/uuid.h>
#endif


#ifdef _WIN32
extern "C"
{
	extern int uuid_parse(char* in, unsigned char* uu);
}
#endif




namespace XLNX
{




class HWDeviceInterface : public DeviceInterface
{

public:
	HWDeviceInterface(xclDeviceHandle deviceHandle);
	virtual ~HWDeviceInterface();



public:
	char* GetDeviceName(void);

	MACAddresses* GetMACAddresses(void);

	xuid_t* GetBitstreamUUID(void);


public:
	void Close(void);


public: //Register Access (via absolute address)
	uint32_t ReadReg32(uint64_t address, uint32_t* value);
	uint32_t WriteReg32(uint64_t address, uint32_t value);
	uint32_t WriteRegWithMask32(uint64_t address, uint32_t value, uint32_t mask);
	uint32_t BlockReadReg32(uint64_t address, uint32_t* buffer, uint32_t numWords);
	uint32_t BlockWriteReg32(uint64_t address, uint32_t* buffer, uint32_t numWords);


public: //Register Access (via CU index) - only available in 2019.2
	uint32_t GetCUAddress(const char* cuName, uint64_t* address);
	uint32_t GetCUIndex(const char* cuName, uint32_t* cuIndex);
	uint32_t ReadCUReg32(uint32_t cuIndex, uint64_t offset, uint32_t* value);
	uint32_t WriteCUReg32(uint32_t cuIndex, uint64_t offset, uint32_t value);
	uint32_t WriteCURegWithMask32(uint32_t cuIndex, uint64_t offset, uint32_t value, uint32_t mask);
	uint32_t BlockReadCUReg32(uint32_t cuIndex, uint64_t offset, uint32_t* buffer, uint32_t numWords);
	uint32_t BlockWriteCUReg32(uint32_t cuIndex, uint64_t offset, uint32_t* buffer, uint32_t numWords);

	uint32_t ConvertAddressToCUIndexOffset(uint64_t address, uint32_t* cuIndex, uint64_t* offset);




public: //Memory Topology Functionality
	uint32_t GetArgumentMemTopologyIndex(const char* cuName, uint32_t argIndex, uint32_t* pTopologyIndex);
	uint32_t TopologyIndexIsHostBank(uint32_t topologyIndex, bool* pbIsHostBank);



public: //Device+Host Buffer Allocation
	
	BufferDescriptor* AllocateBufferPair(uint32_t sizeInBytes, uint32_t bankInfo);
	void FreeBufferPair(BufferDescriptor* pDescriptor);


	uint64_t GetDeviceBufferAddress(BufferDescriptor* pDescriptor);


	uint32_t MapBufferToUserspace(BufferDescriptor* pDescriptor, BufferMapType mapType, void** ppMapped);
	uint32_t UnmapBufferFromUserspace(BufferDescriptor* pDescriptor, void* pMapped);


	uint32_t SyncBuffer(BufferDescriptor* pDescriptor, SyncDirection direction);
	uint32_t SyncBuffer(BufferDescriptor* pDescriptor, SyncDirection direction, uint32_t size, uint32_t offset);



public: //Device Memory (DDR, HBM, etc.) Access
	uint32_t ReadMem32(uint64_t address, uint32_t* value);
	uint32_t WriteMem32(uint64_t address, uint32_t value);
	uint32_t WriteMemWithMask32(uint64_t address, uint32_t value, uint32_t mask);
	uint32_t BlockReadMem32(uint64_t address, uint32_t* buffer, uint32_t numWords);
	uint32_t BlockWriteMem32(uint64_t address, uint32_t* buffer, uint32_t numWords);



public: //Host-Only Buffer Allocation

	//NOTE - in the following function, use GetArgumentMemTopologyIndex to get the topologyIndex/bankInfo
	virtual BufferDescriptor* AllocateBufferHostOnly(uint32_t sizeInBytes, uint32_t bankInfo);
	virtual void FreeBufferHostOnly(BufferDescriptor* pDescriptor);


	 
public: //Streaming Interface
	uint32_t OpenStream(const char* cuName, uint32_t cuStreamArgIndex, StreamDirection direction, StreamHandleType* streamHandle);
	uint32_t CloseStream(StreamHandleType streamHandle);

	uint32_t WriteToStream(StreamHandleType streamHandle, void* dataBuffer, uint32_t numBytesToWrite, uint32_t* numBytesWritten);
	uint32_t ReadFromStreamBlocking(StreamHandleType streamHandle, void* dataBuffer, uint32_t numBytesToRead, uint32_t* numBytesRead);
	uint32_t ReadFromStreamNonBlocking(StreamHandleType streamHandle, void* dataBuffer, uint32_t numBytesToRead, uint32_t* numBytesRead);
    uint32_t PollReadStream(StreamHandleType streamHandle, uint32_t* numBytesRead, int timeout);


public:
	uint32_t AllocateStreamBuffer(uint64_t bufferSize, StreamBufferHandleType* pBufferHandle, void** ppBuffer);
	uint32_t DeallocateStreamBuffer(StreamBufferHandleType bufferHandle);


public: //Bitstream Functions
	uint32_t DownloadBitstream(char* filePath);
	
	//Get Info on RUNNING bitstream
	uint32_t GetBitstreamSectionInfo(void* info, size_t* size, enum axlf_section_kind kind, int index); 




public: //Clock Functions
	uint32_t GetClocks(uint32_t frequencyMHz[MAX_SUPPORTED_CLOCKS], uint32_t* pNumClocks);







private: //Stream Creation Helpers
	uint32_t GetStreamIDs(const char* cuName, uint32_t cuArgIndex, uint64_t* routeID, uint64_t* flowID);

	uint32_t GetCUIPLayoutIndex(const char* cuName, uint32_t* ipLayoutIndex);
	uint32_t GetMemTopologyIndex(uint32_t ipLayoutIndex, uint32_t kernelArgIndex, uint32_t* memTopologyIndex);
	uint32_t GetStreamIDs(uint32_t memTopologyIndex, uint64_t* routeID, uint64_t* flowID);
	



private: //Emulation Helpers
	uint32_t EmulationGetSectionInfo(void* info, size_t * size, enum axlf_section_kind sectionKind, int index);
	uint32_t EmulationGetIPLayoutElement(struct ip_layout* pIPLayoutSection, int elementIndex, void* pElementBuffer, size_t* pElementSize);
	uint32_t EmulationGetMemTopologyElement(struct mem_topology* pMemTopologySection, int elementIndex, void* pElement, size_t* pElementSize);
	uint32_t EmulationGetConnectivityElement(struct connectivity* pConnectivity, int elementIndex, void* pElement, size_t* pElementSize);



private:
	void ReadMACAddresses(void);

	bool ReadMACAddressOldMethod(uint32_t index);
	bool ReadMACAddressNewMethod(uint32_t index);
	void IncrementMACAddress(uint8_t* pA, uint8_t* pB, uint8_t* pC, uint8_t* pD, uint8_t* pE, uint8_t* pF, uint32_t index);



private:
	xclDeviceHandle m_deviceHandle;
	xclDeviceInfo2 m_deviceInfo;

	MACAddresses m_macAddresses;

	xuid_t m_xuid;
	bool m_bSharedAccess;
	

	uint8_t* m_xclbinBuffer;
	uint64_t m_xclbinBufferLength;

};







#ifdef _WIN32
typedef void* xrtBufferHandle;
#else
typedef unsigned int xrtBufferHandle;
#endif




class HWDeviceBufferDescriptor : public BufferDescriptor
{

public:
	xrtBufferHandle m_bufferHandle;
	uint64_t m_deviceBufferAddress;
	uint32_t m_sizeInBytes;


	bool m_bIsHostOnlyBuffer;



};









}


#endif 
