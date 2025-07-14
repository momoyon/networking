#ifndef _ETHERNET_FRAME_H_
#define _ETHERNET_FRAME_H_

typedef struct Ethernet_frame Ethernet_frame;

typedef enum Ether_type Ether_type;

// Complete type table can be found at: https://en.wikipedia.org/wiki/EtherType#Values
#define ETHER_TYPE_IPV4 0x0800
#define ETHER_TYPE_ARP  0x0806
#define ETHER_TYPE_IPV6 0x86DD

// Ethernet II (DIX Ethernet) format taken from:https://en.wikipedia.org/wiki/Ethernet_frame#Ethernet_II
struct Ethernet_frame {
	uint8 dst[6]; // Destination Mac-Address
	uint8 src[6]; // Source Mac-Address
	uint16 ether_type_or_length; // if > 0x05DC Type of ethernet frame else length of payload
	uint8 *payload;
	uint32 crc;
};

#endif // _ETHERNET_FRAME_H_
