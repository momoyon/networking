#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0')

int main(int argc, char **argv) {
	const char *program = argv[0];
	argv++;
	argc--;

	int max_count = 1000;
	if (argc > 0) {
		char *endptr = NULL;
		int64_t n = strtol(argv[0], &endptr, 10);
		if (endptr == argv[0]) {
			printf("ERROR: Failed to convert `%s` to integer!", argv[0]);
			return 1;
		}
		max_count = n;
	}

	FILE *f = fopen("mac_address_list.c", "w");

	uint64_t addr = 2571259346944; // This number has the 2nd bit set in the 6th byte
	size_t count = 0;

	fprintf(f, "uint8 mac_addrs[] = {\n", max_count);
	do {
		uint8_t oct1 = (addr >> (8 * 0)) & 0xFF;
		uint8_t oct2 = (addr >> (8 * 1)) & 0xFF;
		uint8_t oct3 = (addr >> (8 * 2)) & 0xFF;
		uint8_t oct4 = (addr >> (8 * 3)) & 0xFF;
		uint8_t oct5 = (addr >> (8 * 4)) & 0xFF;
		uint8_t oct6 = (addr >> (8 * 5)) & 0xFF;

		int la = ((oct6 >> 1) & 1);
		if (la) {
			// printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(oct6));
			// printf(" %zu: %02X:%02X:%02X:%02X:%02X:%02X (%lu)\n", count, oct6, oct5, oct4, oct3, oct2, oct1, addr);
			fprintf(f, "	0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X,\n", oct6, oct5, oct4, oct3, oct2, oct1);
			count++;
		}
		addr++;
	} while (count < max_count);
	fprintf(f, "};\n");

	fclose(f);
	return 0;
}
