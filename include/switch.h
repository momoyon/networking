#ifndef _SWITCH_H_
#define _SWITCH_H_

#include <predecls.h>
#include <config.h>
#include <engine.h>

#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

typedef struct Entity Entity;

typedef struct {
	int vlan;
    Entity *conn;
	int conn_id; // NOTE: Only used when loading switches.
} Port;

typedef enum Switch_model Switch_model;

enum Switch_model {
    SW_MODEL_MOMO_SW_2025_A,
    SW_MODEL_COUNT,
};

const char *switch_model_as_str(const Switch_model sw_m);

typedef struct Switch Switch;
struct Switch {
	Console console;
	Port fe[1][4];
    Switch_model model;
    const char *version;

    bool booted;
    int boot_perc;
    bool boot_started;
    Alarm boot_load_alarm;

    Arena *tmp_arena;
};

void make_switch(Switch_model model, const char *version, Switch *switch_out, Arena *arena, Arena *tmp_arena);
void make_switch_console(Console *console_out, Arena *arena);
void boot_switch(Switch *switchh, float dt);

#endif // _SWITCH_H_
