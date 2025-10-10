#include <switch.h>
#include <raylib.h>
#include <common.h>

const char *switch_commands[] = {
    [SW_CMD_ID_EXIT]   = "exit",
    [SW_CMD_ID_LOGOUT] = "logout",
    [SW_CMD_ID_ENABLE] = "enable",
};

size_t switch_commands_count = ARRAY_LEN(switch_commands);

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
        add_line_to_console_simple(console, "BOOTING SWITCH...", GREEN);
        add_line_to_console_simple(console, "[", GREEN);
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
        add_line_to_console_simple(console, arena_alloc_str(*(switchh->tmp_arena), "Switch Model: %s", switch_model_as_str(switchh->model)), GREEN);
        add_line_to_console_simple(console, "", GREEN);
        add_line_to_console_simple(console, arena_alloc_str(*(switchh->tmp_arena), "Momo software, Version %s, RELEASE SOFTWARE", switchh->version), GREEN);
        add_line_to_console_simple(console, "Copyright (c) 2025 by Momoyon", GREEN);
        add_line_to_console_simple(console, "", GREEN);
        switchh->booted = true;
    }
}

// TODO: Sometimes the cmd_str (or smth) is jumbled mess.
bool parse_switch_console_cmd(Switch *switchh, String_view_array cmd_args) {
    bool res = true;
    Console *console = &switchh->console;
    if (cmd_args.count <= 0) return true;
    String_view cmd = cmd_args.items[0];

    char *cmd_str = sv_to_cstr(cmd);
    Ids matched_command_ids = match_command(cmd_str, switch_commands, switch_commands_count);

    if (matched_command_ids.count == 0) {
        char *err_msg = arena_alloc_str(*(switchh->tmp_arena), "%s is not a valid command!", cmd_str);
        add_line_to_console_simple(console, err_msg, RED);
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
            case SW_CMD_ID_COUNT:
            default: ASSERT(false, "UNREACHABLE!");
        }
    } else { 
        add_line_to_console_simple(console, arena_alloc_str(*(switchh->tmp_arena), "%% Ambigiuous command: \"%s\"", cmd_str), YELLOW);
    }

    free(cmd_str);
    return res;
}

// TODO: arena_alloc_str() doesn't work correctly now
void switch_change_mode(Switch *switchh, Switch_console_mode new_mode) {
    switchh->mode = new_mode;
    switch (switchh->mode) {
        case SW_CNSL_MODE_USER: {
            if (switchh->console.prefix) arena_dealloc(switchh->str_arena, (void*)switchh->console.prefix);
            switchh->console.prefix = arena_alloc_str(*(switchh->str_arena), "%s>", switchh->hostname ? switchh->hostname : "Switch");
        } break;
        case SW_CNSL_MODE_ENABLED: {
            if (switchh->console.prefix) arena_dealloc(switchh->str_arena, (void*)switchh->console.prefix);
            switchh->console.prefix = arena_alloc_str(*(switchh->str_arena), "%s#", switchh->hostname ? switchh->hostname : "Switch");
        } break;
        case SW_CNSL_MODE_CONFIG: {
            if (switchh->console.prefix) arena_dealloc(switchh->str_arena, (void*)switchh->console.prefix);
            switchh->console.prefix = arena_alloc_str(*(switchh->str_arena), "%s(config)#", switchh->hostname ? switchh->hostname : "Switch");
        } break;
        case SW_CNSL_MODE_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
}
