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

#include "xlnx_ethernet.h"
#include "xlnx_ethernet_address_map.h"
#include "xlnx_ethernet_error_codes.h"
#include "xlnx_ethernet_types.h"
using namespace XLNX;





uint32_t Ethernet::CheckLoopback(GTLoopback loopback)
{
	uint32_t retval = XLNX_OK;

	switch (loopback)
	{
	case(GTLoopback::NO_LOOPACK):
	case(GTLoopback::NEAR_END_PCS_LOOPBACK):
	case(GTLoopback::NEAR_END_PMA_LOOPBACK):
	case(GTLoopback::FAR_END_PMA_LOOPBACK):
	case(GTLoopback::FAR_END_PCS_LOOPBACK):
	{
		retval = XLNX_OK;
		break;
	}

	default:
	{
		retval = XLNX_ETHERNET_ERROR_INVALID_LOOPBACK_TYPE;
		break;
	}
	}


	return retval;
}





uint32_t Ethernet::CheckClockSelect(GTClockSelect clockSelect)
{
	uint32_t retval = XLNX_OK;

	switch (clockSelect)
	{
		case(GTClockSelect::STATIC_1):
		case(GTClockSelect::OUTCLKPCS):
		case(GTClockSelect::OUTCLKPMA):
		case(GTClockSelect::PLLREFCLK_DIV1):
		case(GTClockSelect::PLLREFCLK_DIV2):
		case(GTClockSelect::PROGDIVCLK):
		{
			//OK;
			break;
		}

		default:
		{
			retval = XLNX_ETHERNET_ERROR_INVALID_CLOCK_SELECT_TYPE;
			break;
		}
	}

	return retval;
}





uint32_t Ethernet::CheckPolarity(GTPolarity polarity)
{
	uint32_t retval = XLNX_OK;

	switch (polarity)
	{
		case(GTPolarity::NORMAL):
		case(GTPolarity::INVERTED):
		{
			//OK;
			break;
		}

		default:
		{
			retval = XLNX_ETHERNET_ERROR_INVALID_POLARITY_TYPE;
			break;
		}
	}

	return retval;
}




uint32_t Ethernet::CheckEqualizationMode(GTEqualizationMode equalizationMode)
{
	uint32_t retval = XLNX_OK;

	switch (equalizationMode)
	{
		case(GTEqualizationMode::DFE):
		case(GTEqualizationMode::LPM):
		{
			//OK;
			break;
		}

		default:
		{
			retval = XLNX_ETHERNET_ERROR_INVALID_EQUALIZATION_MODE;
			break;
		}
	}

	return retval;
}









uint32_t Ethernet::CheckTxDiffControl(uint32_t value)
{
	uint32_t retval = XLNX_OK;
	
	if (value > MAX_TX_DIFF_CONTROL_COEFFICIENT)
	{
		retval = XLNX_ETHERNET_ERROR_PARAMETER_OUT_OF_RANGE;
	}

	return retval;
}




uint32_t Ethernet::CheckTxMainCursor(uint32_t coefficient)
{
	uint32_t retval = XLNX_OK;

	if (coefficient > MAX_TX_MAIN_CURSOR_COEFFICIENT)
	{
		retval = XLNX_ETHERNET_ERROR_PARAMETER_OUT_OF_RANGE;
	}

	return retval;
}




uint32_t Ethernet::CheckTxPostCursor(uint32_t coefficient)
{
	uint32_t retval = XLNX_OK;

	if (coefficient > MAX_TX_POST_CURSOR_COEFFICIENT)
	{
		retval = XLNX_ETHERNET_ERROR_PARAMETER_OUT_OF_RANGE;
	}

	return retval;
}




uint32_t Ethernet::CheckTxPreCursor(uint32_t coefficient)
{
	uint32_t retval = XLNX_OK;

	if (coefficient > MAX_TX_PRE_CURSOR_COEFFICIENT)
	{
		retval = XLNX_ETHERNET_ERROR_PARAMETER_OUT_OF_RANGE;
	}

	return retval;
}







uint32_t Ethernet::SetLoopbackType(uint32_t channel, GTLoopback loopback)
{
	uint32_t retval = XLNX_OK;
	uint32_t value = 0;
	uint32_t mask = 0x7;
	uint32_t shift = 0;

	retval = CheckIsInitialised();


	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = CheckLoopback(loopback);
	}


	if (retval == XLNX_OK)
	{
		switch (loopback)
		{
			case(GTLoopback::NO_LOOPACK):
			{
				value = 0x00;
				break;
			}

			case(GTLoopback::NEAR_END_PCS_LOOPBACK):
			{
				value = 0x01;
				break;
			}

			case(GTLoopback::NEAR_END_PMA_LOOPBACK):
			{
				value = 0x02;
				break;
			}

			case(GTLoopback::FAR_END_PMA_LOOPBACK):
			{
				value = 0x04;
				break;
			}

			case(GTLoopback::FAR_END_PCS_LOOPBACK):
			{
				value = 0x06;
				break;
			}

			default:
			{
				retval = XLNX_ETHERNET_ERROR_INVALID_LOOPBACK_TYPE;
				break;
			}
		}
	}




	if (retval == XLNX_OK)
	{
		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_LOOPBACK_OFFSET(channel), value, mask);
	}



	return retval;
}










uint32_t Ethernet::GetLoopbackType(uint32_t channel, GTLoopback* pLoopback)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x07;
	uint32_t shift = 0;


	retval = CheckIsInitialised();


	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}


	if (retval == XLNX_OK)
	{

		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_LOOPBACK_OFFSET(channel), &value);
	}


	if (retval == XLNX_OK)
	{
		value = (value >> shift) & mask;

		switch (value)
		{
			case(0x00):
			{
				*pLoopback = GTLoopback::NO_LOOPACK;
				break;
			}

			case(0x01):
			{
				*pLoopback = GTLoopback::NEAR_END_PCS_LOOPBACK;
				break;
			}


			case(0x02):
			{
				*pLoopback = GTLoopback::NEAR_END_PMA_LOOPBACK;
				break;
			}


			case(0x04):
			{
				*pLoopback = GTLoopback::FAR_END_PMA_LOOPBACK;
				break;
			}


			case(0x06):
			{
				*pLoopback = GTLoopback::FAR_END_PCS_LOOPBACK;
				break;
			}


			default:
			{
				retval = XLNX_ETHERNET_ERROR_INVALID_LOOPBACK_TYPE;
				break;
			}
		}
	}


	return retval;
}



uint32_t Ethernet::SetTxOutClockSelect(uint32_t channel, GTClockSelect clockSelect)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x07;
	uint32_t shift = 20;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = CheckClockSelect(clockSelect);
	}

	if (retval == XLNX_OK)
	{
		value = (uint32_t)clockSelect;

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_CLOCK_SELECT_OFFSET(channel), value, mask);
	}

	return retval;
}






uint32_t Ethernet::GetTxOutClockSelect(uint32_t channel, GTClockSelect* pClockSelect)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x07;
	uint32_t shift = 20;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_CLOCK_SELECT_OFFSET(channel), &value);
	}

	if (retval == XLNX_OK)
	{
		value = (value >> shift) & mask;

		*pClockSelect = (GTClockSelect)value;
	}


	return retval;
}







uint32_t Ethernet::SetRxOutClockSelect(uint32_t channel, GTClockSelect clockSelect)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x07;
	uint32_t shift = 4;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = CheckClockSelect(clockSelect);
	}

	if (retval == XLNX_OK)
	{
		value = (uint32_t)clockSelect;

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_CLOCK_SELECT_OFFSET(channel), value, mask);
	}

	return retval;
}







uint32_t Ethernet::GetRxOutClockSelect(uint32_t channel, GTClockSelect* pClockSelect)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x07;
	uint32_t shift = 4;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_CLOCK_SELECT_OFFSET(channel), &value);
	}

	if (retval == XLNX_OK)
	{
		value = (value >> shift)& mask;

		*pClockSelect = (GTClockSelect)value;
	}

	return retval;
}




uint32_t Ethernet::SetTxPolarity(uint32_t channel, GTPolarity polarity)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 16;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = CheckPolarity(polarity);
	}

	if (retval == XLNX_OK)
	{
		value = (uint32_t)polarity;

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_POLARITY_OFFSET(channel), value, mask);
	}

	return retval;
}



uint32_t Ethernet::GetTxPolarity(uint32_t channel, GTPolarity* pPolarity)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 16;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_POLARITY_OFFSET(channel), &value);
	}

	if (retval == XLNX_OK)
	{
		value = (value >> shift)& mask;

		*pPolarity = (GTPolarity)value;
	}

	return retval;
}



uint32_t Ethernet::SetRxPolarity(uint32_t channel, GTPolarity polarity)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 0;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = CheckPolarity(polarity);
	}

	if (retval == XLNX_OK)
	{
		value = (uint32_t)polarity;

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_POLARITY_OFFSET(channel), value, mask);
	}

	return retval;
}




uint32_t Ethernet::GetRxPolarity(uint32_t channel, GTPolarity* pPolarity)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 0;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_POLARITY_OFFSET(channel), &value);
	}

	if (retval == XLNX_OK)
	{
		value = (value >> shift)& mask;

		*pPolarity = (GTPolarity)value;
	}

	return retval;
}





uint32_t Ethernet::SetTxDiffControl(uint32_t channel, uint32_t coefficient)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = MAX_TX_DIFF_CONTROL_COEFFICIENT;
	uint32_t shift = 0;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = CheckTxDiffControl(coefficient);
	}

	if (retval == XLNX_OK)
	{
		value = coefficient;

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_TX_DRIVER_OFFSET(channel), value, mask);
	}

	return retval;
}





uint32_t Ethernet::GetTxDiffControl(uint32_t channel, uint32_t* pCoefficient)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = MAX_TX_DIFF_CONTROL_COEFFICIENT;
	uint32_t shift = 0;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_TX_DRIVER_OFFSET(channel), &value);
	}


	if (retval == XLNX_OK)
	{
		*pCoefficient = (value >> shift) & mask;
	}

	return retval;
}





uint32_t Ethernet::SetTxMainCursor(uint32_t channel, uint32_t coefficient)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = MAX_TX_MAIN_CURSOR_COEFFICIENT;
	uint32_t shift = 8;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = CheckTxMainCursor(coefficient);
	}

	if (retval == XLNX_OK)
	{
		value = coefficient;

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_TX_DRIVER_OFFSET(channel), value, mask);
	}

	return retval;
}




uint32_t Ethernet::GetTxMainCursor(uint32_t channel, uint32_t* pCoefficient)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = MAX_TX_MAIN_CURSOR_COEFFICIENT;
	uint32_t shift = 8;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_TX_DRIVER_OFFSET(channel), &value);
	}


	if (retval == XLNX_OK)
	{
		*pCoefficient = (value >> shift)& mask;
	}

	return retval;
}


uint32_t Ethernet::SetTxPostCursor(uint32_t channel, uint32_t coefficient)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = MAX_TX_POST_CURSOR_COEFFICIENT;
	uint32_t shift = 16;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = CheckTxPostCursor(coefficient);
	}

	if (retval == XLNX_OK)
	{
		value = coefficient;

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_TX_DRIVER_OFFSET(channel), value, mask);
	}

	return retval;
}





uint32_t Ethernet::GetTxPostCursor(uint32_t channel, uint32_t* pCoefficient)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = MAX_TX_POST_CURSOR_COEFFICIENT;
	uint32_t shift = 16;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_TX_DRIVER_OFFSET(channel), &value);
	}


	if (retval == XLNX_OK)
	{
		*pCoefficient = (value >> shift)& mask;
	}

	return retval;
}



uint32_t Ethernet::SetTxPreCursor(uint32_t channel, uint32_t coefficient)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = MAX_TX_PRE_CURSOR_COEFFICIENT;
	uint32_t shift = 24;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = CheckTxPreCursor(coefficient);
	}

	if (retval == XLNX_OK)
	{
		value = coefficient;

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_TX_DRIVER_OFFSET(channel), value, mask);
	}

	return retval;
}






uint32_t Ethernet::GetTxPreCursor(uint32_t channel, uint32_t* pCoefficient)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = MAX_TX_PRE_CURSOR_COEFFICIENT;
	uint32_t shift = 24;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_TX_DRIVER_OFFSET(channel), &value);
	}


	if (retval == XLNX_OK)
	{
		*pCoefficient = (value >> shift)& mask;
	}

	return retval;
}






uint32_t Ethernet::SetTxInhibit(uint32_t channel, bool bEnabled)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 31;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		if (bEnabled)
		{
			value = 0x01;
		}
		else
		{
			value = 0x00;
		}

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_TX_DRIVER_OFFSET(channel), value, mask);
	}

	return retval;
}




uint32_t Ethernet::GetTxInhibit(uint32_t channel, bool* pbEnabled)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 31;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_TX_DRIVER_OFFSET(channel), &value);
	}


	if (retval == XLNX_OK)
	{
		value = (value >> shift) & mask;

		if (value != 0)
		{
			*pbEnabled = true;
		}
		else
		{
			*pbEnabled = false;
		}
	}

	return retval;
}





uint32_t Ethernet::SetTxElectricalIdle(uint32_t channel, bool bEnabled)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 7;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		if (bEnabled)
		{
			value = 0x01;
		}
		else
		{
			value = 0x00;
		}

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_TX_RESET_INIT_CONTROL_OFFSET(channel), value, mask);
	}

	return retval;
}





uint32_t Ethernet::GetTxElectricalIdle(uint32_t channel, bool* pbEnabled)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 7;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_TX_RESET_INIT_CONTROL_OFFSET(channel), &value);
	}


	if (retval == XLNX_OK)
	{
		value = (value >> shift)& mask;

		if (value != 0)
		{
			*pbEnabled = true;
		}
		else
		{
			*pbEnabled = false;
		}
	}

	return retval;
}





uint32_t Ethernet::SetRxEqualizationMode(uint32_t channel, GTEqualizationMode equalizationMode)
{
	uint32_t retval = XLNX_OK;;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 0;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = CheckEqualizationMode(equalizationMode);
	}

	if (retval == XLNX_OK)
	{
		value = (uint32_t)equalizationMode;

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_RX_EQUALIZER_OFFSET(channel), value, mask);
	}

	return retval;
}





uint32_t Ethernet::GetRxEqualizationMode(uint32_t channel, GTEqualizationMode* pEqualizationMode)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 0;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_RX_EQUALIZER_OFFSET(channel), &value);
	}


	if (retval == XLNX_OK)
	{
		*pEqualizationMode = (GTEqualizationMode)((value >> shift) & mask);
	}

	return retval;
}





uint32_t Ethernet::ResetLFEDPMDataPath(uint32_t channel)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 1;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	
	if (retval == XLNX_OK)
	{
		value = 1;

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_RX_EQUALIZER_OFFSET(channel), value, mask);
	}


	if (retval == XLNX_OK)
	{
		value = 0;
		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_RX_EQUALIZER_OFFSET(channel), value, mask);
	}

	return retval;

}








uint32_t Ethernet::SetRxPMAReset(uint32_t channel, bool bInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 1;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{

		if (bInReset)
		{
			value = 1;
		}
		else
		{
			value = 0;
		}

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_RX_RESET_INIT_CONTROL_OFFSET(channel), value, mask);
	}



	return retval;
}




uint32_t Ethernet::GetRxPMAReset(uint32_t channel, bool* pbInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 1;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}


	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_RX_RESET_INIT_CONTROL_OFFSET(channel), &value);
	}


	if (retval == XLNX_OK)
	{
		*pbInReset = (bool)((value >> shift) & mask);
	}



	return retval;
}






uint32_t Ethernet::SetRxBufReset(uint32_t channel, bool bInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 2;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{

		if (bInReset)
		{
			value = 1;
		}
		else
		{
			value = 0;
		}

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_RX_RESET_INIT_CONTROL_OFFSET(channel), value, mask);
	}


	return retval;
}



uint32_t Ethernet::GetRxBufReset(uint32_t channel, bool* pbInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 2;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}


	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_RX_RESET_INIT_CONTROL_OFFSET(channel), &value);
	}


	if (retval == XLNX_OK)
	{
		*pbInReset = (bool)((value >> shift) & mask);
	}



	return retval;
}








uint32_t Ethernet::SetRxPCSReset(uint32_t channel, bool bInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 3;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{

		if (bInReset)
		{
			value = 1;
		}
		else
		{
			value = 0;
		}

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_RX_RESET_INIT_CONTROL_OFFSET(channel), value, mask);
	}

	return retval;
}






uint32_t Ethernet::GetRxPCSReset(uint32_t channel, bool* pbInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 3;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}


	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_RX_RESET_INIT_CONTROL_OFFSET(channel), &value);
	}


	if (retval == XLNX_OK)
	{
		*pbInReset = (bool)((value >> shift)& mask);
	}



	return retval;
}






uint32_t Ethernet::SetEyeScanReset(uint32_t channel, bool bInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 8;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{

		if (bInReset)
		{
			value = 1;
		}
		else
		{
			value = 0;
		}

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_RX_RESET_INIT_CONTROL_OFFSET(channel), value, mask);
	}

	return retval;
}



uint32_t Ethernet::GetEyeScanReset(uint32_t channel, bool* pbInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 8;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}


	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_RX_RESET_INIT_CONTROL_OFFSET(channel), &value);
	}


	if (retval == XLNX_OK)
	{
		*pbInReset = (bool)((value >> shift) & mask);
	}



	return retval;
}









uint32_t Ethernet::SetRxGTWizReset(uint32_t channel, bool bInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 28;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{

		if (bInReset)
		{
			value = 1;
		}
		else
		{
			value = 0;
		}

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_RX_RESET_INIT_CONTROL_OFFSET(channel), value, mask);
	}

	return retval;
}





uint32_t Ethernet::GetRxGTWizReset(uint32_t channel, bool* pbInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 28;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}


	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_RX_RESET_INIT_CONTROL_OFFSET(channel), &value);
	}


	if (retval == XLNX_OK)
	{
		*pbInReset = (bool)((value >> shift) & mask);
	}



	return retval;
}





uint32_t Ethernet::SetTxPMAReset(uint32_t channel, bool bInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 1;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{

		if (bInReset)
		{
			value = 1;
		}
		else
		{
			value = 0;
		}

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_TX_RESET_INIT_CONTROL_OFFSET(channel), value, mask);
	}



	return retval;
}





uint32_t Ethernet::GetTxPMAReset(uint32_t channel, bool* pbInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 1;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}


	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_TX_RESET_INIT_CONTROL_OFFSET(channel), &value);
	}


	if (retval == XLNX_OK)
	{
		*pbInReset = (bool)((value >> shift) & mask);
	}



	return retval;
}





uint32_t Ethernet::SetTxPCSReset(uint32_t channel, bool bInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 3;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{

		if (bInReset)
		{
			value = 1;
		}
		else
		{
			value = 0;
		}

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_TX_RESET_INIT_CONTROL_OFFSET(channel), value, mask);
	}



	return retval;
}




uint32_t Ethernet::GetTxPCSReset(uint32_t channel, bool* pbInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 3;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}


	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_TX_RESET_INIT_CONTROL_OFFSET(channel), &value);
	}


	if (retval == XLNX_OK)
	{
		*pbInReset = (bool)((value >> shift)& mask);
	}



	return retval;
}






uint32_t Ethernet::SetTxGTWizReset(uint32_t channel, bool bInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 28;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{

		if (bInReset)
		{
			value = 1;
		}
		else
		{
			value = 0;
		}

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_TX_RESET_INIT_CONTROL_OFFSET(channel), value, mask);
	}


	

	return retval;
}



uint32_t Ethernet::GetTxGTWizReset(uint32_t channel, bool* pbInReset)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 28;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}


	if (retval == XLNX_OK)
	{
		retval = ReadReg32(XLNX_ETHERNET_GT_CONTROL_TX_RESET_INIT_CONTROL_OFFSET(channel), &value);
	}


	if (retval == XLNX_OK)
	{
		*pbInReset = (bool)((value >> shift)& mask);
	}



	return retval;
}





uint32_t Ethernet::TriggerEyescan(uint32_t channel)
{
	uint32_t retval = XLNX_OK;
	uint32_t value;
	uint32_t mask = 0x01;
	uint32_t shift = 0;

	retval = CheckIsInitialised();

	if (retval == XLNX_OK)
	{
		retval = CheckChannel(channel);
	}

	if (retval == XLNX_OK)
	{

		value = 1;

		value = value << shift;
		mask = mask << shift;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_RX_MARGIN_ANALYSIS_OFFSET(channel), value, mask);
	}


	if (retval == XLNX_OK)
	{
		value = 0;

		retval = WriteRegWithMask32(XLNX_ETHERNET_GT_CONTROL_RX_MARGIN_ANALYSIS_OFFSET(channel), value, mask);
	}

	return retval;
}



