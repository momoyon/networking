#ifndef _SWITCH_H_
#define _SWITCH_H_

#include <predecls.h>
#include <config.h>
#include <engine.h>

#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

typedef struct Switch_console_cmd Switch_console_cmd;
typedef struct Switch_console_arg Switch_console_arg;
typedef struct Switch_console_args Switch_console_args;
typedef struct Switch_console_arg_types Switch_console_arg_types;
typedef enum Switch_console_arg_type Switch_console_arg_type;

enum Switch_console_arg_type {
    SW_CNSL_ARG_TYPE_WORD,
    SW_CNSL_ARG_TYPE_ABCD,
    SW_CNSL_ARG_TYPE_ABCDM,
    SW_CNSL_ARG_TYPE_INVALID,
    SW_CNSL_ARG_TYPE_COUNT,
};

extern const char *switch_console_arg_types[];
extern size_t switch_console_arg_types_count;

const char *switch_console_arg_type_as_str(const Switch_console_arg_type t);
Switch_console_arg_type switch_console_arg_type_from_str(const char *t);
bool valid_switch_console_arg(const char *cmd, Switch_console_arg_type type);

struct Switch_console_cmd {
    const char *name;
    const char *desc;
    int id;
};

extern Switch_console_cmd switch_user_commands[];
extern size_t switch_user_commands_count;

extern Switch_console_cmd switch_enabled_commands[];
extern size_t switch_enabled_commands_count;

extern Switch_console_cmd switch_config_commands[];
extern size_t switch_config_commands_count;
struct Switch_console_arg_types {
    Switch_console_arg_type *items;
    size_t count;
    size_t capacity;
};

struct Switch_console_arg {
    const char *name;
    const char *desc;
    Switch_console_arg_types types;
};

struct Switch_console_args {
    Switch_console_arg *items;
    size_t count;
    size_t capacity;
};

#define ITEM(name) SW_CMD_ID_##name
#define LIST \
    ITEM(EXIT),\
    ITEM(ENABLE),\
    ITEM(LOGOUT),\
    ITEM(PING),\
    ITEM(CONNECT),\
    ITEM(DISABLE),\
    ITEM(DISCONNECT),\
    ITEM(RESUME),\
    ITEM(SHOW),\
    ITEM(SSH),\
    ITEM(TELNET),\
    ITEM(TERMINAL),\
    ITEM(TRACEROUTE),\
    ITEM(CLEAR),\
    ITEM(CLOCK),\
    ITEM(CONFIGURE),\
    ITEM(COPY),\
    ITEM(DEBUG),\
    ITEM(DELETE),\
    ITEM(DIR),\
    ITEM(ERASE),\
    ITEM(MORE),\
    ITEM(NO),\
    ITEM(RELOAD),\
    ITEM(SETUP),\
    ITEM(UNDEBUG),\
    ITEM(WRITE),\
    ITEM(COUNT)


typedef enum {
    LIST
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
typedef enum Switch_console_config_mode Switch_console_config_mode;

enum Switch_console_mode {
    SW_CNSL_MODE_USER,
    SW_CNSL_MODE_ENABLED,
    SW_CNSL_MODE_CONFIG,
    SW_CNSL_MODE_COUNT,
};

const char *switch_console_mode_as_str(const Switch_console_mode m);

enum Switch_console_config_mode {
    SW_CNSL_CONF_MODE_TERMINAL,
    SW_CNSL_CONF_MODE_COUNT,
};

const char *switch_console_config_mode_as_str(const Switch_console_config_mode m);

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
    Switch_console_config_mode config_mode;
    const char *hostname;

    Arena *tmp_arena;
    Arena *str_arena;
};

void make_switch(Switch_model model, const char *version, Switch *switch_out, Arena *arena, Arena *tmp_arena, Arena *str_arena);
void make_switch_console(Console *console_out, Arena *arena);
void boot_switch(Switch *switchh, float dt);
bool parse_switch_console_cmd(Switch *switchh, String_array cmd_args);

bool get_next_switch_console_command_arg(String_array current_args, Switch_console_arg *next_arg_out);
void get_switch_console_commands(Switch *switchh, Switch_console_cmd **commands_out, size_t *commands_count_out);

void switch_change_mode(Switch *switchh, Switch_console_mode new_mode);

Switch_console_args get_args_for_switch_cmd(const char *cmd);

#endif // _SWITCH_H_
