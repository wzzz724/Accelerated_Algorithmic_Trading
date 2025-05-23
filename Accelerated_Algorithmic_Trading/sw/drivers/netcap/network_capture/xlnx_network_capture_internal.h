#ifndef XLNX_NETWORK_CAPTURE_INTERNAL_H
#define XLNX_NETWORK_CAPTURE_INTERNAL_H

#include <cstdint>


#define XLNX_NETWORK_CAPTURE_INITIALISED_MAGIC_NUMBER   (0x71540326)


//Each chunk contains both PACKET_DATA and PACKET_METADATA.
//The PACKET_DATA starts at the beginning of the chunk  (offset 0x0000 - 0x07BF) (1984 bytes)
//The PACKET_METADATA starts at a fixed offset          (offset 0x07C0 - 0x07CB) (12 bytes)

#define XLNX_NETWORK_CAPTURE_METADATA_OFFSET            (0x07C0)



typedef struct _PacketMetadata
{
    uint32_t packetLengthInBytes;
    uint32_t packetNum;
    uint32_t timestampLower; //in kernel clock cycles
    uint32_t timestampUpper;

}PacketMetadata;



//The tail pointer is actually an INDEX into the card RAM.  
//It refers to the "next free" location where data will be written.
//However HW writes data to card RAM in 64-byte (512-bit) units,
//and this pointer increments by 1 each time a 64-byte unit is written.
//Therefore to get the byte offset, we must multiply the tail pointer value by 64
#define XLNX_NETWORK_CAPTURE_TAIL_POINTER_MULTIPLIER    (64)






#endif

