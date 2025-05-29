CC=gcc
CFLAGS=-Wall -Wextra -I./include -I./raylib-5.0_win64_mingw-w64/include/ -Wswitch-enum -Wno-char-subscripts -Wno-sign-compare
LDFLAGS=-L./raylib-5.0_win64_mingw-w64/lib
LIBS=-lraylib -lgdi32 -lwinmm

networking: main.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

debug: main.c
	$(CC) $(CFLAGS) -DDEBUG=1 -o networking-debug $< $(LDFLAGS) $(LIBS)

all: networking debug
