#ifndef _AP_H_
#define _AP_H_

typedef struct Access_point Access_point;

struct Access_point {
    uint8 mgmt_ipv4[4];
    uint8 mgmt_subnet_mask[4];
};

void make_ap(Access_point *ap_out, Arena *arena);

#endif // _AP_H_
