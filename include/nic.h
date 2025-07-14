#ifndef _NIC_H_
#define _NIC_H_

#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

typedef struct Entity Entity;
typedef struct Switch Switch;
typedef enum Ipv4_class Ipv4_class;

enum Ipv4_class {
	IPV4_CLASS_A,
	IPV4_CLASS_B,
	IPV4_CLASS_C,
	IPV4_CLASS_D,
	IPV4_CLASS_E,
	IPV4_CLASS_UNKNOWN,
	IPV4_CLASS_COUNT,
};

const char *ipv4_class_as_str(const Ipv4_class ic);
Ipv4_class determine_ipv4_class(uint8 *ipv4);

struct Nic {
    uint8 ipv4_address[4];
    uint8 subnet_mask[4];
    uint8 mac_address[6];

	int id; // NOTE: This is the same ID in the entity. Just temp solution to save the Switch's ports' NIC ids.
	// Entity *self_entity; // Self entity.
    Entity *nic_entity; // Other nic connected to
	Entity *switch_entity; // Switch this network interface belongs to.
};

void make_nic(Entity *e, struct Nic *nic, Arena *arena);
bool ipv4_from_input(Entity *e, char *chars_buff, size_t *chars_buff_count, size_t chars_buff_cap);
bool subnet_mask_from_input(Entity *e, char *chars_buff, size_t *chars_buff_count, size_t chars_buff_cap);

typedef struct {
    uint8 addr[6];
} Mac_address;

typedef struct {
    Mac_address *items;
    size_t count;
    size_t capacity;
} Mac_addresses;

extern Mac_addresses free_mac_addresses;
void get_unique_mac_address(uint8 *mac_address);


#endif // _NIC_H_

