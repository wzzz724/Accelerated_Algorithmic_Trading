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

#include <cinttypes>

#include "xlnx_shell_bitstream_utils.h"

#include <boost/foreach.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>



using namespace XLNX;
using boost::property_tree::ptree;




static const char* LINE_STRING = "----------------------------------------------------------------------------------";


#define STR_CASE(TAG)	case(TAG):					\
						{							\
							pString = (char*) #TAG;	\
							break;					\
						}




static char* AXLFSectionKindToString(enum axlf_section_kind sectionKind)
{
	char* pString;

	switch (sectionKind)
	{
		STR_CASE(BITSTREAM)
		STR_CASE(CLEARING_BITSTREAM)
		STR_CASE(EMBEDDED_METADATA)
		STR_CASE(FIRMWARE)
		STR_CASE(DEBUG_DATA)
		STR_CASE(SCHED_FIRMWARE)
		STR_CASE(MEM_TOPOLOGY)
		STR_CASE(CONNECTIVITY)
		STR_CASE(IP_LAYOUT)
		STR_CASE(DEBUG_IP_LAYOUT)
		STR_CASE(DESIGN_CHECK_POINT)
		STR_CASE(CLOCK_FREQ_TOPOLOGY)
		STR_CASE(MCS)
		STR_CASE(BMC)
		STR_CASE(BUILD_METADATA)
		STR_CASE(KEYVALUE_METADATA)
		STR_CASE(USER_METADATA)
		STR_CASE(DNA_CERTIFICATE)
		STR_CASE(PDI)
		STR_CASE(BITSTREAM_PARTIAL_PDI)
		STR_CASE(PARTITION_METADATA)
		STR_CASE(EMULATION_DATA)
		STR_CASE(SYSTEM_METADATA)
		STR_CASE(SOFT_KERNEL)
		STR_CASE(ASK_FLASH)


		default:
		{
			pString = (char*) "UNKNOWN";
			break;
		}
	}

	return pString;
}




static char* IPTypeToString(enum IP_TYPE ipType)
{
	char* pString;

	switch (ipType)
	{
		STR_CASE(IP_MB)
		STR_CASE(IP_KERNEL)
		STR_CASE(IP_DNASC)
		STR_CASE(IP_DDR4_CONTROLLER)
		STR_CASE(IP_MEM_DDR4)
		STR_CASE(IP_MEM_HBM)

		default:
		{
			pString = (char*) "UNKNOWN";
			break;
		}
	}

	return pString;
}



static char* MemTypeToString(enum MEM_TYPE memType)
{
	char* pString;

	switch (memType)
	{
		STR_CASE(MEM_DDR3)
		STR_CASE(MEM_DDR4)
		STR_CASE(MEM_DRAM)
		STR_CASE(MEM_STREAMING)
		STR_CASE(MEM_PREALLOCATED_GLOB)
		STR_CASE(MEM_ARE)	//Aurora
		STR_CASE(MEM_HBM)
		STR_CASE(MEM_BRAM)
		STR_CASE(MEM_URAM)
		STR_CASE(MEM_STREAMING_CONNECTION)


		default:
		{
			pString = (char*) "UNKNOWN";
			break;
		}
	}

	return pString;
}



static char* XCLBINModeToString(enum XCLBIN_MODE mode)
{
	char* pString;

	switch (mode)
	{
		STR_CASE(XCLBIN_FLAT)
        STR_CASE(XCLBIN_PR)
        STR_CASE(XCLBIN_TANDEM_STAGE2)
        STR_CASE(XCLBIN_TANDEM_STAGE2_WITH_PR)
        STR_CASE(XCLBIN_HW_EMU)
        STR_CASE(XCLBIN_SW_EMU)


		default:
		{
			pString = (char*)"UNKNOWN";
			break;
		}
	}

	return pString;
}




static char* ClockTypeToString(enum CLOCK_TYPE clockType)
{
	char* pString;

	switch (clockType)
	{
		STR_CASE(CT_UNUSED)
		STR_CASE(CT_DATA)
		STR_CASE(CT_KERNEL)
		STR_CASE(CT_SYSTEM) 
		

		default:
		{
			pString = (char*)"UNKNOWN";
			break;
		}
	}

	return pString;
}








void PrintXCLBINHeaderTableHeader(Shell* pShell)
{
	pShell->printf("+-%.24s-+-%.64s-+\n", LINE_STRING, LINE_STRING);
	pShell->printf("| %-24s | %-64s |\n", "Field", "Value");
	pShell->printf("+-%.24s-+-%.64s-+\n", LINE_STRING, LINE_STRING);
}




void PrintXCLBINHeaderTableRow(Shell* pShell, const char* fieldName, char* fieldValue)
{
	pShell->printf("| %-24s | %-64s |\n", fieldName, fieldValue);
}




void PrintXCLBINHeaderTableFooter(Shell* pShell)
{
	pShell->printf("+-%.24s-+-%.64s-+\n", LINE_STRING, LINE_STRING);
}













void PrintXCLBINHeaderInfo(Shell* pShell, struct axlf* pAXLF)
{
	struct axlf_header* pAXLFTopLevelHeader = nullptr;
	static const uint32_t BUFFER_SIZE = 256;
	char formatBuffer[BUFFER_SIZE + 1];

	pAXLFTopLevelHeader = &(pAXLF->m_header);


	PrintXCLBINHeaderTableHeader(pShell);




	snprintf(formatBuffer, BUFFER_SIZE, "%u.%u.%u", pAXLFTopLevelHeader->m_versionMajor, pAXLFTopLevelHeader->m_versionMinor, pAXLFTopLevelHeader->m_versionPatch);
	PrintXCLBINHeaderTableRow(pShell, "Version", formatBuffer);

	PrintXCLBINHeaderTableRow(pShell, "Mode", XCLBINModeToString((enum XCLBIN_MODE)pAXLFTopLevelHeader->m_mode));


	PrintXCLBINHeaderTableRow(pShell, "Platform VBNV", (char*)pAXLFTopLevelHeader->m_platformVBNV);



	uuid_unparse_lower(pAXLFTopLevelHeader->uuid, formatBuffer);
	PrintXCLBINHeaderTableRow(pShell, "XCLBIN UUID", formatBuffer);


	uuid_unparse_lower(pAXLFTopLevelHeader->rom_uuid, formatBuffer);
	PrintXCLBINHeaderTableRow(pShell, "Feature ROM UUID", formatBuffer);



	snprintf(formatBuffer, BUFFER_SIZE, "%" PRIu64, pAXLFTopLevelHeader->m_timeStamp);
	PrintXCLBINHeaderTableRow(pShell, "XCLBIN Timestamp", formatBuffer);

	snprintf(formatBuffer, BUFFER_SIZE, "%" PRIu64, pAXLFTopLevelHeader->m_featureRomTimeStamp);
	PrintXCLBINHeaderTableRow(pShell, "Feature ROM Timestamp", formatBuffer);


	PrintXCLBINHeaderTableFooter(pShell);


}








void PrintSectionInfoTableHeader(Shell* pShell)
{
	pShell->printf("+-%.5s-+-%.20s-+-%.16s-+-%.18s-+-%.18s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
	pShell->printf("| %-5s | %-20s | %-16s | %-18s | %-18s |\n", "Index", "Section Kind", "Name", "Offset", "Size");
	pShell->printf("+-%.5s-+-%.20s-+-%.16s-+-%.18s-+-%.18s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
}




void PrintSectionInfoTableElement(Shell* pShell, uint32_t rowIndex, struct axlf_section_header* pSectionHeader)
{
	pShell->printf("| %-5u | %-20s | %-16s | 0x%016" PRIx64 " | 0x%016" PRIx64 " |\n", rowIndex,
																					   AXLFSectionKindToString((enum axlf_section_kind) pSectionHeader->m_sectionKind),
																					   pSectionHeader->m_sectionName,
																					   pSectionHeader->m_sectionOffset,
																					   pSectionHeader->m_sectionSize);
}




void PrintSectionInfoTableFooter(Shell* pShell)
{
	pShell->printf("+-%.5s-+-%.20s-+-%.16s-+-%.18s-+-%.18s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
}







 void PrintXCLBINSectionInfo(Shell* pShell, struct axlf* pAXLF)
{
	struct axlf_header* pAXLFTopLevelHeader = nullptr;
	struct axlf_section_header* pSectionHeader = nullptr;

	pAXLFTopLevelHeader = &(pAXLF->m_header);

	
	PrintSectionInfoTableHeader(pShell);


	for (uint32_t i = 0; i < pAXLFTopLevelHeader->m_numSections; i++)
	{
		pSectionHeader = &(pAXLF->m_sections[i]);

		PrintSectionInfoTableElement(pShell, i, pSectionHeader);

	}

	PrintSectionInfoTableFooter(pShell);
}















void PrintIPLayoutTableHeader(Shell* pShell)
{
	pShell->printf("+-%.5s-+-%.20s-+-%.18s-+-%.64s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
	pShell->printf("| %-5s | %-20s | %-18s | %-64s |\n", "Index", "IP Type", "Base Address", "Name");
	pShell->printf("+-%.5s-+-%.20s-+-%.18s-+-%.64s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
}




void PrintIPLayoutTableElement(Shell* pShell, uint32_t rowIndex, struct ip_data* pIPData)
{
	pShell->printf("| %-5u | %-20s | 0x%016" PRIx64 " | %-64s |\n", rowIndex,
																	IPTypeToString((enum IP_TYPE)pIPData->m_type),
																	pIPData->m_base_address,
																	pIPData->m_name);
}




void PrintIPLayoutTableFooter(Shell* pShell)
{
	pShell->printf("+-%.5s-+-%.20s-+-%.18s-+-%.64s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
}







void PrintXCLBINIPLayout(Shell* pShell, struct axlf* pAXLF)
{
	struct axlf_section_header* pSectionHeader = nullptr;
	struct ip_layout* pIPLayoutSection;
	struct ip_data* pIPData;

	// First retrieve the section header for the IP_LAYOUT section.
	pSectionHeader = (struct axlf_section_header*) xclbin::get_axlf_section(pAXLF, axlf_section_kind::IP_LAYOUT);


	// The section header gives us the OFFSET to the actual section within the overall XCLBIN.
	// So to get a pointer to the actual section we are interested in, simply add the offset to
	// the base address of our XCLBIN buffer...
	pIPLayoutSection = (struct ip_layout*)((uint8_t*)pAXLF + pSectionHeader->m_sectionOffset);


	PrintIPLayoutTableHeader(pShell);
	
	for (int i = 0; i < pIPLayoutSection->m_count; i++)
	{
		pIPData = &(pIPLayoutSection->m_ip_data[i]);

		PrintIPLayoutTableElement(pShell, i, pIPData);
		
	}

	PrintIPLayoutTableFooter(pShell);
	
}















void PrintMemTopologyTableHeader(Shell* pShell)
{
	pShell->printf("+-%.5s-+-%.16s-+-%.24s-+-%.4s-+-%.18s-+-%.18s-+-%.18s-+-%.18s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
	pShell->printf("| %-5s | %-16s | %-24s | %-4s | %-18s | %-18s | %-18s | %-18s |\n", "Index", "Name", "Memory Type", "Used", "Base Address", "Size (KB)", "Route ID", "Flow ID");
	pShell->printf("+-%.5s-+-%.16s-+-%.24s-+-%.4s-+-%.18s-+-%.18s-+-%.18s-+-%.18s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
}




void PrintMemTopologyTableElement(Shell* pShell, uint32_t rowIndex, struct mem_data* pMemData)
{
	if ((pMemData->m_type == MEM_STREAMING) || (pMemData->m_type == MEM_STREAMING_CONNECTION))
	{
		 pShell->printf("| %-5u | %-16s | %-24s | %-04u | %-18s | %-18s | %-18" PRIu64 " | %-18" PRIu64 " |\n", rowIndex, pMemData->m_tag, MemTypeToString((enum MEM_TYPE)pMemData->m_type), pMemData->m_used, "-", "-", pMemData->route_id, pMemData->flow_id);
	}
	else
	{
		 pShell->printf("| %-5u | %-16s | %-24s | %-04u | 0x%016" PRIx64 " | 0x%016" PRIx64 " | %-18s | %-18s |\n", rowIndex, pMemData->m_tag, MemTypeToString((enum MEM_TYPE)pMemData->m_type), pMemData->m_used, pMemData->m_base_address, pMemData->m_size, "-", "-");
	}
}
 



void PrintMemTopologyTableFooter(Shell* pShell)
{
	pShell->printf("+-%.5s-+-%.16s-+-%.24s-+-%.4s-+-%.18s-+-%.18s-+-%.18s-+-%.18s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
}




void PrintXCLBINMemTopology(Shell* pShell, struct axlf* pAXLF)
{
	struct axlf_section_header* pSectionHeader = nullptr;
	struct mem_topology* pMemTopologySection;
	struct mem_data* pMemData;

	// First retrieve the section header for the IP_LAYOUT section.
	pSectionHeader = (struct axlf_section_header*) xclbin::get_axlf_section(pAXLF, axlf_section_kind::MEM_TOPOLOGY);


	// The section header gives us the OFFSET to the actual section within the overall XCLBIN.
	// So to get a pointer to the actual section we are interested in, simply add the offset to
	// the base address of our XCLBIN buffer...
	pMemTopologySection = (struct mem_topology*)((uint8_t*)pAXLF + pSectionHeader->m_sectionOffset);



	PrintMemTopologyTableHeader(pShell);


	for (int i = 0; i < pMemTopologySection->m_count; i++)
	{
		pMemData = &(pMemTopologySection->m_mem_data[i]);

		PrintMemTopologyTableElement(pShell, i, pMemData);
	}


	PrintMemTopologyTableFooter(pShell);
}














void PrintConnectivityTableHeader(Shell* pShell)
{
	pShell->printf("+-%.5s-+-%.18s-+-%.18s-+-%.18s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
	pShell->printf("| %-5s | %-18s | %-18s | %-18s |\n", "Index", "IP Layout (Kernel)", "Kernel Arg Index", "Connectivity Index");
	pShell->printf("+-%.5s-+-%.18s-+-%.18s-+-%.18s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
}



void PrintConnectivityTableElement(Shell* pShell, uint32_t rowIndex, struct connection* pConnection)
{
	pShell->printf("| %-5u | %-18u | %-18u | %-18u |\n", rowIndex, pConnection->m_ip_layout_index, pConnection->arg_index, pConnection->mem_data_index);
}



void PrintConnectivityTableFooter(Shell* pShell)
{
	pShell->printf("+-%.5s-+-%.18s-+-%.18s-+-%.18s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
}






void PrintXCLBINConnectivity(Shell* pShell, struct axlf* pAXLF)
{
	struct axlf_section_header* pSectionHeader = nullptr;
	struct connectivity* pConnectivitySection;
	struct connection* pConnection;

	// First retrieve the section header for the CONNECTIVITY section.
	pSectionHeader = (struct axlf_section_header*) xclbin::get_axlf_section(pAXLF, axlf_section_kind::CONNECTIVITY);


	// The section header gives us the OFFSET to the actual section within the overall XCLBIN.
	// So to get a pointer to the actual section we are interested in, simply add the offset to
	// the base address of our XCLBIN buffer...
	pConnectivitySection = (struct connectivity*)((uint8_t*)pAXLF + pSectionHeader->m_sectionOffset);


	PrintConnectivityTableHeader(pShell);

	for (int i = 0; i < pConnectivitySection->m_count; i++)
	{
		pConnection = &(pConnectivitySection->m_connection[i]);

		PrintConnectivityTableElement(pShell, i, pConnection);
	}

	PrintConnectivityTableFooter(pShell);

}








void PrintClockFrequencyTopologyTableHeader(Shell* pShell)
{
	pShell->printf("+-%.5s-+-%.10s-+-%.80s-+-%.15s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
	pShell->printf("| %-5s | %-10s | %-80s | %-15s |\n", "Index", "Clock Type", "Name", "Frequency (MHz)");
	pShell->printf("+-%.5s-+-%.10s-+-%.80s-+-%.15s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
}


void PrintClockFrequencyTopologyTableElement(Shell* pShell, uint32_t rowIndex, struct clock_freq* pClockFreq)
{
	pShell->printf("| %-5u | %-10s | %-80s | %-15u |\n", rowIndex,
														 ClockTypeToString((enum CLOCK_TYPE)pClockFreq->m_type),
														 pClockFreq->m_name,
														 pClockFreq->m_freq_Mhz);
}

void PrintClockFrequencyTopologyTableFooter(Shell* pShell)
{
	pShell->printf("+-%.5s-+-%.10s-+-%.80s-+-%.15s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
}





void PrintXCLBINClockFrequency(Shell* pShell, struct axlf* pAXLF)
{
	struct axlf_section_header* pSectionHeader = nullptr;
	struct clock_freq_topology* pClockFreqTopology;
	struct clock_freq* pClockFreq;

	// First retrieve the section header for the CONNECTIVITY section.
	pSectionHeader = (struct axlf_section_header*) xclbin::get_axlf_section(pAXLF, axlf_section_kind::CLOCK_FREQ_TOPOLOGY);


	// The section header gives us the OFFSET to the actual section within the overall XCLBIN.
	// So to get a pointer to the actual section we are interested in, simply add the offset to
	// the base address of our XCLBIN buffer...
	pClockFreqTopology = (struct clock_freq_topology*)((uint8_t*)pAXLF + pSectionHeader->m_sectionOffset);


	PrintClockFrequencyTopologyTableHeader(pShell);

	for (int i = 0; i < pClockFreqTopology->m_count; i++)
	{
		pClockFreq = &(pClockFreqTopology->m_clock_freq[i]);

		PrintClockFrequencyTopologyTableElement(pShell, i, pClockFreq);
	}

	PrintClockFrequencyTopologyTableFooter(pShell);

}









void PrintKernelArgsTableHeader(Shell* pShell)
{
	pShell->printf("+-%.25s-+-%.32s-+-%.50s-+-%.10s-+-%.10s-+-%.10s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
	pShell->printf("| %-25s | %-32s | %-50s | %-10s | %-10s | %-10s |\n", "Kernel Name", "Arg Name", "Arg Type", "Offset", "Size", "Id");
	pShell->printf("+-%.25s-+-%.32s-+-%.50s-+-%.10s-+-%.10s-+-%.10s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
}


void PrintKernelArgsTableElement(Shell* pShell, uint32_t rowIndex, string kernelName, string argName, string argType, string argOffset, string argSize, string argId)
{
	if (rowIndex == 0) //only want to print kernel name on first row...
	{
		 pShell->printf("| %-25s | %-32s | %-50s | %-10s | %-10s | %-10s |\n", kernelName.c_str(), argName.c_str(), argType.c_str(), argOffset.c_str(), argSize.c_str(), argId.c_str());
	}
	else
	{
		 pShell->printf("| %-25s | %-32s | %-50s | %-10s | %-10s | %-10s |\n", "", argName.c_str(), argType.c_str(), argOffset.c_str(), argSize.c_str(), argId.c_str());
	}
}




void PrintKernelArgsTableFooter(Shell* pShell)
{
	pShell->printf("+-%.25s-+-%.32s-+-%.50s-+-%.10s-+-%.10s-+-%.10s-+\n", LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING, LINE_STRING);
}





void PrintXCLBINKernelArgs(Shell* pShell, struct axlf* pAXLF)
{
	struct axlf_section_header* pSectionHeader = nullptr;
	char* pBuildMetaDataSection;
	uint64_t sectionSize;
	

	// First retrieve the section header for the CONNECTIVITY section.
	pSectionHeader = (struct axlf_section_header*) xclbin::get_axlf_section(pAXLF, axlf_section_kind::BUILD_METADATA);


	// The section header gives us the OFFSET to the actual section within the overall XCLBIN.
	// So to get a pointer to the actual section we are interested in, simply add the offset to
	// the base address of our XCLBIN buffer...
	pBuildMetaDataSection = (char*)((uint8_t*)pAXLF + pSectionHeader->m_sectionOffset);
	sectionSize = pSectionHeader->m_sectionSize;



	// The BUILD_METADATA section is actually a big JSON hierarchy

	boost::property_tree::ptree ptSection;

	boost::iostreams::stream<boost::iostreams::array_source> stream(pBuildMetaDataSection, sectionSize);
	boost::property_tree::read_json(stream, ptSection);


	boost::property_tree::ptree& ptXclBin = ptSection.get_child("build_metadata.xclbin");


	BOOST_FOREACH(boost::property_tree::ptree::value_type& userRegionPair, ptXclBin.get_child("user_regions")) //potentially multiple user_regions...
	{

		boost::property_tree::ptree& ptUserRegion = userRegionPair.second;



		BOOST_FOREACH(boost::property_tree::ptree::value_type& kernelPair, ptUserRegion.get_child("kernels"))
		{
			boost::property_tree::ptree& ptKernel = kernelPair.second;


			
			PrintKernelArgsTableHeader(pShell);


			string kernelName = ptKernel.get<string>("name");
			int numArgs = 0;

			BOOST_FOREACH(boost::property_tree::ptree::value_type& argPair, ptKernel.get_child("arguments"))
			{
				boost::property_tree::ptree& ptArgument = argPair.second;

				string argName		= ptArgument.get<string>("name");
				string argType		= ptArgument.get<string>("type");
				string argOffset	= ptArgument.get<string>("offset");
				string argSize		= ptArgument.get<string>("size");
				string argId		= ptArgument.get<string>("id");


				PrintKernelArgsTableElement(pShell, numArgs, kernelName, argName, argType, argOffset, argSize, argId);
	
				numArgs++;
			}

			PrintKernelArgsTableFooter(pShell);
			
			pShell->printf("\n");
		}
	}
	

}



