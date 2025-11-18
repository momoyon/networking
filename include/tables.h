#ifndef TABLES_H_
#define TABLES_H_

#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

typedef struct {
	uint8 ipv4[4];
} Ipv4;

typedef struct {
	uint8 mac[6];
} MAC;

typedef struct {
	Ipv4 key;
	MAC value;
} Ipv4_to_mac_KV;

typedef struct {
	Ipv4_to_mac_KV *arp_table;
} ARP_Table;

typedef struct {
	int module;
	int port;
} Port_ID;

typedef struct {
	Port_ID key;
	MAC value;
} Port_to_MAC_KV;

typedef struct {
	Port_to_MAC_KV *mac_table;
} MAC_Table;


#endif // TABLES_H_
