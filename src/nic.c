#include <nic.h>
#include <config.h>
#include <ethernet_frame.h>

Mac_addresses free_mac_addresses = {0};

#include "../mac_address_list.c"

static int fmb = 0;

void flip_bytes4(uint8 *bytes, uint8 *bytes_flipped) {
	uint8 res[4] = {
		bytes[3],
		bytes[2],
		bytes[1],
		bytes[0],
	};

	bytes_flipped[0] = res[0];
	bytes_flipped[1] = res[1];
	bytes_flipped[2] = res[2];
	bytes_flipped[3] = res[3];
}


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
		case IPV4_TYPE_LINK_LOCAL: return "Link Local";
		case IPV4_TYPE_DOCUMENTATION: return "Documentation";
		case IPV4_TYPE_INTERNET: return "Internet";
		case IPV4_TYPE_LIMITED_BROADCAST: return "Limited Broadcast";
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
	} else if (strcmp(str, "link_local") == 0) {
		return IPV4_TYPE_LINK_LOCAL;
	} else if (strcmp(str, "documentation") == 0) {
		return IPV4_TYPE_DOCUMENTATION;
	} else if (strcmp(str, "internet") == 0) {
		return IPV4_TYPE_INTERNET;
	} else if (strcmp(str, "limited_broadcast") == 0) {
		return IPV4_TYPE_LIMITED_BROADCAST;
	}
	return -1;
}

Ipv4_type determine_ipv4_type(uint8 ipv4[4]) {
	Ipv4_class class = determine_ipv4_class(ipv4);
	switch (class) {
		case IPV4_CLASS_A: {
			if (is_ipv4_in_range(ipv4, (uint8[4]){0, 0, 0, 0}, (uint8[4]){0, 255, 255, 255}) ||
			    is_ipv4_in_range(ipv4, (uint8[4]){10, 0, 0, 0}, (uint8[4]){10, 255, 255, 255}) ||
				is_ipv4_in_range(ipv4, (uint8[4]){100, 64, 0, 0}, (uint8[4]){100, 127, 255, 255})) {
				return IPV4_TYPE_PRIVATE;
			} else if (is_ipv4_in_range(ipv4, (uint8[4]){127, 0, 0, 0}, (uint8[4]){127, 255, 255, 255})) {
				return IPV4_TYPE_LOOPBACK;
			} else {
				return IPV4_TYPE_PUBLIC;
			}
	    } break;
		case IPV4_CLASS_B: {
			if (is_ipv4_in_range(ipv4, (uint8[4]){169, 254, 0, 0}, (uint8[4]){169, 254, 255, 255})) {
				return IPV4_TYPE_LINK_LOCAL;
			} else if (is_ipv4_in_range(ipv4, (uint8[4]){172, 16, 0, 0}, (uint8[4]){172, 31, 255, 255})) {
				return IPV4_TYPE_PRIVATE;
			} else {
				return IPV4_TYPE_PUBLIC;
			}
	    } break;
		case IPV4_CLASS_C: {
			if (is_ipv4_in_range(ipv4, (uint8[4]){192, 0, 0, 0}, (uint8[4]){192, 0, 0, 255}) ||
				is_ipv4_in_range(ipv4, (uint8[4]){192, 168, 0, 0}, (uint8[4]){192, 168, 255, 255}) ||
				is_ipv4_in_range(ipv4, (uint8[4]){198, 18, 0, 0}, (uint8[4]){198, 19, 255, 255}))  {
				return IPV4_TYPE_PRIVATE;
			} else if (is_ipv4_in_range(ipv4, (uint8[4]){ 192, 0, 2, 0}, (uint8[4]){ 192, 0, 2, 255}) ||
					   is_ipv4_in_range(ipv4, (uint8[4]){ 198, 51, 100, 0}, (uint8[4]){ 198, 51, 100, 255}) ||
					   is_ipv4_in_range(ipv4, (uint8[4]){ 203, 0, 113, 0}, (uint8[4]){ 203, 0, 113, 255})) {
				return IPV4_TYPE_DOCUMENTATION;
			} else if (is_ipv4_in_range(ipv4, (uint8[4]){ 192, 88, 99, 0}, (uint8[4]){ 192, 88, 99, 255})) {
				return IPV4_TYPE_INTERNET;
			} else {
				return IPV4_TYPE_PUBLIC;
			}
	    } break;
		case IPV4_CLASS_D: {
			return IPV4_TYPE_INTERNET; // MULTICAST
	    } break;
		case IPV4_CLASS_E: {
			if (is_ipv4_in_range(ipv4, (uint8[4]){ 240, 0, 0, 0}, (uint8[4]){ 255, 255, 255, 254 })) {
				return IPV4_TYPE_RESERVED;
		    }
			return IPV4_TYPE_LIMITED_BROADCAST;
	    } break;
		case IPV4_CLASS_UNKNOWN: {
			log_debug("IPV4_CLASS_UNKNOWN determine_ipv4_type is UNIMPLEMENTED!");
	    } break;
		case IPV4_CLASS_COUNT:
		default: ASSERT(false, "UNREACHABLE!");
	}
	return false;
}

bool is_ipv4_in_range(uint8 *ipv4, uint8 *min, uint8 *max) {
	flip_bytes4(min, min);
	uint32 min_uint = *((uint *)min);
	flip_bytes4(min, min);

	flip_bytes4(max, max);
	uint32 max_uint = *((uint *)max);
	flip_bytes4(max, max);


	flip_bytes4(ipv4, ipv4);
	uint32 ipv4_uint = *((uint *)ipv4);
	flip_bytes4(ipv4, ipv4);

	// log_debug("CHECKING %u >= %u && %u <= %u", ipv4_uint, min_uint, ipv4_uint, max_uint);

	return ipv4_uint >= min_uint && ipv4_uint <= max_uint;
}
