#ifndef _SWITCH_H_
#define _SWITCH_H_

#include <predecls.h>
#include <config.h>
#include <engine.h>

#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

extern const char *switch_user_commands[];
extern size_t switch_user_commands_count;
extern const char *switch_user_command_descriptions[];
extern size_t switch_user_command_descriptions_count;

extern const char *switch_enabled_commands[];
extern size_t switch_enabled_commands_count;

extern const char *switch_config_commands[];
extern size_t switch_config_commands_count;

typedef enum {
    SW_CMD_ID_EXIT = 0,
    SW_CMD_ID_ENABLE,
    SW_CMD_ID_LOGOUT,
    SW_CMD_ID_COUNT,
} Switch_console_cmd_id;

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

typedef enum Switch_console_mode Switch_console_mode;

enum Switch_console_mode {
    SW_CNSL_MODE_USER,
    SW_CNSL_MODE_ENABLED,
    SW_CNSL_MODE_CONFIG,
    SW_CNSL_MODE_COUNT,
};

const char *switch_console_mode_as_str(const Switch_console_mode m);

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

    // Console stuff
    Switch_console_mode mode;
    const char *hostname;

    Arena *tmp_arena;
    Arena *str_arena;
};

void make_switch(Switch_model model, const char *version, Switch *switch_out, Arena *arena, Arena *tmp_arena, Arena *str_arena);
void make_switch_console(Console *console_out, Arena *arena);
void boot_switch(Switch *switchh, float dt);
bool parse_switch_console_cmd(Switch *switchh, String_array cmd_args);

void get_switch_console_commands(Switch *switchh, const char ***commands_out, size_t *commands_count_out);

void switch_change_mode(Switch *switchh, Switch_console_mode new_mode);

#endif // _SWITCH_H_
