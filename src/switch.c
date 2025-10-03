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

void boot_switch(Switch *switchh) {
    Console *console = &switchh->console;

    add_line_to_console_simple(console, "BOOTING SWITCH...", GREEN);

    switchh->booted = true;
}
