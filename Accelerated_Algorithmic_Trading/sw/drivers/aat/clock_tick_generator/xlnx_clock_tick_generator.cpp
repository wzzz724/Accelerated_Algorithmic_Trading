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

#include "xlnx_clock_tick_generator.h"
#include "xlnx_clock_tick_generator_address_map.h"
#include "xlnx_clock_tick_generator_error_codes.h"
using namespace XLNX;



static const uint32_t IS_INITIALISED_MAGIC_NUMBER = 0x5283CA3B;



ClockTickGenerator::ClockTickGenerator()
{
    m_pDeviceInterface = nullptr;

    m_cuIndex = 0xFFFFFFFF;
    m_cuAddress = 0;

    m_initialisedMagicNumber = 0;

	m_clockFrequencyMHz = 0;
}



ClockTickGenerator::~ClockTickGenerator()
{
    
}





uint32_t ClockTickGenerator::Initialise(DeviceInterface* pDeviceInterface, const char* cuName)
{
	uint32_t retval = XLNX_OK;

	uint32_t clockFrequencyArray[DeviceInterface::MAX_SUPPORTED_CLOCKS];
	uint32_t numClocks;

	m_initialisedMagicNumber = 0;	//will be set to magic number if we successfully initialise...

	m_pDeviceInterface = pDeviceInterface;

	
	retval = m_pDeviceInterface->GetCUAddress(cuName, &m_cuAddress);

	if (retval != XLNX_OK)
	{
		retval = XLNX_CLOCK_TICK_GENERATOR_ERROR_CU_NAME_NOT_FOUND;
	}


	if (retval == XLNX_OK)
	{
		retval = m_pDeviceInterface->GetCUIndex(cuName, &m_cuIndex);

		if (retval != XLNX_OK)
		{
			retval = XLNX_CLOCK_TICK_GENERATOR_ERROR_CU_NAME_NOT_FOUND;
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
		m_initialisedMagicNumber = IS_INITIALISED_MAGIC_NUMBER;
	}

	return retval;
}







uint32_t ClockTickGenerator::CheckIsInitialised(void)
{
	uint32_t retval = XLNX_OK;

	if (m_initialisedMagicNumber != IS_INITIALISED_MAGIC_NUMBER)
	{
		retval = XLNX_CLOCK_TICK_GENERATOR_ERROR_NOT_INITIALISED;
	}

	return retval;
}







void ClockTickGenerator::IsInitialised(bool* pbIsInitialised)
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





uint32_t ClockTickGenerator::GetCUIndex(uint32_t* pCUIndex)
{
	uint32_t retval = XLNX_OK;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		*pCUIndex = m_cuIndex;
	}

	return retval;
}




uint32_t ClockTickGenerator::GetCUAddress(uint64_t* pCUAddress)
{
	uint32_t retval = XLNX_OK;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		*pCUAddress = m_cuAddress;
	}

	return retval;
}



uint32_t ClockTickGenerator::GetClockFrequency(uint32_t* pFrequencyMHz)
{
	uint32_t retval = XLNX_OK;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		*pFrequencyMHz = m_clockFrequencyMHz;
	}

	return retval;
}




uint32_t ClockTickGenerator::ReadReg32(uint64_t offset, uint32_t* value)
{
	uint32_t retval = XLNX_OK;
	uint64_t address;


	address = m_cuAddress + offset;

	retval = m_pDeviceInterface->ReadReg32(address, value);




	if (retval != XLNX_OK)
	{
		retval = XLNX_CLOCK_TICK_GENERATOR_ERROR_IO_FAILED;
	}

	return retval;
}






uint32_t ClockTickGenerator::WriteReg32(uint64_t offset, uint32_t value)
{
	uint32_t retval = XLNX_OK;
	uint64_t address;


	address = m_cuAddress + offset;

	retval = m_pDeviceInterface->WriteReg32(address, value);


	if (retval != XLNX_OK)
	{
		retval = XLNX_CLOCK_TICK_GENERATOR_ERROR_IO_FAILED;
	}

	return retval;
}




uint32_t ClockTickGenerator::WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask)
{
	uint32_t retval = XLNX_OK;
	uint64_t address;


	address = m_cuAddress + offset;

	retval = m_pDeviceInterface->WriteRegWithMask32(address, value, mask);

	if (retval != XLNX_OK)
	{
		retval = XLNX_CLOCK_TICK_GENERATOR_ERROR_IO_FAILED;
	}

	return retval;
}



uint32_t ClockTickGenerator::BlockReadReg32(uint64_t offset, uint32_t* buffer, uint32_t numWords)
{
	uint32_t retval = XLNX_OK;
	uint64_t address;


	address = m_cuAddress + offset;

	retval = m_pDeviceInterface->BlockReadReg32(address, buffer, numWords);


	if (retval != XLNX_OK)
	{
		retval = XLNX_CLOCK_TICK_GENERATOR_ERROR_IO_FAILED;
	}

	return retval;
}





uint32_t ClockTickGenerator::BlockReadMem32(uint64_t address, uint32_t* buffer, uint32_t numWords)
{
	uint32_t retval = XLNX_OK;

	retval = m_pDeviceInterface->BlockReadMem32(address, buffer, numWords);

	if (retval != XLNX_OK)
	{
		retval = XLNX_CLOCK_TICK_GENERATOR_ERROR_IO_FAILED;
	}

	return retval;
}



uint32_t ClockTickGenerator::CheckStreamIndex(uint32_t streamIndex)
{
	uint32_t retval = XLNX_OK;

	if (streamIndex > NUM_SUPPORTED_TICK_STREAMS)
	{
		retval = XLNX_CLOCK_TICK_GENERATOR_ERROR_STREAM_INDEX_OUT_OF_RANGE;
	}

	return retval;
}




uint32_t ClockTickGenerator::SetTickEnabled(uint32_t streamIndex, bool bEnabled)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t shift;
	uint32_t mask = 0x01;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckStreamIndex(streamIndex);
	}

	if (retval == XLNX_OK)
	{
		if (bEnabled)
		{
			value = 1;
		}
		else
		{
			value = 0;
		}

		shift = streamIndex;

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_CLOCK_TICK_GENERATOR_STREAM_ENABLE_OFFSET, value, mask);
	}

	return retval;
}



uint32_t ClockTickGenerator::GetTickEnabled(uint32_t streamIndex, bool* pbEnabled)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t shift;
	uint32_t mask = 0x01;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckStreamIndex(streamIndex);
	}

	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_CLOCK_TICK_GENERATOR_STREAM_ENABLE_OFFSET, &value);
	}


	if(retval == XLNX_OK)
	{
		shift = streamIndex;

		*pbEnabled = ((value >> shift) & mask);
	}

	return retval;
}










uint32_t ClockTickGenerator::SetTickInterval(uint32_t streamIndex, uint32_t microseconds)
{
	uint32_t retval = XLNX_OK;
	bool bTickWasEnabled = false;
	uint32_t clockCycles;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckStreamIndex(streamIndex);
	}


	//If the tick is enabled, and we are changing the interval, we need to 
	//briefly disable it, change the interval, then re-enable it...

	if (retval == XLNX_OK)
	{
		retval = GetTickEnabled(streamIndex, &bTickWasEnabled);
	}


	if (retval == XLNX_OK)
	{
		if (bTickWasEnabled)
		{
			retval = SetTickEnabled(streamIndex, false);
		}
	}


	if (retval == XLNX_OK)
	{
		retval = ConvertMicrosecondsToClockCycles(microseconds, &clockCycles);
	}


	if(retval == XLNX_OK)
	{

		retval = WriteReg32(XLNX_CLOCK_TICK_GENERATOR_TICK_INTERVAL_OFFSET(streamIndex), clockCycles);
	}


	if (retval == XLNX_OK)
	{
		if (bTickWasEnabled)
		{
			retval = SetTickEnabled(streamIndex, true);
		}
	}

	return retval;
}


uint32_t ClockTickGenerator::GetTickInterval(uint32_t streamIndex, uint32_t* pMicroseconds)
{
	uint32_t retval = XLNX_OK;
	uint32_t clockCycles;

	retval = GetTickIntervalClockCycles(streamIndex, &clockCycles);

	if (retval == XLNX_OK)
	{
		retval = ConvertClockCyclesToMicroseconds(clockCycles, pMicroseconds);
	}

	return retval;
}



uint32_t ClockTickGenerator::GetTickIntervalClockCycles(uint32_t streamIndex, uint32_t* pClockCycles)
{
	uint32_t retval = XLNX_OK;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckStreamIndex(streamIndex);
	}

	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_CLOCK_TICK_GENERATOR_TICK_INTERVAL_OFFSET(streamIndex), pClockCycles);
	}

	return retval;
}


uint32_t ClockTickGenerator::GetStats(Stats* pStats)
{
	uint32_t retval = XLNX_OK;
	uint32_t buffer[XLNX_CLOCK_TICK_GENERATOR_NUM_STATS_REGISTERS];

	memset(buffer, 0, sizeof(buffer));

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = BlockReadReg32(XLNX_CLOCK_TICK_GENERATOR_STATS_START, buffer, XLNX_CLOCK_TICK_GENERATOR_NUM_STATS_REGISTERS);
	}

	if (retval == XLNX_OK)
	{
		pStats->tickCount = buffer[0];

		for (uint32_t i = 0; i < NUM_SUPPORTED_TICK_STREAMS; i++)
		{
			pStats->numTickEventsFired[i] = buffer[1+i];
		}
	}

	return retval;
}



uint32_t ClockTickGenerator::ConvertMicrosecondsToClockCycles(uint32_t microseconds, uint32_t* pClockCycles)
{
	uint32_t retval = XLNX_OK;
	uint64_t cyclesPerMicrosecond;
	uint64_t clockCycles;

	cyclesPerMicrosecond = m_clockFrequencyMHz;

	clockCycles = cyclesPerMicrosecond * microseconds;


	//The HW register is a 32-bit register - we need to check that our calculated number of clock cycles doesn't
	//exceed the 32-bit range.
	if (clockCycles > 0xFFFFFFFF)
	{
		retval = XLNX_CLOCK_TICK_GENERATOR_ERROR_INTERVAL_TOO_LARGE;
	}
	
	if (retval == XLNX_OK)
	{
		*pClockCycles = (uint32_t)clockCycles;
	}

	return retval;
}



uint32_t ClockTickGenerator::ConvertClockCyclesToMicroseconds(uint32_t clockCycles, uint32_t* pMicroseconds)
{
	uint32_t retval = XLNX_OK;
	uint32_t cyclesPerMicrosecond;

	cyclesPerMicrosecond = m_clockFrequencyMHz;

	//Here we are rounding up or down to the nearest whole number of microseconds...
	*pMicroseconds = (uint32_t) (((double)clockCycles / (double)cyclesPerMicrosecond) + 0.5);

	return retval;
}