#include <switch.h>
#include <raylib.h>

const char *switch_model_as_str(const Switch_model sw_m) {
    switch (sw_m) {
        case SW_MODEL_MOMO_SW_2025_A: return "MOMO-SW-2025-A";
        case SW_MODEL_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
    return "YOU ARE CURSED!";
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
            add_character_to_console_line(console, '#', console->lines.count-1);
            switchh->boot_perc++;
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

void parse_switch_console_cmd(Switch *switchh, String_view_array cmd_args) {
    for (int i = 0; i < cmd_args.count; ++i) {
        log_debug("SWITCH CONSOLE CMD: "SV_FMT, SV_ARG(cmd_args.items[i]));
    }
}
