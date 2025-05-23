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

#ifndef XLNX_ETHERNET_H
#define XLNX_ETHERNET_H


#include <cstdint>

#include "xlnx_device_interface.h"

#include "xlnx_ethernet_error_codes.h"
#include "xlnx_ethernet_types.h"


namespace XLNX
{

class Ethernet
{


public:
	Ethernet();
	virtual ~Ethernet();


public:
	uint32_t Initialise(DeviceInterface* pDeviceInterface, const char* cuName);



public:
	static const uint32_t MAX_SUPPORTED_CHANNELS = 4;

	//the HW block can be built to support either 2 or 4 channels...
	uint32_t GetNumSupportedChannels(uint32_t* pNumChannels);



public: //Channel Control

	//The following functions control MAC filtering behaviour.
	//When either unicast or multicast promiscuous mode is DISABLED, any corresponding frame with a MAC address
	//that does NOT match the specified MAC filter address will be discarded.

	uint32_t SetMACFilterAddress(uint32_t channel, uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f);
	uint32_t GetMACFilterAddress(uint32_t channel, uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d, uint8_t* e, uint8_t* f);

	uint32_t SetUnicastPromiscuousMode(uint32_t channel, bool bEnabled);
	uint32_t GetUnicastPromiscuousMode(uint32_t channel, bool* pbEnabled);

	uint32_t SetMulticastPromiscuousMode(uint32_t channel, bool bEnabled);
	uint32_t GetMulticastPromiscuousMode(uint32_t channel, bool* pbEnabled);



	//The following functions allow the configuration of the threshold value at which data is released from the Tx Data FIFO
	uint32_t SetTxFIFOThreshold(uint32_t channel, uint8_t thresholdValue);
	uint32_t GetTxFIFOThreshold(uint32_t channel, uint8_t* pThresholdValue);


	uint32_t SetRxCutThroughFIFO(uint32_t channel, bool bEnabled);
	uint32_t GetRxCutThroughFIFO(uint32_t channel, bool* pbEnabled);




public: //Channel Status

	typedef struct
	{
		bool bLockedLive;
		bool bLockedLatchedLow;

	}RxBlockLock;

	typedef struct
	{
		GTRxBufStatus statusLive;

		bool bUnderflowLatched;
		bool bOverflowLatched;
	}RxBufStatus;

	typedef struct
	{
		bool bFIFOHalfFull;
		bool bFIFOOverflowUnderflow;

		bool bFIFOHalfFullLatchedHigh;
		bool bFIFOOverflowUnderflowLatchedHigh;
	}TxBufStatus;


	typedef struct
	{
		bool bPowerGoodLive;
		bool bPowerGoodLatchedLow; 
	}GTPowerGood;

	typedef struct
	{
		bool bDataFIFOOverflowLive;
		bool bCommandFIFOOverflowLive;

		bool bDataFIFOOverflowLatched;
		bool bCommandFIFOOverflowLatched;
	}RxTrafficProcessorStatus;


	typedef struct
	{
		bool bFIFOFullLive;
		bool bFIFOFullLatched;
	}TxTrafficProcessorStatus;

	typedef struct
	{
		bool bEyescanDataErrorLatchedHigh;
	}EyescanStatus;



	typedef struct
	{
		RxBlockLock					rxBlockLock;
		RxBufStatus					rxBufStatus;
		TxBufStatus					txBufStatus;
		GTPowerGood					gtPowerGood;
		RxTrafficProcessorStatus	rxTrafficProcStatus;
		TxTrafficProcessorStatus	txTrafficProcStatus;
		EyescanStatus				eyescanStatus;

	}StatusFlags;


	uint32_t GetStatusFlags(uint32_t channel, StatusFlags* pStatusFlags);
	uint32_t ClearStatusLatches(uint32_t channel);










public: //GT Control

	uint32_t SetLoopbackType(uint32_t channel, GTLoopback loopback);
	uint32_t GetLoopbackType(uint32_t channel, GTLoopback* pLoopback);
	


	uint32_t SetTxOutClockSelect(uint32_t channel, GTClockSelect clockSelect);
	uint32_t GetTxOutClockSelect(uint32_t channel, GTClockSelect* pClockSelect);

	uint32_t SetRxOutClockSelect(uint32_t channel, GTClockSelect clockSelect);
	uint32_t GetRxOutClockSelect(uint32_t channel, GTClockSelect* pClockSelect);

	

	uint32_t SetTxPolarity(uint32_t channel, GTPolarity polarity);
	uint32_t GetTxPolarity(uint32_t channel, GTPolarity* pPolarity);

	uint32_t SetRxPolarity(uint32_t channel, GTPolarity polarity);
	uint32_t GetRxPolarity(uint32_t channel, GTPolarity* pPolarity);





public: //GT Tx Driver Characteristics

	//The following functions can be used to manipulate advanced transmit driver settings for a GT.
	//Please consult UG578 for details on these settings.

	static const uint32_t MAX_TX_DIFF_CONTROL_COEFFICIENT = 0x1F;
	uint32_t SetTxDiffControl(uint32_t channel, uint32_t coefficient);
	uint32_t GetTxDiffControl(uint32_t channel, uint32_t* pCoefficient);


	static const uint32_t MAX_TX_MAIN_CURSOR_COEFFICIENT = 0x7F;
	uint32_t SetTxMainCursor(uint32_t channel, uint32_t coefficient);
	uint32_t GetTxMainCursor(uint32_t channel, uint32_t* pCoefficient);


	static const uint32_t MAX_TX_POST_CURSOR_COEFFICIENT = 0x1F;
	uint32_t SetTxPostCursor(uint32_t channel, uint32_t coefficient);
	uint32_t GetTxPostCursor(uint32_t channel, uint32_t* pCoefficient);


	static const uint32_t MAX_TX_PRE_CURSOR_COEFFICIENT = 0x1F;
	uint32_t SetTxPreCursor(uint32_t channel, uint32_t coefficient);
	uint32_t GetTxPreCursor(uint32_t channel, uint32_t* pCoefficient);


	uint32_t SetTxInhibit(uint32_t channel, bool bEnabled);
	uint32_t GetTxInhibit(uint32_t channel, bool* pbEnabled);


	uint32_t SetTxElectricalIdle(uint32_t channel, bool bEnabled);
	uint32_t GetTxElectricalIdle(uint32_t channel, bool* pbEnabled);





public: //GT Rx Equalizer

	//The following functions can be used to manipulate advanced receiver settings for a GT.
	//Please consult UG578 for details on these settings.

	uint32_t SetRxEqualizationMode(uint32_t channel, GTEqualizationMode equalizationMode);
	uint32_t GetRxEqualizationMode(uint32_t channel, GTEqualizationMode* pEqualizationMode);

	uint32_t ResetLFEDPMDataPath(uint32_t channel);








public: //Test Pattern Generator/Checker
	uint32_t SetTxTestPattern(uint32_t channel, GTTestPattern testPattern);
	uint32_t GetTxTestPattern(uint32_t channel, GTTestPattern* pTestPattern);

	uint32_t SetTxErrorInjection(uint32_t channel, bool bEnabled);
	uint32_t GetTxErrorInjection(uint32_t channel, bool* pbEnabled);


	uint32_t SetRxExpectedTestPattern(uint32_t channel, GTTestPattern testPattern);
	uint32_t GetRxExpectedTestPattern(uint32_t channel, GTTestPattern* pTestPattern);

	uint32_t GetRxTestPatternStatus(uint32_t channel, bool* pbLocked, bool* pbLiveError, bool* pbLatchedError);
	uint32_t ClearRxTestPatternErrorLatch(uint32_t channel);


	uint32_t ResetRxTestPatternErrorCount(uint32_t channel); 
	//TODO need DRP bridge to retrieve actual error count...





public:
	uint32_t TriggerEyescan(uint32_t channel);





public: //Stats

	typedef struct
	{
		uint32_t numFIFOOverflowDroppedFrames; 
		uint32_t numUnicastFilterDroppedFrames;		
		uint32_t numMulticastFilterDroppedFrames;	
		uint32_t numOversizedDroppedFrames;
		uint32_t txFIFOUnderflowCount;

	}Stats;

	uint32_t GetStats(uint32_t channel, Stats* pStats);




public: //Miscellaneous Resets

	uint32_t SetKernelReset(bool bInReset);
	uint32_t GetKernelReset(bool* pbInReset);

	uint32_t SetTxFIFOReset(uint32_t channel, bool bInReset);
	uint32_t GetTxFIFOReset(uint32_t channel, bool* pbInReset);

	uint32_t SetRxFIFOReset(uint32_t channel, bool bInReset);
	uint32_t GetRxFIFOReset(uint32_t channel, bool* pbInReset);

	uint32_t SetTxTrafficProcessorReset(uint32_t channel, bool bInReset);
	uint32_t GetTxTrafficProcessorReset(uint32_t channel, bool* pbInReset);

	uint32_t SetRxTrafficProcessorReset(uint32_t channel, bool bInReset);
	uint32_t GetRxTrafficProcessorReset(uint32_t channel, bool* pbInReset);

	uint32_t SetRxPMAReset(uint32_t channel, bool bInReset);
	uint32_t GetRxPMAReset(uint32_t channel, bool* pbInReset);

	uint32_t SetRxBufReset(uint32_t channel, bool bInReset);
	uint32_t GetRxBufReset(uint32_t channel, bool* pbInReset);

	uint32_t SetRxPCSReset(uint32_t channel, bool bInReset);
	uint32_t GetRxPCSReset(uint32_t channel, bool* pbInReset);

	uint32_t SetEyeScanReset(uint32_t channel, bool bInReset);
	uint32_t GetEyeScanReset(uint32_t channel, bool* pbInReset);

	uint32_t SetRxGTWizReset(uint32_t channel, bool bInReset);
	uint32_t GetRxGTWizReset(uint32_t channel, bool* pbInReset);

	uint32_t SetTxPMAReset(uint32_t channel, bool bInReset);
	uint32_t GetTxPMAReset(uint32_t channel, bool* pbInReset);

	uint32_t SetTxPCSReset(uint32_t channel, bool bInReset);
	uint32_t GetTxPCSReset(uint32_t channel, bool* pbInReset);

	uint32_t SetTxGTWizReset(uint32_t channel, bool bInReset);
	uint32_t GetTxGTWizReset(uint32_t channel, bool* pbInReset);





public:
    void IsInitialised(bool* pbIsInitialised);
	uint32_t GetCUIndex(uint32_t* pCUIndex);
	uint32_t GetCUAddress(uint64_t* pCUAddress);


public:
	//The following function is used to enable/disable CONFIGURATION of the specified channel.
	//This was introduced to allow an instance of the block to be disabled due to HW limitations on the underlying card
	//e.g. insufficient MAC addresses 
	void SetChannelConfigurationAllowed(uint32_t channel, bool bAllowed);
	void GetChannelConfigurationAllowed(uint32_t channel, bool* pbAllowed);


protected:
	uint32_t ReadReg32(uint64_t offset, uint32_t* value);
	uint32_t WriteReg32(uint64_t offset, uint32_t value);
	uint32_t WriteRegWithMask32(uint64_t offset, uint32_t value, uint32_t mask);

protected: 
	uint32_t InitialiseNumSupportedChannels(void);

protected: //Internal Parameter Validation Functions
	uint32_t CheckIsInitialised(void);
	uint32_t CheckChannel(uint32_t channel);
	uint32_t CheckChannelConfigurationAllowed(uint32_t channel);
	uint32_t CheckLoopback(GTLoopback loopback);
	uint32_t CheckClockSelect(GTClockSelect clockSelect);
	uint32_t CheckPolarity(GTPolarity polarity);
	uint32_t CheckTxDiffControl(uint32_t value);
	uint32_t CheckTxMainCursor(uint32_t coefficient);
	uint32_t CheckTxPostCursor(uint32_t coefficient);
	uint32_t CheckTxPreCursor(uint32_t coefficient);
	uint32_t CheckTestPattern(GTTestPattern testPattern);
	uint32_t CheckEqualizationMode(GTEqualizationMode equalizationMode);



protected: //Internal Status Functions
	uint32_t GetRxBlockLock(uint32_t channel, RxBlockLock* pStatus);
	uint32_t ClearRxBlockLockLatches(uint32_t channel);

	uint32_t GetRxBufStatus(uint32_t channel, RxBufStatus* pStatus);
	uint32_t ClearRxBufStatusLatches(uint32_t channel);

	uint32_t GetTxBufStatus(uint32_t channel, TxBufStatus* pStatus);
	uint32_t ClearTxBufStatusLatches(uint32_t channel);

	uint32_t GetGTPowerGoodStatus(uint32_t channel, GTPowerGood* pStatus);
	uint32_t ClearGTPowerGoodLatches(uint32_t channel);

	uint32_t GetRxTrafficProcessorStatus(uint32_t channel, RxTrafficProcessorStatus* pStatus);
	uint32_t ClearRxTrafficProcessorStatusLatches(uint32_t channel);

	uint32_t GetTxTrafficProcessorStatus(uint32_t channel, TxTrafficProcessorStatus* pStatus);
	uint32_t ClearTxTrafficProcessorStatusLatches(uint32_t channel);

	uint32_t GetEyescanStatus(uint32_t channel, EyescanStatus* pStatus);
	uint32_t ClearEyescanStatusLatches(uint32_t channel);



protected:
	uint32_t m_initialisedMagicNumber;

	uint64_t m_cuAddress;
	uint32_t m_cuIndex;

	uint32_t m_numSupportedChannels;

	DeviceInterface* m_pDeviceInterface;


protected:
	bool m_bChannelConfigurationAllowed[MAX_SUPPORTED_CHANNELS];


}; //class Ethernet



} //namespace XLNX



#endif //XLNX_ETHERNET_H

