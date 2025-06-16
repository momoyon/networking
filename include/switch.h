#ifndef _SWITCH_H_
#define _SWITCH_H_

#include <predecls.h>

#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

typedef struct {
	Nic **items;
	size_t count;
	size_t capacity;
} Nic_ptrs; // Arr

typedef struct Switch Switch;
struct Switch {
	Nic_ptrs nic_ptrs;
};

void make_switch(Switch *switch_out, Arena *arena, size_t nic_count);
void free_switch(Switch *s);

#endif // _SWITCH_H_
