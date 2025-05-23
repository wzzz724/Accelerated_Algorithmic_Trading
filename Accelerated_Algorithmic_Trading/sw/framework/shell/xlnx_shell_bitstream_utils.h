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

#ifndef XLNX_SHELL_BITSTREAM_UTILS_H
#define XLNX_SHELL_BITSTREAM_UTILS_H

#include "xlnx_shell.h"

#include "xrt.h"

using namespace XLNX;



void PrintXCLBINHeaderInfo(Shell* pShell, struct axlf* pAXLF);


void PrintSectionInfoTableHeader(Shell* pShell);
void PrintSectionInfoTableElement(Shell* pShell, uint32_t rowIndex, struct axlf_section_header* pSectionHeader);
void PrintSectionInfoTableFooter(Shell* pShell);
void PrintXCLBINSectionInfo(Shell* pShell, struct axlf* pAXLF);

	

void PrintIPLayoutTableHeader(Shell* pShell);
void PrintIPLayoutTableElement(Shell* pShell, uint32_t rowIndex, struct ip_data* pIPData);
void PrintIPLayoutTableFooter(Shell* pShell);
void PrintXCLBINIPLayout(Shell* pShell, struct axlf* pAXLF);


void PrintMemTopologyTableHeader(Shell* pShell);
void PrintMemTopologyTableElement(Shell* pShell, uint32_t rowIndex, struct mem_data* pMemData);
void PrintMemTopologyTableFooter(Shell* pShell);
void PrintXCLBINMemTopology(Shell* pShell, struct axlf* pAXLF);


void PrintConnectivityTableHeader(Shell* pShell);
void PrintConnectivityTableElement(Shell* pShell, uint32_t rowIndex, struct connection* pConnection);
void PrintConnectivityTableFooter(Shell* pShell);
void PrintXCLBINConnectivity(Shell* pShell, struct axlf* pAXLF);


void PrintClockFrequencyTopologyTableHeader(Shell* pShell);
void PrintClockFrequencyTopologyTableElement(Shell* pShell, uint32_t rowIndex, struct clock_freq* pClockFreq);
void PrintClockFrequencyTopologyTableFooter(Shell* pShell);
void PrintXCLBINClockFrequency(Shell* pShell, struct axlf* pAXLF);

void PrintKernelArgsTableHeader(Shell* pShell);
void PrintKernelArgsTableElement(Shell* pShell, uint32_t rowIndex, string kernelName, string argName, string argType, string argOffset, string argSize, string argId);
void PrintKernelArgsTableFooter(Shell* pShell);
void PrintXCLBINKernelArgs(Shell* pShell, struct axlf* pAXLF);


#endif
