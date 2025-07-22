#ifndef _AP_H_
#define _AP_H_

#include <commonlib.h>

typedef struct Access_point Access_point;
typedef struct Wifi_wave Wifi_wave;
typedef struct Wifi_waves Wifi_waves;

struct Access_point {
    uint8 mgmt_ipv4[4];
    uint8 mgmt_subnet_mask[4];

    Alarm wifi_wave_alarm;
    bool on;
};

struct Wifi_wave {
    Vector2 pos;
    float radius;
    Color color;
    float dead_zone; // radius at which the wave dies out
};

struct Wifi_waves {
    Wifi_wave *items;
    size_t count;
    size_t capacity;
};

void make_ap(Access_point *ap_out, Arena *arena);

#endif // _AP_H_
