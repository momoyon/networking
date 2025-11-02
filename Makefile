CC=gcc
CFLAGS=-Wall -Wextra -I./include -I./raylib-5.0_win64_mingw-w64/include/ -Wswitch-enum -Werror=switch-enum -Wno-char-subscripts -Wno-sign-compare -Wno-type-limits
LDFLAGS=-L./raylib-5.0_win64_mingw-w64/lib
LIBS=-lraylib -lgdi32 -lwinmm

networking_icon.o: resources/gfx/icon.ico networking.rc
	windres networking.rc -O coff networking_icon.o

networking: networking_icon.o src/*.c mac_address_list.c
	$(CC) $(CFLAGS) -O2 -o $@ networking_icon.o src/*.c $(LDFLAGS) $(LIBS)

debug: networking_icon.o src/*.c mac_address_list.c
	$(CC) $(CFLAGS) -ggdb -DDEBUG=1 -o networking-debug networking_icon.o src/*.c $(LDFLAGS) $(LIBS)

mac_address_list.c: generate_mac_addr_list
	generate_mac_addr_list

generate_mac_addr_list: generate_mac_addr_list.c
	$(CC) -o $@ $<

test_ipv4: test_ipv4.c
	$(CC) $(CFLAGS) -ggdb -DDEBUG -I./include -o $@ $< src/nic.c

embeded: tools\embed.c
	$(CC) $(CFLAGS) -ggdb -I./include -o $@ $<



LATEST_TAG := $(shell git describe --tags --abbrev=0 2>/dev/null || git tag --sort=-creatordate | head -n1)
zip: resources/ networking
	zip -r -9 -q "networking.$(LATEST_TAG).zip" networking.exe resources/

all: networking debug test_ipv4

clean:
	rm *.exe
