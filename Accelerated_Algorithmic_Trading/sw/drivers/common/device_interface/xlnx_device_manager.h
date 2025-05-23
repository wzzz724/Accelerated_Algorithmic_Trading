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

#ifndef XLNX_DEVICE_MANAGER_H
#define XLNX_DEVICE_MANAGER_H

#include <cstdint>
#include <iostream>

#include "xrt.h"

#include "xlnx_device_interface.h"

namespace XLNX
{



class DeviceManager
{
public:
	DeviceManager();
	virtual ~DeviceManager();




public:
	uint32_t EnumerateDevices(void);

	uint32_t GetNumEnumeratedDevices(void);
	char* GetEnumeratedDeviceName(uint32_t deviceIndex);
	void GetEnumeratedDeviceBDF(uint32_t deviceIndex, uint32_t* pBus, uint32_t* pDevice, uint32_t* pFunction);

	DeviceInterface* CreateHWDeviceInterface(uint32_t deviceIndex);
	DeviceInterface* CreateVirtualDeviceInterface(void);



private:
	xclDeviceInfo2* m_pDeviceInfoArray;
	uint32_t m_numDevicesEnumerated;


};















}




#endif



