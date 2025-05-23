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

#ifndef XLNX_DEVICE_INTERFACE_H
#define XLNX_DEVICE_INTERFACE_H

#include <iostream>

#include "xrt.h"
#include "experimental/xrt-next.h"
#include "experimental/xrt_xclbin.h"

#include "xlnx_device_interface_error_codes.h"






namespace XLNX
{





class BufferDescriptor
{
	
};








class MACAddresses
{
public:

	static const uint32_t NUM_SUPPORTED_MAC_ADDRESSES = 4;
	static const uint32_t NUM_MAC_ADDRESS_BYTES = 6;
public:
	uint8_t addr[NUM_SUPPORTED_MAC_ADDRESSES][NUM_MAC_ADDRESS_BYTES];
	bool isValid[NUM_SUPPORTED_MAC_ADDRESSES];
};












class DeviceInterface
{

public:
	virtual char* GetDeviceName(void) = 0;

	virtual MACAddresses* GetMACAddresses(void) = 0;

	virtual xuid_t* GetBitstreamUUID(void) = 0;

public:
	virtual void Close(void) = 0;





public: //Register Access (via absolute address)
	virtual uint32_t ReadReg32(uint64_t address, uint32_t* value) = 0;
	virtual uint32_t WriteReg32(uint64_t address, uint32_t value) = 0;
	virtual uint32_t WriteRegWithMask32(uint64_t address, uint32_t value, uint32_t mask) = 0;
	virtual uint32_t BlockReadReg32(uint64_t address, uint32_t* buffer, uint32_t numWords) = 0;
	virtual uint32_t BlockWriteReg32(uint64_t address, uint32_t* buffer, uint32_t numWords) = 0;








public: //Register Access (via CU index) - only introduced in 2019.2

	// The cuName parmeter should be of the form "CLASSNAME:INSTANCENAME"
	// i.e. it is the name listed in the IP_LAYOUT table of the XCLBIN
	// NOTE - instance name may be the same as class name
	// e.g. 
	// data_fifo_krnl:df1
	// data_fifo_krnl:df2
	// pingTop:pingTop
	// pongTop:pongTop
	virtual uint32_t GetCUAddress(const char* cuName, uint64_t* address) = 0;
	virtual uint32_t GetCUIndex(const char* cuName, uint32_t* cuIndex) = 0;
	virtual uint32_t ReadCUReg32(uint32_t cuIndex, uint64_t offset, uint32_t* value) = 0;
	virtual uint32_t WriteCUReg32(uint32_t cuIndex, uint64_t offset, uint32_t value) = 0;
	virtual uint32_t WriteCURegWithMask32(uint32_t cuIndex, uint64_t offset, uint32_t value, uint32_t mask) = 0;
	virtual uint32_t BlockReadCUReg32(uint32_t cuIndex, uint64_t offset, uint32_t* buffer, uint32_t numWords) = 0;
	virtual uint32_t BlockWriteCUReg32(uint32_t cuIndex, uint64_t offset, uint32_t* buffer, uint32_t numWords) = 0;

	static const uint64_t MAX_SUPPORTED_CU_SIZE = 64 * 1024; //this is taken from internal XRT values....
	virtual uint32_t ConvertAddressToCUIndexOffset(uint64_t address, uint32_t* cuIndex, uint64_t* offset) = 0;




public: //Memory Topology Functionality

	//A HW kernel that takes a pointer argument will have that argument wired to a specific memory bank within the FPGA.
	//This means than the pointer that is supplied must point to a location within that memory bank.
	//The following function can be used to query the memory bank that a particular argument is wired to.
	virtual uint32_t GetArgumentMemTopologyIndex(const char* cuName, uint32_t cuArgIndex, uint32_t* pTopologyIndex) = 0;


	//The Host Bank is a new memory bank that is used with the Slave Bridge IP block.  When a kernel argument is wired
	//to the host bank, it can write directly in host RAM, without needing to go via DDR.
	virtual uint32_t TopologyIndexIsHostBank(uint32_t topologyIndex, bool* pbIsHostBank) = 0;




public: //Host+Device Buffer Allocation

	

	//The following functions allocate a pair of buffers - one on the host and one on the card.
	//To access the host buffer from a userspace application, it must first be mapped using a call to MapBufferToUserspace
	//Data can be moved between the pair of buffers using the SyncBuffer

	//NOTE - in the following function, use GetArgumentMemTopologyIndex to get the topologyIndex/bankInfo
	virtual BufferDescriptor* AllocateBufferPair(uint32_t sizeInBytes, uint32_t bankInfo) = 0;
	virtual void FreeBufferPair(BufferDescriptor* pDescriptor) = 0;

	//The following function retrieves the address buffer on the card.
	virtual uint64_t GetDeviceBufferAddress(BufferDescriptor* pDescriptor) = 0;




	enum class BufferMapType 
	{
		READ_ONLY,
		READ_WRITE
	};


	//The following functions map the buffer into userspace so it may be accessed from a userspace application.
	virtual uint32_t MapBufferToUserspace(BufferDescriptor* pDescriptor, BufferMapType mapType, void** ppMapped) = 0;
	virtual uint32_t UnmapBufferFromUserspace(BufferDescriptor* pDescriptor, void* pMapped) = 0;




	enum class SyncDirection
	{
		TO_DEVICE,
		FROM_DEVICE
	};

	//The following functions are used to initiate a DMA transfer between a host buffer and a card buffer.
	virtual uint32_t SyncBuffer(BufferDescriptor* pDescriptor, SyncDirection direction) = 0;
	virtual uint32_t SyncBuffer(BufferDescriptor* pDescriptor, SyncDirection direction, uint32_t size, uint32_t offset) = 0;









public: //Raw Device Memory (DDR, HBM, etc.) Access
	virtual uint32_t ReadMem32(uint64_t address, uint32_t* value) = 0;
	virtual uint32_t WriteMem32(uint64_t address, uint32_t value) = 0;
	virtual uint32_t WriteMemWithMask32(uint64_t address, uint32_t value, uint32_t mask) = 0;
	virtual uint32_t BlockReadMem32(uint64_t address, uint32_t* buffer, uint32_t numWords) = 0;
	virtual uint32_t BlockWriteMem32(uint64_t address, uint32_t* buffer, uint32_t numWords) = 0;








public: //Host-Only Buffer Allocation

	//The following functions are used to allocate HOST-ONLY buffers.  These are intended for use with the
	//new SLAVE BRIDGE block - which has the ability to allow a HW kernel to write directly into host memory without
	//having to go via card memory

	//NOTE - in the following function, use GetArgumentMemTopologyIndex to get the topologyIndex/bankInfo
	virtual BufferDescriptor* AllocateBufferHostOnly(uint32_t sizeInBytes, uint32_t bankInfo) = 0;
	virtual void FreeBufferHostOnly(BufferDescriptor* pDescriptor) = 0;






public: //Stream Interface

	//NOTE - Reading/writing from/to streams uses an underlying HW DMA mechanism.  This has a restriction that
	//		 the address of the user buffers being supplied MUST BE ON A 4KB PAGE BOUNDARY.
	//



	typedef uint64_t StreamHandleType;
	typedef uint64_t StreamBufferHandleType;

	enum class StreamDirection
	{
		READ_STREAM,	//Card-to-Host
		WRITE_STREAM	//Host-to-Card
	};

	virtual uint32_t OpenStream(const char* cuName, uint32_t cuStreamArgIndex, StreamDirection direction, StreamHandleType* streamHandle) = 0;
	virtual uint32_t CloseStream(StreamHandleType streamHandle) = 0;

	virtual uint32_t WriteToStream(StreamHandleType streamHandle, void* dataBuffer, uint32_t numBytesToWrite, uint32_t* numBytesWritten) = 0;
	virtual uint32_t ReadFromStreamBlocking(StreamHandleType streamHandle, void* dataBuffer, uint32_t numBytesToRead, uint32_t* numBytesRead) = 0;
	virtual uint32_t ReadFromStreamNonBlocking(StreamHandleType streamHandle, void* dataBuffer, uint32_t numBytesToRead, uint32_t* numBytesRead) = 0;
    virtual uint32_t PollReadStream(StreamHandleType streamHandle, uint32_t* numBytesRead, int timeout) = 0;


	virtual uint32_t AllocateStreamBuffer(uint64_t bufferSize, StreamBufferHandleType* pBufferHandle, void** ppBuffer) = 0;
	virtual uint32_t DeallocateStreamBuffer(StreamBufferHandleType bufferHandle) = 0;
	





public: //Bitstream Functions
	virtual uint32_t DownloadBitstream(char* filePath) = 0;

	//Get Info on RUNNING bitstream
	virtual uint32_t GetBitstreamSectionInfo(void* info, size_t* size, enum axlf_section_kind kind, int index) = 0;





public:
	static const uint32_t MAX_SUPPORTED_CLOCKS = 4;
	virtual uint32_t GetClocks(uint32_t frequencyMHz[MAX_SUPPORTED_CLOCKS], uint32_t* pNumClocks) = 0;



public:
	static const uint32_t MAX_CU_NAME_LENGTH = 64;

};




}





#endif

