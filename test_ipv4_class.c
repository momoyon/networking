#include <nic.h>
#define COMMONLIB_REMOVE_PREFIX
#define COMMONLIB_IMPLEMENTATION
#include <commonlib.h>

// Crash on failure
int str_to_int(const char *str) {
	char *endptr = NULL;
	int converted = strtol(str, &endptr, 10);
	if (endptr == str) {
		log_error("Failed to convert '%s' to int!", str);
		exit(1);
	}
	return converted;
}

static void log_valid_ipv4_class_and_type(void) {
	log_info("	<class:[A,B,C,D,E,UNKNOWN]> <type:private,public,reserved,loopback,link_local,documentation,internet,limited_broadcast>");
}

void validate_str(const char *str, const char **array, size_t array_len, const char *type_name, void (*error_func)(void)) {
	bool valid = false;
	for (int i = 0; i < array_len; ++i) {
		const char *elm = array[i];
		if (strcmp(elm, str) == 0) {
			return;
		}
	};

	if (!valid) {
		log_error("`%s` is not a valid %s!", str, type_name);
		if (error_func) error_func();
		exit(1);
	}

}

void check_ipv4_class(const char *class) {
	const char *valid_classes[] = { "A", "B", "C", "D", "E", "UNKNOWN" };
	validate_str(class, valid_classes, ARRAY_LEN(valid_classes), "ipv4 class", log_valid_ipv4_class_and_type);
}

void check_ipv4_type(const char *type) {
	const char *valid_types[] = { "private", "public", "reserved", "loopback", "link_local", "documentation", "internet", "limited_broadcast", };
	validate_str(type, valid_types, ARRAY_LEN(valid_types), "ipv4 type", log_valid_ipv4_class_and_type);
}

uint parse_ipv4(String_view sv) {
	sv_ltrim(&sv);
	String_view ipv4_oct1_sv = sv_lpop_until_char(&sv, '.');
	if (ipv4_oct1_sv.count == 0) {
		log_error("Please pass a valid ipv4! <uint8.uint8.uint8.uint8>");
		exit(1);
	}
	sv_lremove(&sv, 1); // Remove .
	String_view ipv4_oct2_sv = sv_lpop_until_char(&sv, '.');
	if (ipv4_oct2_sv.count == 0) {
		log_error("Please pass a valid ipv4! <uint8.uint8.uint8.uint8>");
		exit(1);
	}
	sv_lremove(&sv, 1); // Remove .
	String_view ipv4_oct3_sv = sv_lpop_until_char(&sv, '.');
	if (ipv4_oct3_sv.count == 0) {
		log_error("Please pass a valid ipv4! <uint8.uint8.uint8.uint8>");
		exit(1);
	}
	sv_lremove(&sv, 1); // Remove .
	String_view ipv4_oct4_sv = sv_lpop_until_char(&sv, ' ');
	if (ipv4_oct4_sv.count == 0) {
		log_error("Please pass a valid ipv4! <uint8.uint8.uint8.uint8>");
		exit(1);
	}
	sv_lremove(&sv, 1); // Remove SPACE

	uint8 ipv4[4] = {0};
	int   ipv4_counts[4] = { -1, -1, -1, -1 };
	ipv4[0] = sv_to_uint(ipv4_oct1_sv, &ipv4_counts[0], 10);
	if (ipv4_counts[0] <= 0) {
		log_debug("Failed to convert ipv4 oct1`"SV_FMT"` to int!", SV_ARG(ipv4_oct1_sv));
		exit(1);
	}
	ipv4[1] = sv_to_uint(ipv4_oct2_sv, &ipv4_counts[1], 10);
	if (ipv4_counts[1] <= 0) {
		log_debug("Failed to convert ipv4 oct2`"SV_FMT"` to int!", SV_ARG(ipv4_oct2_sv));
		exit(1);
	}
	ipv4[2] = sv_to_uint(ipv4_oct3_sv, &ipv4_counts[2], 10);
	if (ipv4_counts[2] <= 0) {
		log_debug("Failed to convert ipv4 oct3`"SV_FMT"` to int!", SV_ARG(ipv4_oct3_sv));
		exit(1);
	}
	ipv4[3] = sv_to_uint(ipv4_oct4_sv, &ipv4_counts[3], 10);
	if (ipv4_counts[3] <= 0) {
		log_debug("Failed to convert ipv4 oct4`"SV_FMT"` to int!", SV_ARG(ipv4_oct4_sv));
		exit(1);
	}

	flip_bytes4(ipv4, ipv4);
	uint res = *((uint *)&ipv4);

	return res;
}

void usage(const char *program) {
	log_info("Usage: %s [subcommand] [sbcmd_arg(s)]", program);
	log_info("");
	log_info("Subcommands: ");
	log_info("	-help                    				- Prints this help message.");
	log_info("	-ip <ip:ipv4>							- Specify a single ipv4 address to check.");
	log_info("	-max_count <max_count:int>				- Specify the maximum number of ips to check. (Increments from 0.0.0.0)");
	log_info("	-range <min:ipv4> <max:ipv4>			- Specify the range of ips to check.");
	log_info("	-expect_class <class>     				- Expect the ip to be this class. SEE BELOW FOR FORMAT");
	log_info("	-expect_type <type>     				- Expect the ip to be this type. SEE BELOW FOR FORMAT");
	log_valid_ipv4_class_and_type();
	log_info("	-nolog     				                - Don't log excessively.");
}

int main(int argc, char **argv) {
	uint ipv4 = 0;

	const char *program = shift_args(argv, argc);

	int expected_class = -1;
	int expected_type  = -1;

	uint min = 0;
	uint max = UINT_MAX;

	bool log = true;

	while (argc > 0) {
		const char *arg = shift_args(argv, argc);

		if (strcmp(arg, "-max_count") == 0) {
			if (argc <= 0) {
				log_error("%s needs one argument!", arg);
				return 1;
			}
			char *endptr = NULL;
			const char *provided_max_ipv4 = shift_args(argv, argc);
			int max_ipv4_converted = strtol(provided_max_ipv4, &endptr, 10);
			if (endptr == provided_max_ipv4) {
				log_error("Failed to convert '%s' to int!", provided_max_ipv4);
				return 1;
			}

			max = max_ipv4_converted;
		} else if (strcmp(arg, "-ip") == 0) {
			if (argc <= 0) {
				log_error("%s needs one argument!", arg);
				return 1;
			}
			const char *ip_str = shift_args(argv, argc);
			uint ip = parse_ipv4(SV(ip_str));

			min = ip;
			max = ip;

		} else if (strcmp(arg, "-range") == 0) {
			if (argc < 2) {
				log_error("%s needs two argument!", arg);
				return 1;
			}
			const char *min_str = shift_args(argv, argc);
			const char *max_str = shift_args(argv, argc);
			uint min_ip = parse_ipv4(SV(min_str));
			uint max_ip = parse_ipv4(SV(max_str));

			min = min_ip;
			max = max_ip;
		} else if (strcmp(arg, "-help") == 0) {
			usage(program);
			return 0;
		} else if (strcmp(arg, "-expect_class") == 0) {
			if (argc < 1) {
				log_error("%s needs one argument!", arg);
				return 1;
			}
			const char *class = shift_args(argv, argc);

			check_ipv4_class(class);

			Ipv4_class c = ipv4_class_from_str(class);
			ASSERT(c != -1, "THIS SHOULDN'T HAPPEN BECAUSE WE CHECK IN ABOVE!");

			expected_class = c;
		} else if (strcmp(arg, "-expect_type") == 0) {
			if (argc < 1) {
				log_error("%s needs one argument!", arg);
				return 1;
			}
			const char *type = shift_args(argv, argc);

			check_ipv4_type(type);

			Ipv4_type c = ipv4_type_from_str(type);
			ASSERT(c != -1, "THIS SHOULDN'T HAPPEN BECAUSE WE CHECK IN ABOVE!");

			expected_type = c;
		} else if (strcmp(arg, "-nolog") == 0) {
			log = false;
		} else {
			log_error("Invalid subcommand `%s`", arg);
			return 1;
		}
	}

	for (ipv4 = min; ipv4 <= max; ++ipv4) {
		uint8 *ipv4__ = (uint8 *)&ipv4;
		uint8 ipv4_[] = {
			ipv4__[3],
			ipv4__[2],
			ipv4__[1],
			ipv4__[0],
		};

		Ipv4_class class = determine_ipv4_class(ipv4_);
		Ipv4_type  type  = determine_ipv4_type(ipv4_);
		if (expected_class >= 0 && expected_class != class) {
			log_error(IPV4_FMT" -> Expected `%s` as Ipv4 Class but got `%s`", IPV4_ARG(ipv4_), ipv4_class_as_str(expected_class), ipv4_class_as_str(class));
			exit(1);
		}
		if (expected_type >= 0 && expected_type != type) {
			log_error(IPV4_FMT" -> Expected `%s` as Ipv4 Type but got `%s`", IPV4_ARG(ipv4_), ipv4_type_as_str(expected_type), ipv4_type_as_str(type));
			exit(1);
		}

		if (log)
			log_debug(IPV4_FMT" -> %s %s", IPV4_ARG(ipv4_), ipv4_class_as_str(class), ipv4_type_as_str(type));

		if (ipv4 == UINT_MAX) {
			break;
		}
	}
	log_info("DONE");

	return 0;
}
