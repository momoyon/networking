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

int main(int argc, char **argv) {
	int ipv4 = 0;

	const char *program = shift_args(argv, argc);

	int max_ipv4 = INT_MAX;
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

			max_ipv4 = max_ipv4_converted;
		} else if (strcmp(arg, "-range") == 0) {
			if (argc < 2) {
				log_error("%s needs two argument!", arg);
				return 1;
			}
		}
	}

	while (ipv4 < max_ipv4) {
		uint8 *ipv4__ = (uint8 *)&ipv4;
		uint8 ipv4_[4] = {
			ipv4__[3],
			ipv4__[2],
			ipv4__[1],
			ipv4__[0],
		};

		log_debug(IPV4_FMT" -> %s %s", IPV4_ARG(ipv4_), ipv4_class_as_str(determine_ipv4_class(ipv4_)), ipv4_type_as_str(determine_ipv4_type(ipv4_)));

		ipv4++;
	}

	return 0;
}
