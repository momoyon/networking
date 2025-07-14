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

const char *ipv4_class_as_str(const Ipv4_class ic) {
	switch (ic) {
		case IPV4_CLASS_A: return "A";
		case IPV4_CLASS_B: return "B";
		case IPV4_CLASS_C: return "C";
		case IPV4_CLASS_D: return "D";
		case IPV4_CLASS_E: return "E";
		case IPV4_CLASS_UNKNOWN: return "U";
		case IPV4_CLASS_COUNT:
		default: ASSERT(false, "UNREACHABLE!");
	}
	return "BRUH YOU ARE NOT ABLE TO SEE THIS!!!!!!!!!!!!!!!!";
}

Ipv4_class determine_ipv4_class(uint8 *ipv4) {
	ASSERT(ipv4 != NULL, "Bro please pass a valid ipv4...");
	if ((int)ipv4[0] >= 1 && (int)ipv4[0] <= 127) return IPV4_CLASS_A;
	if ((int)ipv4[0] >= 128 && ((int)ipv4[0] <= 191 && (int)ipv4[1] <= 255)) return IPV4_CLASS_B;
	if ((int)ipv4[0] >= 192 && ((int)ipv4[0] <= 223 && (int)ipv4[1] <= 255 && (int)ipv4[2] <= 255)) return IPV4_CLASS_C;
	if ((int)ipv4[0] >= 224 && ((int)ipv4[0] <= 239 && (int)ipv4[1] <= 255 && (int)ipv4[2] <= 255 && (int)ipv4[3] <= 255)) return IPV4_CLASS_D;
	if ((int)ipv4[0] >= 240 && ((int)ipv4[0] <= 255 && (int)ipv4[1] <= 255 && (int)ipv4[2] <= 255 && (int)ipv4[3] <= 255)) return IPV4_CLASS_D;
	return IPV4_CLASS_UNKNOWN;
}
