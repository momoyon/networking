CC=gcc
CFLAGS=-Wall -Wextra -I./include -I./raylib-5.0_win64_mingw-w64/include/ -Wswitch-enum -Werror=switch-enum -Wno-char-subscripts -Wno-sign-compare -Wno-type-limits
LDFLAGS=-L./raylib-5.0_win64_mingw-w64/lib
LIBS=-lraylib -lgdi32 -lwinmm

networking: src/*.c mac_address_list.c
	$(CC) $(CFLAGS) -O2 -o $@ src/*.c $(LDFLAGS) $(LIBS)

debug: src/*.c mac_address_list.c
	$(CC) $(CFLAGS) -ggdb -DDEBUG=1 -o networking-debug src/*.c $(LDFLAGS) $(LIBS)

mac_address_list.c: generate_mac_addr_list
	generate_mac_addr_list

generate_mac_addr_list: generate_mac_addr_list.c
	$(CC) -o $@ $<

test_ipv4_class: test_ipv4_class.c
	$(CC) $(CFLAGS) -ggdb -DDEBUG -I./include -o $@ $< src/nic.c

all: networking debug test_ipv4_class
