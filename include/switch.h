#ifndef _SWITCH_H_
#define _SWITCH_H_

#include <predecls.h>

#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

typedef struct {
	int vlan;
	Nic *nic;
} Port;

typedef struct Switch Switch;
struct Switch {
	Port fa[1][4];
};

void make_switch(Switch *switch_out, Arena *arena, size_t nic_count);

#endif // _SWITCH_H_
