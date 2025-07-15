#include <nic.h>
#include <config.h>
#include <ethernet_frame.h>

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
		default: {
			 log_debug("GOT: %d", ic);
			 ASSERT(false, "UNREACHABLE!");
		};
	}
	return "BRUH YOU ARE NOT ABLE TO SEE THIS!!!!!!!!!!!!!!!!";
}

Ipv4_class ipv4_class_from_str(const char *str) {
	if (strcmp(str, "A") == 0) {
		return IPV4_CLASS_A;
	} else if (strcmp(str, "B") == 0) {
		return IPV4_CLASS_B;
	} else if (strcmp(str, "C") == 0) {
		return IPV4_CLASS_C;
	} else if (strcmp(str, "D") == 0) {
		return IPV4_CLASS_D;
	} else if (strcmp(str, "E") == 0) {
		return IPV4_CLASS_E;
	} else if (strcmp(str, "UNKNOWN") == 0) {
		return IPV4_CLASS_UNKNOWN;
	}
	return -1;
}

Ipv4_class determine_ipv4_class(uint8 *ipv4) {
	ASSERT(ipv4 != NULL, "Bro please pass a valid ipv4...");
	if (ipv4[0] >= 0   && ipv4[0] <= 127) return IPV4_CLASS_A;
	if (ipv4[0] >= 128 && ipv4[0] <= 191) return IPV4_CLASS_B;
	if (ipv4[0] >= 192 && ipv4[0] <= 223) return IPV4_CLASS_C;
	if (ipv4[0] >= 224 && ipv4[0] <= 239) return IPV4_CLASS_D;
	if (ipv4[0] >= 240 && ipv4[0] <= 255) return IPV4_CLASS_E;
	return IPV4_CLASS_UNKNOWN;
}

const char *ipv4_class_additional_info(const Ipv4_class ic) {
	switch (ic) {
		case IPV4_CLASS_A: return "";
		case IPV4_CLASS_B: return "";
		case IPV4_CLASS_C: return "";
		case IPV4_CLASS_D: return "Multicast";
		case IPV4_CLASS_E: return "Ephimeral (Future Use)";
		case IPV4_CLASS_UNKNOWN: return "Unknown / Reserved";
		case IPV4_CLASS_COUNT:
		default: ASSERT(false, "UNREACHABLE!");
	}
	return "BRUH YOU ARE NOT ABLE TO SEE THIS!!!!!!!!!!!!!!!!";
}

const char *ipv4_type_as_str(const Ipv4_type it) {
	switch (it) {
		case IPV4_TYPE_PRIVATE: return "Private";
		case IPV4_TYPE_PUBLIC: return "Public";
		case IPV4_TYPE_RESERVED: return "Reserved";
		case IPV4_TYPE_LOOPBACK: return "Loopback";
		case IPV4_TYPE_COUNT:
		default: ASSERT(false, "UNREACHABLE!");
	}
	return "UNVIEWABLE!";
}

Ipv4_type ipv4_type_from_str(const char *str) {
	if (strcmp(str, "private") == 0) {
		return IPV4_TYPE_PRIVATE;
	} else if (strcmp(str, "public") == 0) {
		return IPV4_TYPE_PUBLIC;
	} else if (strcmp(str, "public") == 0) {
		return IPV4_TYPE_PUBLIC;
	} else if (strcmp(str, "reserved") == 0) {
		return IPV4_TYPE_RESERVED;
	} else if (strcmp(str, "loopback") == 0) {
		return IPV4_TYPE_LOOPBACK;
	}
	return -1;
}

Ipv4_type determine_ipv4_type(uint8 ipv4[4]) {
	Ipv4_class class = determine_ipv4_class(ipv4);
	switch (class) {
		case IPV4_CLASS_A: {
			if (ipv4[0] == 10) {
				return IPV4_TYPE_PRIVATE;
			} else {
				return IPV4_TYPE_PUBLIC;
			}
	    } break;
		case IPV4_CLASS_B: {
			log_debug("IPV4_CLASS_B is_ipv4_private is UNIMPLEMENTED!");
	    } break;
		case IPV4_CLASS_C: {
			log_debug("IPV4_CLASS_C is_ipv4_private is UNIMPLEMENTED!");
	    } break;
		case IPV4_CLASS_D: {
			log_debug("IPV4_CLASS_D is_ipv4_private is UNIMPLEMENTED!");
	    } break;
		case IPV4_CLASS_E: {
			log_debug("IPV4_CLASS_E is_ipv4_private is UNIMPLEMENTED!");
	    } break;
		case IPV4_CLASS_UNKNOWN: {
			log_debug("IPV4_CLASS_UNKNOWN is_ipv4_private is UNIMPLEMENTED!");
	    } break;
		case IPV4_CLASS_COUNT:
		default: ASSERT(false, "UNREACHABLE!");
	}
	return false;
}

bool is_ipv4_public(uint8 ipv4[4]) {
	(void)ipv4;
	ASSERT(false, "UNIMPLEMENTED!");
}
