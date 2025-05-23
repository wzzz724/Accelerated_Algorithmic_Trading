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

#include <cstring> //for memset

#include "xlnx_network_capture.h"
#include "xlnx_network_capture_address_map.h"
#include "xlnx_network_capture_pcap_writer.h"
#include "xlnx_network_capture_internal.h"


using namespace XLNX;







NetworkCapture::NetworkCapture()
{
    m_pDeviceInterface              = nullptr;
    m_cuAddress                     = 0;
    m_cuIndex                       = 0;
    m_initialisedMagicNumber        = 0;


    m_bUsingHostBank                = false;
    m_bNeedToSetupBuffer            = true;
    m_bufferMemTopologyIndex        = 0;
    m_hostBankMemTopologyIndex      = 0;
    m_pBufferDescriptor             = nullptr;
    m_bufferHostVirtualAddress      = nullptr;
    m_bufferHWAddress               = INVALID_HW_BUFFER_ADDRESS;

    m_clockFrequencyMHz             = 0;

    m_bKeepRunning                  = false;
    m_bYield                        = false;
    m_numCapturedPackets            = 0;
    m_pollRateMilliseconds          = 0; 

    m_hwEmulationPollDelaySeconds   = HW_EMU_POLL_DELAY_DEFAULT_SECONDS;
}



NetworkCapture::~NetworkCapture()
{
    Stop();
}





uint32_t NetworkCapture::Initialise(DeviceInterface* pDeviceInterface, const char* cuName)
{
    uint32_t retval = XLNX_OK;
    bool bHostBank = false;
    uint32_t clockFrequencyArray[DeviceInterface::MAX_SUPPORTED_CLOCKS];
    uint32_t numClocks;


    m_initialisedMagicNumber = 0;	//will be set to magic number if we successfully initialise...

    m_pDeviceInterface = pDeviceInterface;


    retval = m_pDeviceInterface->GetCUAddress(cuName, &m_cuAddress);

    if (retval != XLNX_OK)
    {
        retval = XLNX_NETWORK_CAPTURE_ERROR_CU_NAME_NOT_FOUND;
    }



    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->GetCUIndex(cuName, &m_cuIndex);

        if (retval != XLNX_OK)
        {
            retval = XLNX_NETWORK_CAPTURE_ERROR_CU_NAME_NOT_FOUND;
        }
    }



    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->GetArgumentMemTopologyIndex(cuName, KERNEL_MEMORY_ARGUMENT_BUFFER_INDEX, &m_bufferMemTopologyIndex);
        if (retval != XLNX_OK)
        {
            retval = XLNX_NETWORK_CAPTURE_ERROR_CU_NAME_NOT_FOUND;
        }
    }




    if (retval == XLNX_OK)
    {
        //We need to figure out if the kernel is connected to CARD-RAM or HOST-BANK (aka SLAVE-BRIDGE)
        //as the way we need to deal with buffers differs...
        retval = m_pDeviceInterface->TopologyIndexIsHostBank(m_bufferMemTopologyIndex, &bHostBank);
        if (retval == XLNX_OK)
        {
            if (bHostBank)
            { 
                m_hostBankMemTopologyIndex = m_bufferMemTopologyIndex;
                m_bUsingHostBank = true;
            }
        }
    }



    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->GetClocks(clockFrequencyArray, &numClocks);
        if (retval == XLNX_OK)
        {
            m_clockFrequencyMHz = clockFrequencyArray[0]; //NOTE - assuming our clock is the first one in the list
        }
    }



    if (retval == XLNX_OK)
    {
        m_initialisedMagicNumber = XLNX_NETWORK_CAPTURE_INITIALISED_MAGIC_NUMBER;
    }

    return retval;
}











uint32_t NetworkCapture::Uninitialise(void)
{
    uint32_t retval = XLNX_OK;

    retval = Stop();

    if (retval == XLNX_OK)
    {
        retval = CleanupBufferObjects();
    }

    return retval;
}














void NetworkCapture::IsInitialised(bool* pbIsInitialised)
{
    if (CheckIsInitialised() == XLNX_OK)
    {
        *pbIsInitialised = true;
    }
    else
    {
        *pbIsInitialised = false;
    }
}







uint32_t NetworkCapture::GetCUIndex(uint32_t* pCUIndex)
{
    uint32_t retval = XLNX_OK;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        *pCUIndex = m_cuIndex;
    }

    return retval;
}




uint32_t NetworkCapture::GetCUAddress(uint64_t* pCUAddress)
{
    uint32_t retval = XLNX_OK;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        *pCUAddress = m_cuAddress;
    }

    return retval;
}





uint32_t NetworkCapture::GetBufferCUMemTopologyIndex(uint32_t* pMemTopologyIndex)
{
    uint32_t retval = XLNX_OK;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        *pMemTopologyIndex = m_bufferMemTopologyIndex;
    }

    return retval;
}










uint32_t NetworkCapture::StartHWKernel(void)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;
    uint32_t mask;

    const uint32_t AP_START = (1 << 0);
    const uint32_t AP_CONTINUE = (1 << 7);

    retval = CheckIsInitialised();



    if (retval == XLNX_OK)
    {
        retval = SetupBufferIfNecessary();
    }


    if (retval == XLNX_OK)
    {
        //NOTE - This Kernel is NOT a free running kernel.  This means that it must be manually started running from the host.
        //		 To do this, we set the AP_START bit in the kernel control register...
        //       
        //       A typical compute kernel will run once (i.e perform one iteration), then stop and wait until it is started
        //       again from the host.  However once we start this kernel, we want it to continue running without further intervention.
        //		 To achieve this, we also set the AP_CONTINUE bit. 

        value = AP_START | AP_CONTINUE;
        mask = value;

        retval = WriteRegWithMask32(XLNX_NETWORK_CAPTURE_KERNEL_CONTROL_OFFSET, value, mask);
    }



    if (retval == XLNX_OK)
    {
        value = 1 << 16;
        mask = value;
        retval = WriteRegWithMask32(XLNX_NETWORK_CAPTURE_ENABLE_PROCESSING_OFFSET, value, mask);
    }

    return retval;
}












uint32_t NetworkCapture::IsHWKernelRunning(bool* pbIsRunning)
{
    uint32_t retval = XLNX_OK;
    uint32_t value;

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = ReadReg32(XLNX_NETWORK_CAPTURE_KERNEL_CONTROL_OFFSET, &value);
    }

    if (retval == XLNX_OK)
    {
        if (value & 0x01)
        {
            *pbIsRunning = true;
        }
        else
        {
            *pbIsRunning = false;
        }
    }

    return retval;
}










uint32_t NetworkCapture::ReadReg32(uint64_t offset, uint32_t* value)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->ReadReg32(address, value);




    if (retval != XLNX_OK)
    {
        retval = XLNX_NETWORK_CAPTURE_ERROR_IO_FAILED;
    }

    return retval;
}






uint32_t NetworkCapture::WriteReg32(uint64_t offset, uint32_t value)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->WriteReg32(address, value);


    if (retval != XLNX_OK)
    {
        retval = XLNX_NETWORK_CAPTURE_ERROR_IO_FAILED;
    }

    return retval;
}




uint32_t NetworkCapture::WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->WriteRegWithMask32(address, value, mask);

    if (retval != XLNX_OK)
    {
        retval = XLNX_NETWORK_CAPTURE_ERROR_IO_FAILED;
    }

    return retval;
}






uint32_t NetworkCapture::BlockReadReg32(uint64_t offset, uint32_t* buffer, uint32_t numWords)
{
    uint32_t retval = XLNX_OK;
    uint64_t address;


    address = m_cuAddress + offset;

    retval = m_pDeviceInterface->BlockReadReg32(address, buffer, numWords);


    if (retval != XLNX_OK)
    {
        retval = XLNX_NETWORK_CAPTURE_ERROR_IO_FAILED;
    }

    return retval;
}










uint32_t NetworkCapture::SetupBufferObjects(void)
{
	uint32_t retval = XLNX_OK;

    if (m_bUsingHostBank)
    {
        retval = SetupBufferObjectsFromHostBank();
    }
    else
    {
        retval = SetupBufferObjectsFromCardRAM();
    }
	
	

	return retval;
}





uint32_t NetworkCapture::CleanupBufferObjects(void)
{
	uint32_t retval = XLNX_OK;

    if (m_bUsingHostBank)
    {
        retval = CleanupBufferObjectsFromHostBank();
    }
    else
    {
        retval = CleanupBufferObjectsFromCardRAM();
    }
	

	return retval;
}




uint32_t NetworkCapture::SetupBufferObjectsFromHostBank(void)
{
    uint32_t retval = XLNX_OK;

    //The HW kernel is connected to HOST-BANK.  This means we only have to allocate a buffer on the host...

    m_pBufferDescriptor = m_pDeviceInterface->AllocateBufferHostOnly(BUFFER_SIZE, m_bufferMemTopologyIndex);

    if (m_pBufferDescriptor == nullptr)
    {
        retval = XLNX_NETWORK_CAPTURE_ERROR_FAILED_TO_ALLOCATE_BUFFER_OBJECT;
    }



    if (retval == XLNX_OK)
    {
        //Plug the buffer details into the HW kernel
        retval = SetupBufferDetails();
    }



    return retval;
}












uint32_t NetworkCapture::SetupBufferObjectsFromCardRAM(void)
{
	uint32_t retval = XLNX_OK;

	// The HW kernel is connected to CARD RAM.  This means we need to allocate a BUFFER PAIR (i.e. HOST + CARD)

	m_pBufferDescriptor = m_pDeviceInterface->AllocateBufferPair(BUFFER_SIZE, m_bufferMemTopologyIndex);
	if (m_pBufferDescriptor == nullptr)
	{
		retval = XLNX_NETWORK_CAPTURE_ERROR_FAILED_TO_ALLOCATE_BUFFER_OBJECT;
	}



    if (retval == XLNX_OK)
    {
        //Plug the buffer details into the HW kernel
        retval = SetupBufferDetails();
    }



	return retval;
}







uint32_t NetworkCapture::SetupBufferDetails(void)
{
    uint32_t retval = XLNX_OK;


    //Retrieve the HW address of the buffers in CARD RAM
    if (retval == XLNX_OK)
    {
        m_bufferHWAddress = m_pDeviceInterface->GetDeviceBufferAddress(m_pBufferDescriptor);
    }



    //Retrieve the virtual address of the HOST BUFFERS
    if (retval == XLNX_OK)
    {
        retval = m_pDeviceInterface->MapBufferToUserspace(m_pBufferDescriptor, DeviceInterface::BufferMapType::READ_ONLY, &m_bufferHostVirtualAddress);
        if (retval != XLNX_OK)
        {
            retval = XLNX_NETWORK_CAPTURE_ERROR_FAILED_TO_MAP_BUFFER_OBJECT;
        }
    }




    //Tell the kernel about the buffer addresses...
    if (retval == XLNX_OK)
    {
        retval = SetHWBufferAddress(m_bufferHWAddress);
    }


    return retval;
}






uint32_t NetworkCapture::SetHWBufferAddress(uint64_t address)
{
	uint32_t retval = XLNX_OK;
	uint32_t upperWord;
	uint32_t lowerWord;

	upperWord = (uint32_t)((address >> 32) & 0xFFFFFFFF);
	lowerWord = (uint32_t)(address & 0xFFFFFFFF);


	retval = WriteReg32(XLNX_NETWORK_CAPTURE_BUFFER_ADDRESS_LOWER_WORD_OFFSET, lowerWord);


	if (retval == XLNX_OK)
	{
		retval = WriteReg32(XLNX_NETWORK_CAPTURE_BUFFER_ADDRESS_UPPER_WORD_OFFSET, upperWord);
	}

	return retval;
}







uint32_t NetworkCapture::CleanupBufferObjectsFromHostBank(void)
{
    uint32_t retval = XLNX_OK;

    if (m_bufferHostVirtualAddress != nullptr)
    {
        m_pDeviceInterface->UnmapBufferFromUserspace(m_pBufferDescriptor, m_bufferHostVirtualAddress);
        m_bufferHostVirtualAddress = nullptr;
    }


    if (m_pBufferDescriptor != nullptr)
    {
        m_pDeviceInterface->FreeBufferHostOnly(m_pBufferDescriptor);
        m_pBufferDescriptor = nullptr;
    }



    m_bufferHWAddress = INVALID_HW_BUFFER_ADDRESS;


    return retval;
}






uint32_t NetworkCapture::CleanupBufferObjectsFromCardRAM(void)
{
	uint32_t retval = XLNX_OK;

	if (m_bufferHostVirtualAddress != nullptr)
	{
		m_pDeviceInterface->UnmapBufferFromUserspace(m_pBufferDescriptor, m_bufferHostVirtualAddress);
		m_bufferHostVirtualAddress = nullptr;
	}


	if (m_pBufferDescriptor != nullptr)
	{
		m_pDeviceInterface->FreeBufferPair(m_pBufferDescriptor);
		m_pBufferDescriptor = nullptr;
	}

	

	m_bufferHWAddress = INVALID_HW_BUFFER_ADDRESS;


	return retval;
}











uint32_t NetworkCapture::SetupBufferIfNecessary(void)
{
	uint32_t retval = XLNX_OK;

	if (m_bNeedToSetupBuffer)
	{
		retval = SetupBufferObjects();
		m_bNeedToSetupBuffer = false;
	}

	return retval;
}



void* NetworkCapture::GetBufferHostVirtualAddress(void)
{
	return m_bufferHostVirtualAddress;
}


uint64_t NetworkCapture::GetBufferHWAddress(void)
{
	return m_bufferHWAddress;
}



uint32_t NetworkCapture::GetTailPointer(uint32_t* pTailPointer)
{
    uint32_t retval = XLNX_OK;

    retval = ReadReg32(XLNX_NETWORK_CAPTURE_BUFFER_TAIL_POINTER_OFFSET, pTailPointer);

    return retval;
}







uint32_t NetworkCapture::ConvertClockCyclesToNanoseconds(uint64_t clockCycles, uint64_t* pNanoseconds)
{
    uint32_t retval = XLNX_OK;
    double cyclesPerNanosecond;

    cyclesPerNanosecond = (double)m_clockFrequencyMHz / 1000.0;

    //Here we are rounding up or down to the nearest whole number of nanoseconds...
    *pNanoseconds = (uint64_t)(((double)clockCycles / (double)cyclesPerNanosecond) + 0.5);

    return retval;
}





uint32_t NetworkCapture::Start(char* filepath)
{
    uint32_t retval = XLNX_OK;
   

    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckIsNotRunning();
    }

    if (retval == XLNX_OK)
    {
        retval = StartHWKernel();
    }

    if (retval == XLNX_OK)
    {
        retval = m_pcapWriter.Start(filepath);
    }


    if (retval == XLNX_OK)
    {
        retval = StartThread();
    }


    return retval;
}




uint32_t NetworkCapture::Stop(void)
{
    uint32_t retval = XLNX_OK;


    retval = CheckIsInitialised();

    if (retval == XLNX_OK)
    {
        retval = CheckIsRunning();
    }

    if (retval == XLNX_OK)
    {
        retval = StopThread();
    }

    if (retval == XLNX_OK)
    {
        retval = m_pcapWriter.Stop();
    }


    return retval;
}





uint32_t NetworkCapture::IsRunning(bool* pbIsRunning)
{
    uint32_t retval = XLNX_OK;

    retval = IsThreadRunning(pbIsRunning);

    return retval;
}



uint32_t NetworkCapture::GetNumCapturedPackets(uint32_t* pNumPackets)
{
    uint32_t retval = XLNX_OK;

    retval = CheckIsInitialised();

    *pNumPackets = m_numCapturedPackets;

    return retval;
}



void NetworkCapture::SetHWEmulationPollDelay(uint32_t delaySeconds)
{
    m_hwEmulationPollDelaySeconds = delaySeconds;
}


void NetworkCapture::GetHWEmulationPollDelay(uint32_t* pDelaySeconds)
{
    *pDelaySeconds = m_hwEmulationPollDelaySeconds;
}