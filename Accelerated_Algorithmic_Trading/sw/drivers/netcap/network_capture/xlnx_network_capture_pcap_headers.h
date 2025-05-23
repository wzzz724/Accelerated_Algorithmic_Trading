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


#ifndef XLNX_NETWORK_CAPTURE_PCAP_HEADERS_H
#define XLNX_NETWORK_CAPTURE_PCAP_HEADERS_H

#include <stdint.h>

/* These headers are taken from https://gitlab.com/wireshark/wireshark/-/wikis/Development/LibpcapFileFormat */

typedef struct pcap_hdr_s
{
    uint32_t magic_number;   /* magic number */
    uint16_t version_major;  /* major version number */
    uint16_t version_minor;  /* minor version number */
    int32_t  thiszone;       /* GMT to local correction */
    uint32_t sigfigs;        /* accuracy of timestamps */
    uint32_t snaplen;        /* max length of captured packets, in octets */
    uint32_t network;        /* data link type */

} pcap_hdr_t;





typedef struct pcaprec_hdr_s
{
    uint32_t ts_sec;         /* timestamp seconds */
    uint32_t ts_nsec;        /* timestamp nanoseconds */
    uint32_t incl_len;       /* number of octets of packet saved in file */
    uint32_t orig_len;       /* actual length of packet */

} pcaprec_hdr_t;



#define LINKTYPE_ETHERNET   1
#define LINKTYPE_RAW        101
#define LINKTYPE_IPV4       228
#define LINKTYPE_IPV6       229




#endif
