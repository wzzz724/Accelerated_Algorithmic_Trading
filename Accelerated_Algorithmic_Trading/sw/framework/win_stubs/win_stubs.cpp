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

#include "xrt.h"

//TODO
//TODO - the following functions are a fudge to get around the fact they currently seem to be missing from the XRT library on Windows...
//		 when they are added to the official XRT library, the function below can be removed...
//
#if defined (_WIN32) || defined(_WIN64)

#include "windows/uuid.h"


extern "C"
{
	int __imp_xclUnlockDevice(xclDeviceHandle handle)
	{
		return 0;
	}


	int __imp_xclGetSectionInfo(xclDeviceHandle handle, void* section_info, size_t* section_size, enum axlf_section_kind kind, int index)
	{
		return 0;
	}


	int __imp_xclRegRead(xclDeviceHandle handle, uint32_t cu_index, uint32_t offset, uint32_t* datap)
	{
		return 0;
	}

	int __imp_xclRegWrite(xclDeviceHandle handle, uint32_t cu_index, uint32_t offset, uint32_t data)
	{
		return 0;
	}

	int __imp_xclCuName2Index(xclDeviceHandle handle, const char* cu_name, uint32_t* cu_index)
	{
		return 0;
	}


	int __imp_xclIPName2Index(xclDeviceHandle handle, const char* cu_name, uint32_t* cu_index)
	{
		return 0;
	}

	int uuid_parse(char* in, unsigned char* uu)
	{
		return 0;
	}

	int __imp_xclCreateWriteQueue(xclDeviceHandle handle, struct xclQueueContext* q_ctx, uint64_t* q_hdl)
	{
		*q_hdl = 1;
		return 0;
	}


	int __imp_xclCreateReadQueue(xclDeviceHandle handle, struct xclQueueContext* q_ctx, uint64_t* q_hdl)
	{
		*q_hdl = 1;
		return 0;
	}


	int __imp_xclDestroyQueue(xclDeviceHandle handle, uint64_t q_hdl)
	{
		return 0;
	}


	int __imp_xclWriteQueue(xclDeviceHandle handle, uint64_t q_hdl, struct xclQueueRequest* wr_req)
	{
		return 0;
	}

	int __imp_xclReadQueue(xclDeviceHandle handle, uint64_t q_hdl, struct xclQueueRequest* wr_req)
	{
		return 0;
	}


	void* __imp_xclAllocQDMABuf(xclDeviceHandle handle, size_t size, uint64_t* buf_hdl)
	{
		void* ptr = nullptr;

		//TODO

		return ptr;
	}


	int __imp_xclFreeQDMABuf(xclDeviceHandle handle, uint64_t buf_hdl)
	{
		return 0;
	}

	int __imp_xclPollQueue(xclDeviceHandle handle, uint64_t q_hdl, int min_compl,
		int max_compl, struct xclReqCompletion* comps,
		int* actual_compl, int timeout)
	{
		return 0;
	}

	int __imp_xclSetQueueOpt(xclDeviceHandle handle, uint64_t q_hdl, int type, uint32_t val)
	{
		return 0;
	}

	int __imp_xrtXclbinUUID(xclDeviceHandle handle, xuid_t out)
	{
		return 0;
	}
	
}
#endif
