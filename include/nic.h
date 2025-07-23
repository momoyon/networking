#ifndef _NIC_H_
#define _NIC_H_

#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

typedef struct Entity Entity;
typedef struct Switch Switch;
typedef struct Nic Nic;
typedef enum Ipv4_class Ipv4_class;
typedef enum Ipv4_type Ipv4_type;

#define IPV4_FMT "%d.%d.%d.%d"
#define IPV4_ARG(ipv4) ipv4[0], ipv4[1], ipv4[2], ipv4[3]
#define SUBNET_MASK_FMT IPV4_FMT
#define SUBNET_MASK_ARG(sb) IPV4_ARG(sb)
#define MAC_FMT "%02X:%02X:%02X:%02X:%02X:%02X"
#define MAC_ARG(arg) arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]

enum Ipv4_class {
	IPV4_CLASS_A,
	IPV4_CLASS_B,
	IPV4_CLASS_C,
	IPV4_CLASS_D,
	IPV4_CLASS_E,
	IPV4_CLASS_UNKNOWN,
	IPV4_CLASS_COUNT,
};

void flip_bytes4(uint8 *bytes, uint8 *bytes_flipped);

const char *ipv4_class_as_str(const Ipv4_class ic);
Ipv4_class ipv4_class_from_str(const char *str);
Ipv4_class determine_ipv4_class(uint8 *ipv4);
const char *ipv4_class_additional_info(const Ipv4_class ic);

enum Ipv4_type {
	IPV4_TYPE_PRIVATE,
	IPV4_TYPE_PUBLIC,
	IPV4_TYPE_RESERVED,
	IPV4_TYPE_LOOPBACK,
	IPV4_TYPE_LINK_LOCAL,
	IPV4_TYPE_DOCUMENTATION,
	IPV4_TYPE_INTERNET,
	IPV4_TYPE_LIMITED_BROADCAST,
	IPV4_TYPE_COUNT,
};
#define IPV4_TYPE_LOCAL IPV4_TYPE_PRIVATE

const char *ipv4_type_as_str(const Ipv4_type it);
Ipv4_type ipv4_type_from_str(const char *str);
Ipv4_type determine_ipv4_type(uint8 ipv4[4]);

bool is_ipv4_in_range(uint8 *ipv4, uint8 *min, uint8 *max);

struct Nic {
    uint8 ipv4_address[4];
    uint8 subnet_mask[4];
    uint8 mac_address[6];

	int id; // NOTE: This is the same ID in the entity. Just temp solution to save the Switch's ports' NIC ids.
    Entity *connected_entity;

	int nic_entity_id;
	bool drawing_connection;
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

