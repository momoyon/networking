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

uint8 *flip_bytes4(uint8 *bytes, uint8 *bytes_flipped) {
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
	log_info("	-max_count <max_count:int>				- Specify the maximum number of ips to check. (Increments from 0.0.0.0)");
	log_info("	-range <min:ip> <max:ip>				- Specify the range of ips to check.");
	log_info("	-help                    				- Prints this help message.");
}

int main(int argc, char **argv) {
	uint ipv4 = 0;

	const char *program = shift_args(argv, argc);

	uint min = 0;
	uint max = UINT_MAX;
	if (argc > 0) {
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
		} else if (strcmp(arg, "-range") == 0) {
			if (argc < 2) {
				log_error("%s needs two argument!", arg);
				return 1;
			}
			const char *min_str = shift_args(argv, argc);
			const char *max_str = shift_args(argv, argc);
			int min_ip = parse_ipv4(SV(min_str));
			int max_ip = parse_ipv4(SV(max_str));

			min = min_ip;
			max = max_ip;

			uint8 *ipv4_min = (uint8 *)&min;
			uint8 *ipv4_max = (uint8 *)&max;

		} else if (strcmp(arg, "-help") == 0) {
			usage(program);
			return 0;
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

		log_debug(IPV4_FMT" -> %s %s", IPV4_ARG(ipv4_), ipv4_class_as_str(determine_ipv4_class(ipv4_)), ipv4_type_as_str(determine_ipv4_type(ipv4_)));

	}

	return 0;
}
