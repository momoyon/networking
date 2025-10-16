#include <switch.h>
#include <raylib.h>
#include <common.h>

/// USER EXEC
 Switch_console_cmd switch_user_commands[] = {
    { .name = "exit",       .desc = "Exit from the EXEC",                               .id = ITEM(EXIT) },
    { .name = "logout",     .desc = "Exit from the EXEC",                               .id = ITEM(LOGOUT) },
    { .name = "enable",     .desc = "Turn on priviledged commands",                     .id = ITEM(ENABLE) },
    { .name = "ping",       .desc = "Send echo messages",                               .id = ITEM(PING) },
    { .name = "connect",    .desc = "Open a terminal connection",                       .id = ITEM(CONNECT) },
    { .name = "disable",    .desc = "Turn off priveleged commands",                     .id = ITEM(DISABLE) },
    { .name = "disconnect", .desc = "Disconnect an existing network connection",        .id = ITEM(DISCONNECT) },
    { .name = "resume",     .desc = "Resume an existing network connection",            .id = ITEM(RESUME) },
    { .name = "show",       .desc = "Show running system information",                  .id = ITEM(SHOW) },
    { .name = "ssh",        .desc = "Open a secure shell client connection",            .id = ITEM(SSH) },
    { .name = "telnet",     .desc = "Open a telnet connection",                         .id = ITEM(TELNET) },
    { .name = "terminal",   .desc = "Set terminal line parameters",                    .id = ITEM(TERMINAL) },
    { .name = "traceroute", .desc = "Trace route to destination",                       .id = ITEM(TRACEROUTE) },
};
size_t switch_user_commands_count = ARRAY_LEN(switch_user_commands);
///--------------------------------------------------
/// ENABLED | PRIVEGED
const char *switch_enabled_commands[] = {
    [ITEM(CLEAR)]     = "clear",
    [ITEM(CLOCK)]     = "clock",
    [ITEM(CONFIGURE)] = "configure",
    [ITEM(CONNECT)]   = "connect",
    [ITEM(COPY)]      = "copy",
    [ITEM(DEBUG)]     = "debug",
    [ITEM(DELETE)]    = "delete",
    [ITEM(DIR)]       = "dir",
    [ITEM(DISABLE)]   = "disable",
    [ITEM(DISCONNECT)]= "disconnect",
    [ITEM(ENABLE)]    = "enable",
    [ITEM(ERASE)]     = "erase",
    [ITEM(EXIT)]      = "exit",
    [ITEM(LOGOUT)]    = "logout",
    [ITEM(MORE)]      = "more",
    [ITEM(NO)]        = "no",
    [ITEM(PING)]      = "ping",
    [ITEM(RELOAD)]    = "reload",
    [ITEM(RESUME)]    = "resume",
    [ITEM(SETUP)]     = "setup",
    [ITEM(SHOW)]      = "show",
    [ITEM(SSH)]       = "ssh",
    [ITEM(TELNET)]    = "telnet",
    [ITEM(TERMINAL)]  = "terminal",
    [ITEM(TRACEROUTE)]= "traceroute",
    [ITEM(UNDEBUG)]   = "undebug",
    [ITEM(WRITE)]     = "write",
};

size_t switch_enabled_commands_count = ARRAY_LEN(switch_enabled_commands);

const char *switch_enabled_command_descriptions[] = {
    [ITEM(CLEAR)]     = "Reset functions",
    [ITEM(CLOCK)]     = "Manage the system clock",
    [ITEM(CONFIGURE)] = "Enter configuration mode",
    [ITEM(CONNECT)]   = "Open a terminal connection",
    [ITEM(COPY)]      = "Copy from one file to another",
    [ITEM(DEBUG)]     = "Debugging functions (see also 'undebug')",
    [ITEM(DELETE)]    = "Delete a file",
    [ITEM(DIR)]       = "List files on a filesystem",
    [ITEM(DISABLE)]   = "Turn off privileged commands",
    [ITEM(DISCONNECT)]= "Disconnect an existing network connection",
    [ITEM(ENABLE)]    = "Turn on privileged commands",
    [ITEM(ERASE)]     = "Erase a filesystem",
    [ITEM(EXIT)]      = "Exit from the EXEC",
    [ITEM(LOGOUT)]    = "Exit from the EXEC",
    [ITEM(MORE)]      = "Display the contents of a file",
    [ITEM(NO)]        = "Disable debugging informations",
    [ITEM(PING)]      = "Send echo messages",
    [ITEM(RELOAD)]    = "Halt and perform a cold restart",
    [ITEM(RESUME)]    = "Resume an active network connection",
    [ITEM(SETUP)]     = "Run the SETUP command facility",
    [ITEM(SHOW)]      = "Show running system information",
    [ITEM(SSH)]       = "Open a secure shell client connection",
    [ITEM(TELNET)]    = "Open a telnet connection",
    [ITEM(TERMINAL)]  = "Set terminal line parameters",
    [ITEM(TRACEROUTE)]= "Trace route to destination",
    [ITEM(UNDEBUG)]   = "Disable debugging functions (see also 'debug')",
    [ITEM(WRITE)]     = "Write running configuration to memory, network, or terminal",
};
size_t switch_enabled_command_descriptions_count = ARRAY_LEN(switch_enabled_command_descriptions);
///--------------------------------------------------
/// CONFIG
const char *switch_config_commands[] = {
    [SW_CMD_ID_EXIT]    = "Exit from the EXEC",
    [SW_CMD_ID_LOGOUT]  = "Exit from the EXEC",
    [SW_CMD_ID_ENABLE]  = "Turn on priviledged commands",
    [SW_CMD_ID_PING]    = "Send echo messages",
    [SW_CMD_ID_CONNECT] = "Open a terminal connection",
    [SW_CMD_ID_DISABLE] = "Turn off priveleged commands",
};
size_t switch_config_commands_count = ARRAY_LEN(switch_config_commands);

const char *switch_console_arg_type_as_str(const Switch_console_arg_type t) {
    if (0 <= (size_t)t && (size_t)t < switch_console_arg_types_count) {
        return switch_console_arg_types[(size_t)t];
    }
    return "UNSEEABLE";
}

const char *switch_console_arg_types[] = {
    [SW_CNSL_ARG_TYPE_WORD] = "WORD",
    [SW_CNSL_ARG_TYPE_ABCD] = "A.B.C.D",
    [SW_CNSL_ARG_TYPE_ABCDM] = "A.B.C.D/M",
    [SW_CNSL_ARG_TYPE_INVALID] = "INVALID",
};
size_t switch_console_arg_types_count = ARRAY_LEN(switch_console_arg_types);
///--------------------------------------------------

Switch_console_arg_type switch_console_arg_type_from_str(const char *t) {
    // TODO: Implemente str_to_upper() in commonlib.h
    size_t t_len = strlen(t);
    char *t_upper = (char *)malloc(sizeof(char) * t_len + 1);

    for (int i = 0; i < t_len; ++i) {
        t_upper[i] = toupper(t[i]);
    }
    t_upper[t_len] = '\0';

    Switch_console_arg_type res = SW_CNSL_ARG_TYPE_INVALID;

    if (strcmp(t_upper, "WORD") == 0) {
        res = SW_CNSL_ARG_TYPE_WORD;
    } else if (strcmp(t_upper, "A.B.C.D") == 0) {
        res = SW_CNSL_ARG_TYPE_ABCD;
    } else if (strcmp(t_upper, "A.B.C.D/M") == 0) {
        res = SW_CNSL_ARG_TYPE_ABCDM;
    }

    free(t_upper);

    return res;
}

// See https://fs.momoyon.org/share/SW_CMD_ARG_TABLE.md
bool valid_switch_console_arg(const char *cmd, Switch_console_arg_type type) {
    switch (type) {
        case SW_CNSL_ARG_TYPE_WORD: {
            if (*cmd != '\0') return true;
        } break;
        case SW_CNSL_ARG_TYPE_ABCD: {
            String_view sv = SV(cmd);
            uint8 ipv4[4] = {0};
            return parse_n_octet_from_data(4, &sv, ipv4, 4, false);
        } break;
        case SW_CNSL_ARG_TYPE_ABCDM: {
            String_view sv = SV(cmd);
            uint8 ipv4[4] = {0};
            uint8 mask = 0;
            return parse_n_octet_with_mask_from_data(4, &sv, ipv4, 4, &mask);
        } break;
        case SW_CNSL_ARG_TYPE_INVALID:
        case SW_CNSL_ARG_TYPE_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
    return false;
}

const char *switch_model_as_str(const Switch_model sw_m) {
    switch (sw_m) {
        case SW_MODEL_MOMO_SW_2025_A: return "MOMO-SW-2025-A";
        case SW_MODEL_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
    return "YOU ARE CURSED!";
}

const char *switch_console_mode_as_str(const Switch_console_mode m) {
    switch (m) {
        case SW_CNSL_MODE_USER: return "User";
        case SW_CNSL_MODE_ENABLED: return "Enabled";
        case SW_CNSL_MODE_CONFIG: return "config";
        case SW_CNSL_MODE_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }

    return "*PIC OF SCP-096*";
}

void boot_switch(Switch *switchh, float dt) {
    Console *console = &switchh->console;

    if (!switchh->boot_started) {
        add_line_to_console_simple(console, "BOOTING SWITCH...", GREEN, false);
        add_line_to_console_simple(console, "[", GREEN, false);
        switchh->boot_started = true;
    }


    if (switchh->boot_perc < 50) {
        if (on_alarm(&switchh->boot_load_alarm, dt)) {
            int boot_value = rand() % 3;
            switchh->boot_perc += boot_value;
            if (switchh->boot_perc >= 50) switchh->boot_perc = 50;
            for (int i = 0; i < boot_value; ++i) {
                add_character_to_console_line(console, '#', console->lines.count-1);
            }
        }
    } else {
        add_character_to_console_line(console, ']', console->lines.count-1);
        add_line_to_console_simple(console, arena_alloc_str(*(switchh->tmp_arena), "Switch Model: %s", switch_model_as_str(switchh->model)), GREEN, false);
        add_line_to_console_simple(console, "", GREEN, false);
        add_line_to_console_simple(console, arena_alloc_str(*(switchh->tmp_arena), "Momo software, Version %s, RELEASE SOFTWARE", switchh->version), GREEN, false);
        add_line_to_console_simple(console, "Copyright (c) 2025 by Momoyon", GREEN, false);
        add_line_to_console_simple(console, "", GREEN, false);
        switchh->booted = true;
    }
}

bool parse_switch_console_cmd(Switch *switchh, String_array cmd_args) {
    bool res = true;
    Console *console = &switchh->console;
    if (cmd_args.count <= 0) return true;
    const char *cmd = cmd_args.items[0];


    const char **switch_commands = NULL;
    size_t switch_commands_count = 0;

    get_switch_console_commands(switchh, &switch_commands, &switch_commands_count);


    Ids matched_command_ids = match_command(cmd, switch_commands, switch_commands_count);

    if (matched_command_ids.count == 0) {
        char *err_msg = arena_alloc_str(*(switchh->tmp_arena), "%s is not a valid command!", cmd);
        add_line_to_console_simple(console, err_msg, RED, false);
    } else if (matched_command_ids.count == 1) {
        switch (matched_command_ids.items[0]) {
            case SW_CMD_ID_LOGOUT:
            case SW_CMD_ID_EXIT: {
                if (switchh->mode == SW_CNSL_MODE_ENABLED) {
                    switch_change_mode(switchh, SW_CNSL_MODE_USER);
                } else {
                    res = false;
                }
            } break;
            case SW_CMD_ID_ENABLE: {
                switch_change_mode(switchh, SW_CNSL_MODE_ENABLED);
            } break;
            case SW_CMD_ID_PING: {
                log_error_a(*console, "%s", "`ping` is UNIMPLEMENTED!");
            } break;
            case SW_CMD_ID_CONNECT: {
                log_error_a(*console, "%s", "`connect` is UNIMPLEMENTED!");
            } break;
            case SW_CMD_ID_DISABLE: {
                switch_change_mode(switchh, SW_CNSL_MODE_USER);
            } break;
            case SW_CMD_ID_DISCONNECT: {
                log_error_a(*console, "%s", "`disconnect` is UNIMPLEMENTED!");
            } break;
            case SW_CMD_ID_RESUME: {
                log_error_a(*console, "%s", "`resume` is UNIMPLEMENTED!");
            } break;
            case SW_CMD_ID_SHOW: {
                log_error_a(*console, "%s", "`show` is UNIMPLEMENTED!");
            } break;
            case SW_CMD_ID_SSH: {
                log_error_a(*console, "%s", "`ssh` is UNIMPLEMENTED!");
            } break;
            case SW_CMD_ID_TELNET: {
                log_error_a(*console, "%s", "`telnet` is UNIMPLEMENTED!");
            } break;
            case SW_CMD_ID_TERMINAL: {
                log_error_a(*console, "%s", "`terminal` is UNIMPLEMENTED!");
            } break;
            case SW_CMD_ID_TRACEROUTE: {
                log_error_a(*console, "%s", "`traceroute` is UNIMPLEMENTED!");
            } break;
            case SW_CMD_ID_COUNT:
            default: ASSERT(false, "UNREACHABLE!");
        }
    } else { 
        add_line_to_console_simple(console, arena_alloc_str(*(switchh->tmp_arena), "%% Ambigiuous command: \"%s\"", cmd), YELLOW, false);
    }

    return res;
}

void switch_change_mode(Switch *switchh, Switch_console_mode new_mode) {
    switchh->mode = new_mode;
    switchh->console.prefix = switchh->hostname;
    switch (switchh->mode) {
        case SW_CNSL_MODE_USER: {
            switchh->console.prefix_symbol = '>';
        } break;
        case SW_CNSL_MODE_ENABLED: {
            switchh->console.prefix_symbol = '#';
        } break;
        case SW_CNSL_MODE_CONFIG: {
            switchh->console.prefix_symbol = '#';
            // TODO: Have to change prefix
            // switchh->prefix = 
        } break;
        case SW_CNSL_MODE_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
}


void get_switch_console_commands(Switch *switchh, Switch_console_cmd **commands_out, size_t *commands_count_out) {

    switch (switchh->mode) {
        case SW_CNSL_MODE_USER: {
            *commands_out = switch_user_commands;
            *commands_count_out = switch_user_commands_count;
        } break;
        case SW_CNSL_MODE_ENABLED: {
            *commands_out = switch_enabled_commands;
            *commands_count_out = switch_enabled_commands_count;
        } break;
        case SW_CNSL_MODE_CONFIG: {
            *commands_out = switch_config_commands;
            *commands_count_out = switch_config_commands_count;
        } break;
        case SW_CNSL_MODE_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
}

bool get_next_switch_console_command_arg(String_array current_args, Switch_console_arg *next_arg_out) {
    // TODO: Do we care about the mode here?

    if (current_args.count <= 0) return NULL;

    const char *cmd = current_args.items[0];

    Switch_console_args cmd_args = get_args_for_switch_cmd(cmd);


    int next_arg_idx = current_args.count-1; // -1 because the cmd itself is part of the args

    if (next_arg_idx < cmd_args.count) {
        Switch_console_arg next_arg = cmd_args.items[next_arg_idx];

        *next_arg_out = next_arg;
        return true;
    }

    return false;
}

#define MATCH_CMD(with) (strcmp(cmd, with) == 0)

Switch_console_args get_args_for_switch_cmd(const char *cmd) {
    Switch_console_args res = {0};
    // NOTE: redundant ifs just to list out every command
    if MATCH_CMD("exit") {
    } else if MATCH_CMD("logout") {
    } else if MATCH_CMD("enable") {
    } else if MATCH_CMD("ping") {
        Switch_console_arg arg = {
            .name = "WORD",
            .desc = "Ping destination address or hostname",
        };

        darr_append(arg.types, SW_CNSL_ARG_TYPE_WORD);
        darr_append(arg.types, SW_CNSL_ARG_TYPE_ABCD);

        darr_append(res, arg);
    } else if MATCH_CMD("ping") {
        Switch_console_arg arg = {
            .name = "WORD",
            .desc = "IP address or hostname of a remote system",
        };

        darr_append(arg.types, SW_CNSL_ARG_TYPE_WORD);
        darr_append(arg.types, SW_CNSL_ARG_TYPE_ABCD);

        darr_append(res, arg);
    }


    return res;
}
