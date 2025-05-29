#ifndef _NETWORK_DEVICE_H_
#define _NETWORK_DEVICE_H_

#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

typedef struct {
    uint8 ipv4_address[4];
    uint8 mac_address[6];
} Network_device;

#endif // _NETWORK_DEVICE_H_

