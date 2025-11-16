#ifndef PORT_H_
#define PORT_H_

#include <nic.h>

typedef struct {
	int vlan;
    Nic *nic;
	int conn_id; // NOTE: Only used when loading switches.
    int module, port;
    int switch_entity_id; // NOTE: Parent switch entity id
} Port;


#endif // PORT_H_
