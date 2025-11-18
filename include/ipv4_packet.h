#ifndef IPV4_PACKET_H_
#define IPV4_PACKET_H_


typedef struct Ipv4_packet Ipv4_packet;

#define IPV4_PACKET_FLAG_R  2
#define IPV4_PACKET_FLAG_DF 1
#define IPV4_PACKET_FLAG_MF 0

// Full list of IP Protocols: https://en.wikipedia.org/wiki/List_of_IP_protocol_numbers
#define IPV4_PROTOCOL_ICMP 1 // Internet Control Message Protocol
#define IPV4_PROTOCOL_IGMP 2 // Internet Group Management Protocol
#define IPV4_PROTOCOL_TCP 6  // Transmission Control Protocol
#define IPV4_PROTOCOL_UDP 17 // User Datagram Protocol
#define IPV4_PROTOCOL_ENCAP 41 // IPv6 Encapsulation
#define IPV4_PROTOCOL_OSPF 89 // Open Shortest Path First
#define IPV4_PROTOCOL_SCTP 132 // Stream Control Transmission Protocol

// https://en.wikipedia.org/wiki/IPv4#Packet_structure
struct {
	uint version : 4;
	uint IHL : 4; // Internet Header Length number of 32-bit words in the header
	uint DSCP : 6; // Differentiated Services Code Point
	uint ECN : 2; // Explicit Congestion Notification
	uint16 total_length; // Total packet size in bytes (Header + Data)
	uint16 id;
	uint flags : 3; // Flags above
	uint frag_offset : 13; // units of 8 bytes
	uint8 ttl; // in seconds
	uint8 protocol; // One of the protocols above
	uint16 header_checksum; // See https://en.wikipedia.org/wiki/IPv4#Packet_structure For how to compute checksum
	uint32 src_addr; // Source IPV4 Address
	uint32 dst_addr; // Destination IPV4 Address
	uint options : 320; // Options 0 ~ 320 bits
};


#endif IPV4_PACKET_H_
