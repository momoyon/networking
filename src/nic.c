#include <nic.h>
#include <config.h>

Mac_addresses free_mac_addresses = {0};

#include "../mac_address_list.c"

static int fmb = 0;

void get_unique_mac_address(uint8 *mac_address) {
	if (free_mac_addresses.count > 0) {
		Mac_address m = {0};
		darr_remove(free_mac_addresses, Mac_address, &m, free_mac_addresses.count-1);

		mac_address[0] = m.addr[0];
		mac_address[1] = m.addr[1];
		mac_address[2] = m.addr[2];
		mac_address[3] = m.addr[3];
		mac_address[4] = m.addr[4];
		mac_address[5] = m.addr[5];
	} else {
		mac_address[0] = mac_addrs[fmb++];
		mac_address[1] = mac_addrs[fmb++];
		mac_address[2] = mac_addrs[fmb++];
		mac_address[3] = mac_addrs[fmb++];
		mac_address[4] = mac_addrs[fmb++];
		mac_address[5] = mac_addrs[fmb++];
	}
}

