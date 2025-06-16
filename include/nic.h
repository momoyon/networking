#ifndef _NIC_H_
#define _NIC_H_

#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

typedef struct Entity Entity;
typedef struct Switch Switch;

struct Nic {
    uint8 ipv4_address[4];
    uint8 subnet_mask[4];
    uint8 mac_address[6];

    Entity *nic_entity; // Other nic connected to

	Entity *switch_entity; // Switch this network interface belongs to.
};

void make_nic(struct Nic *nic, Arena *arena);

typedef struct {
    uint8 addr[6];
} Mac_address;

typedef struct {
    Mac_address *items;
    size_t count;
    size_t capacity;
} Mac_addresses;

extern Mac_addresses assigned_mac_addresses;

// TODO: Move this to commonlib.h
int randomi(int from, int to);

void get_unique_mac_address(uint8 *mac_address);

#endif // _NIC_H_

