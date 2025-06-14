#ifndef _SWITCH_H_
#define _SWITCH_H_

#include <predecls.h>

#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

typedef struct {
    Network_interface *eth;
	size_t eth_count;
} Switch;

Switch make_switch(Arena *arena, size_t eth_count);

#endif // _SWITCH_H_
