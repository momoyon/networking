CC=gcc
CFLAGS=-Wall -Wextra -I./include -I./raylib-5.0_win64_mingw-w64/include/ -Wswitch-enum -Werror=switch-enum -Wno-char-subscripts -Wno-sign-compare -Wno-type-limits
LDFLAGS=-L./raylib-5.0_win64_mingw-w64/lib
LIBS=-lraylib -lgdi32 -lwinmm

networking: src/*.c
	$(CC) $(CFLAGS) -O2 -o $@ src/*.c $(LDFLAGS) $(LIBS)

debug: src/*.c
	$(CC) $(CFLAGS) -ggdb -DDEBUG=1 -o networking-debug src/*.c $(LDFLAGS) $(LIBS)

all: networking debug
