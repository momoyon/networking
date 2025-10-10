#include <common.h>
#include <config.h>
#include <entity.h>
#include <nic.h>
#include <predecls.h>

#define COMMONLIB_REMOVE_PREFIX
#define COMMONLIB_IMPLEMENTATION
#include <commonlib.h>

#define ENGINE_IMPLEMENTATION
#include <engine.h>

#define STB_DS_IMPLEMENTATION
#include <stb_ds.h>

#define log_info_a(console, fmt, ...) log_info_console((console), fmt, __VA_ARGS__); log_info(fmt, __VA_ARGS__)
#define log_warning_a(console, fmt, ...) log_warning_console((console), fmt, __VA_ARGS__); log_warning(fmt, __VA_ARGS__)
#define log_error_a(console, fmt, ...) log_error_console((console), fmt, __VA_ARGS__); log_error(fmt, __VA_ARGS__)

#define FACTOR 105
#define SCREEN_WIDTH (16 * FACTOR)
#define SCREEN_HEIGHT (9 * FACTOR)
#define SCREEN_SCALE 0.5

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)                                      \
        ((byte) & 0x80 ? '1' : '0'), ((byte) & 0x40 ? '1' : '0'), \
        ((byte) & 0x20 ? '1' : '0'), ((byte) & 0x10 ? '1' : '0'), \
        ((byte) & 0x08 ? '1' : '0'), ((byte) & 0x04 ? '1' : '0'), \
        ((byte) & 0x02 ? '1' : '0'), ((byte) & 0x01 ? '1' : '0')

typedef enum {
    MODE_NORMAL,
    MODE_COPY,
    MODE_CHANGE,
    MODE_INTERACT,
    MODE_COUNT,
} Mode;

void change_mode(Mode* last_mode, Mode* current_mode, bool *changed_mode_this_frame, Mode mode_to)
{
    *last_mode = *current_mode;
    *current_mode = mode_to;
    *changed_mode_this_frame = true;
}

#define CHANGE_MODE(mode_to)                               \
    do {                                                   \
        change_mode(&last_mode, &current_mode, &changed_mode_this_frame, (mode_to)); \
    } while (0);

const char* mode_as_str(const Mode m)
{
    switch (m) {
    case MODE_NORMAL:
        return "Normal";
    case MODE_COPY:
        return "Copy";
    case MODE_CHANGE:
        return "Change";
    case MODE_INTERACT:
        return "Interact";
    case MODE_COUNT:
    default:
        ASSERT(false, "UNREACHABLE!");
    }
}

typedef struct Var Var;
typedef struct Vars Vars;
typedef enum Var_type Var_type;

enum Var_type {
    VAR_TYPE_INT,
    VAR_TYPE_FLOAT,
    VAR_TYPE_STRING,
    VAR_TYPE_CHAR,
    VAR_TYPE_BOOL,
    VAR_TYPE_COUNT,
};

const char *var_type_as_str(const Var_type t) {
    switch (t) {
        case VAR_TYPE_INT: return "int";
        case VAR_TYPE_FLOAT: return "float";
        case VAR_TYPE_STRING: return "string";
        case VAR_TYPE_CHAR: return "char";
        case VAR_TYPE_BOOL: return "bool";
        case VAR_TYPE_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
    return "UNSEEABLE!";
}

struct Var {
    Var_type type;

    const char *name;

    void *value_ptr; // the pointer in the code that holds the value
};

struct Vars {
    Var *items;
    size_t count;
    size_t capacity;
};

Vars vars = {0};

#define INT_VAR(_name, varname) (Var) { .type = VAR_TYPE_INT, .name = (#_name), .value_ptr = (void*)(&varname) }
#define FLOAT_VAR(_name, varname) (Var) { .type = VAR_TYPE_FLOAT, .name = (#_name), .value_ptr = (void*)(&varname) }
#define STR_VAR(_name, varname) (Var) { .type = VAR_TYPE_STRING, .name = (#_name), .value_ptr = (void*)(&varname) }
#define CHAR_VAR(_name, varname) (Var) { .type = VAR_TYPE_CHAR, .name = (#_name), .value_ptr = (void*)(&varname) }
#define BOOL_VAR(_name, varname) (Var) { .type = VAR_TYPE_BOOL, .name = (#_name), .value_ptr = (void*)(&varname) }

bool set_int_var(Console *console, Var *var, String_view newvalue);
bool set_float_var(Console *console, Var *var, String_view newvalue);
bool set_str_var(Console *console, Var *var, String_view newvalue);
bool set_char_var(Console *console, Var *var, String_view newvalue);
bool set_bool_var(Console *console, Var *var, String_view newvalue);
bool set_var(Console *console, Var *var, String_view newvalue);
Var *get_var(String_view name);
const char *get_var_value(Var *var);

// Externs from common.h
RenderTexture2D ren_tex;
Arena entity_arena;
Arena temp_arena;
Arena str_arena;
Texture_manager tex_man;
size_t entity_save_version = 2;
Wifi_waves wifi_waves = {0};

typedef enum {
    CHANGE_IPV4,
    CHANGE_SUBNET_MASK,
    CHANGE_COUNT,
} Changing_type;

const char* changing_type_as_str(const Changing_type ch)
{
    switch (ch) {
    case CHANGE_IPV4:
        return "ipv4";
    case CHANGE_SUBNET_MASK:
        return "subnet mask";
    case CHANGE_COUNT:
    default:
        ASSERT(false, "UNREACHABLE!");
    }
    return "NOPE";
}

typedef enum {
    CMD_ID_EXIT = 0,
    CMD_ID_NORMAL,
    CMD_ID_CHANGE,
    CMD_ID_INTERACT,
    CMD_ID_COPY,
    CMD_ID_LOAD,
    CMD_ID_SAVE,
    CMD_ID_LS_CMDS,
    CMD_ID_SET,
    CMD_ID_LS_VARS,
    CMD_ID_GET,
    CMD_ID_COUNT,
} Command_id;

typedef Ids Command_ids;

const char *commands[] = {
    [CMD_ID_EXIT]      = "exit",
    [CMD_ID_NORMAL]    = "normal",
    [CMD_ID_CHANGE]    = "change",
    [CMD_ID_INTERACT]  = "interact",
    [CMD_ID_COPY]      = "copy",
    [CMD_ID_LOAD]      = "load",
    [CMD_ID_SAVE]      = "save",
    [CMD_ID_LS_CMDS]   = "ls_cmds",
    [CMD_ID_SET]       = "set",
    [CMD_ID_LS_VARS]   = "ls_vars",
    [CMD_ID_GET]       = "get",
};
// @TODO: ARRAY_LEN is getting -1 of the actual len
size_t commands_count = ARRAY_LEN(commands);

Ids match_var(const char *var) {
    Ids matched_vars = {0};

    size_t var_len = strlen(var);

    for (int i = 0; i < vars.count; ++i) {
        const char *varname = vars.items[i].name;
        size_t varname_len = strlen(varname);
        if (str_starts_with(varname, var)) {
            if ((var_len > varname_len && var[varname_len] == ' ')
              || var_len <= varname_len) {
                darr_append(matched_vars, i);
            }
        }
    }

    return matched_vars;
}


// int main(int argc, char **argv) {
//     shift_args(argv, argc);
//
//     if (argc < 2) {
//         log_error("provide two args!");
//         return 1;
//     }
//     const char *str = shift_args(argv, argc);
//     const char *starts_with = shift_args(argv, argc);
//
//     log_debug("%s %sstarts with %s", str, str_starts_with(str, starts_with) ? "" : "doesn't ", starts_with);
//
//     return 0;
// }


/// @DEBUG
static int radius = 100;
static bool blue = false;
static float xoffset = 10.f;
///

int main(void)
{
    int width = 0;
    int height = 0;


#if defined(DEBUG)
    bool debug_draw = true;
#else
    bool debug_draw = false;
#endif // defined(DEBUG)

    const char* window_name = "Networking";
    ren_tex = init_window(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_SCALE, window_name,
        &width, &height);
    SetExitKey(0);

    // Font font = GetFontDefault();
    //
    SetTextLineSpacing(1);

    arr_heap_init(entities, ENTITIES_MAX_CAP);

    Mode last_mode = MODE_NORMAL;
    Mode current_mode = MODE_NORMAL;
    bool changed_mode_this_frame = false;

    Entity_kind selected_entity_kind = EK_NIC;
    Entity* hovering_entity = NULL;
    Entity* connecting_from = NULL;
    Entity* connecting_to = NULL;

    Texture2D selected_entity_kind_tex = {0};
    ASSERT(load_texture(&tex_man, entity_texture_path_map[selected_entity_kind], &selected_entity_kind_tex), "THIS SHOULDNT FAIL!");

    Rectangle selection = { 0 };
    Vector2 selection_start = { 0 };
    bool selecting = false;

    entity_arena = arena_make(32 * 1024);
    temp_arena = arena_make(0);
    str_arena  = arena_make(0);

    Camera2D cam = {
        .zoom = 1.0,
        .offset = CLITERAL(Vector2) { width / 2, height / 2 },
    };
    Vector2 mpos_from = { 0 };

    bool is_changing = false;
    Changing_type changing_type = CHANGE_IPV4;

#define chars_buff_cap (1024)
    char chars_buff[chars_buff_cap] = { 0 };
    size_t chars_buff_count = 0;

    Console *active_switch_console = NULL;
    Switch *active_switch = NULL;
    float pad_perc = 0.15f;
    Rectangle active_switch_console_rect = {
        .x = 0,
        .y = -height,
        .width = width - (width*pad_perc*2.0f),
        .height = height - (height*pad_perc*2.0f),
    };

    bool is_in_command = false;
    Console command_hist = {
        .font = GetFontDefault(),
        .prefix = ":",
    };

    // prerun cmd (only supports one line rn)
    // TODO: Support multiple commands on multiple lines
    int prerun_cmd_file_size = -1;
    const char *prerun_cmd = read_file(COMMAND_PRERUN_FILENAME, &prerun_cmd_file_size);
    bool prerun_cmd_ran = false;

    bool quit = false;

    // Add vars
    darr_append(vars, (INT_VAR(radius, radius)));
    darr_append(vars, (FLOAT_VAR(xoffset, xoffset)));
    darr_append(vars, (BOOL_VAR(blue, blue)));

    while (!quit && !WindowShouldClose()) {
        arena_reset(&temp_arena);
        const char* title_str = arena_alloc_str(temp_arena, "%s | %d FPS", window_name, GetFPS());

        SetWindowTitle(title_str);

        changed_mode_this_frame = false;

        BeginDrawing();
        Vector2 m = get_mpos_scaled(SCREEN_SCALE);
        Vector2 m_world = GetScreenToWorld2D(m, cam);

        // Input
        if (IsKeyPressed(KEY_GRAVE)) {
            debug_draw = !debug_draw;
        }


        // Run prerun command
        if (prerun_cmd_file_size > 0 && !prerun_cmd_ran) {

            size_t prerun_cmd_len = strlen(prerun_cmd);

            if (command_hist.lines.count <= 0) {
                Console_line cl = {0};

                darr_append(command_hist.lines, cl);
            }

            memcpy(command_hist.lines.items[0].buff, prerun_cmd, prerun_cmd_len-1);
            command_hist.lines.items[0].count = prerun_cmd_len-1;

            prerun_cmd_ran = true;

            goto exec_command;
        }

        if (is_in_command) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                is_in_command = false;
            }

            if (input_to_console(&command_hist)) {
exec_command:
                char* command_buff = get_current_console_line_buff(&command_hist);
                String_view_array args = get_current_console_args(&command_hist);
                Command_ids matched_commands_ids = match_command(command_buff, commands, commands_count);
                String_view command = SV("");
                if (args.count > 0) {
                    command = args.items[0];
                }
                if (matched_commands_ids.count == 0) {
                    log_error_console(command_hist, "`%s` is not a valid command!", command_buff);
                    is_in_command = true;
                    clear_current_console_line(&command_hist);
                    add_line_to_console(&command_hist, command_buff, strlen(command_buff), WHITE);
                } else if (matched_commands_ids.count == 1) {

                    if (sv_equals(command, SV(commands[matched_commands_ids.items[0]]))) {
                        add_line_to_console(&command_hist, command_buff, strlen(command_buff), WHITE);
                        // Run commands
                        is_in_command = false;
                        switch (matched_commands_ids.items[0]) {
                            case CMD_ID_EXIT: {
                                quit = true;
                            } break;
                            case CMD_ID_NORMAL: {
                                CHANGE_MODE(MODE_NORMAL);
                            } break;
                            case CMD_ID_CHANGE: {
                                CHANGE_MODE(MODE_CHANGE);
                            } break;
                            case CMD_ID_INTERACT: {
                                CHANGE_MODE(MODE_INTERACT);
                            } break;
                            case CMD_ID_COPY: {
                                CHANGE_MODE(MODE_COPY);
                            } break;
                            case CMD_ID_LOAD: {
                                if (args.count-1 <= 0) {
                                    log_error_a(command_hist, "%s", "Please provide the name of the file to load!");
                                    is_in_command = true;
                                    break;
                                }
                                String_view load_path_sv = args.items[1];
                                char *load_path = sv_to_cstr(load_path_sv);
                                if (load_entities(&entities, load_path, &entity_arena, &temp_arena, &str_arena)) {
                                    log_info_a(command_hist, "Successfully loaded entities from `%s`", load_path);
                                } else {
                                    log_error_a(command_hist, "Failed to load entities from `%s`", load_path);
                                    is_in_command = true;
                                }
                                free(load_path);
                            } break;
                            case CMD_ID_SAVE: {
                                if (args.count-1 <= 0) {
                                    log_error_a(command_hist, "%s", "Please provide the name of the file to save!");
                                    is_in_command = true;
                                    break;
                                }
                                String_view save_path_sv = args.items[1];
                                char *save_path = sv_to_cstr(save_path_sv);
                                if (save_entities(&entities, save_path, entity_save_version)) {
                                    log_info_a(command_hist, "Successfully saved entities to `%s`", save_path);
                                } else {
                                    log_error_a(command_hist, "Failed to save entities to `%s`", save_path);
                                    is_in_command = true;
                                }
                                free(save_path);
                            } break;
                            case CMD_ID_LS_CMDS: {
                                log_info_a(command_hist, "%s", "Commands: ");
                                for (size_t i = 0; i < ARRAY_LEN(commands); ++i) {
                                    const char *c = commands[i];
                                    log_info_a(command_hist, " - %s", c);
                                }
                                is_in_command = true;
                                clear_current_console_line(&command_hist);
                            } break;
                            case CMD_ID_SET: {
                                const char *cmd = commands[matched_commands_ids.items[0]];
                                if (args.count-1 < 2) { // NOTE: -1 because the command itself is also counted as an arg
                                    log_error_a(command_hist, "Command `%s` wants 2 argument; but %zu provided!", cmd, args.count-1);
                                } else {
                                    String_view var_name = args.items[1];
                                    String_view var_new_value = args.items[2];

                                    Var *v = get_var(var_name);
                                    if (v != NULL) {
                                        set_var(&command_hist, v, var_new_value);
                                    } else {
                                        char *var_name_str = sv_to_cstr(var_name);
                                        Ids matched_var_ids = match_var(var_name_str);
                                        free(var_name_str);
                                        if (matched_var_ids.count <= 0)
                                            log_error_a(command_hist, "Could not find any variable named `"SV_FMT"`", SV_ARG(var_name));
                                    }
                                }

                                clear_current_console_line(&command_hist);
                                is_in_command = true;
                            } break;
                            case CMD_ID_LS_VARS: {
                                if (vars.count > 0) {
                                    log_info_a(command_hist, "%s", "Vars: ");
                                    for (int i = 0; i < vars.count; ++i) {
                                        Var *v = &vars.items[i];
                                        log_info_a(command_hist, " - %s(%s)", v->name, var_type_as_str(v->type));
                                    }
                                } else {
                                    log_info_a(command_hist, "%s", "No var is defined!");
                                }
                                is_in_command = true;
                                clear_current_console_line(&command_hist);
                            } break;
                            case CMD_ID_GET: {
                                if (args.count-1 <= 0) {
                                    log_error_a(command_hist, "%s", "Please provide the varname to get the value of!");
                                    is_in_command = true;
                                    break;
                                }
                                String_view var_name = args.items[1];

                                Var *v = get_var(var_name);
                                if (v != NULL) {
                                    log_info_a(command_hist, SV_FMT": %s", SV_ARG(var_name), get_var_value(v));
                                } else {
                                    char *var_name_str = sv_to_cstr(var_name);
                                    Ids matched_var_ids = match_var(var_name_str);
                                    free(var_name_str);
                                    if (matched_var_ids.count <= 0) {
                                        log_error_a(command_hist, "Could not find any variable named `"SV_FMT"`", SV_ARG(var_name));
                                    } else {
                                        if (matched_var_ids.count == 1) {
                                            const char *varname = vars.items[matched_var_ids.items[0]].name;
                                            const char *complete_get_varname = arena_alloc_str(temp_arena, "get %s", varname);
                                            size_t varname_len = strlen(complete_get_varname);
                                            memcpy(get_current_console_line_buff(&command_hist), complete_get_varname, varname_len);
                                            command_hist.cursor = varname_len;

                                            is_in_command = true;
                                            break;
                                        } else {
                                            for (size_t i = 0; i < matched_var_ids.count; ++i) {
                                                int idx = matched_var_ids.items[i];
                                                log_debug_console(command_hist, "    - %s", vars.items[idx].name);
                                            }
                                        }
                                    }
                                }
                                is_in_command = true;
                                clear_current_console_line(&command_hist);
                            } break;
                            case CMD_ID_COUNT:
                            default: ASSERT(false, "UNREACHABLE!");
                        }
                    } else {
                        const char *cmd = commands[matched_commands_ids.items[0]];
                        size_t cmd_len = strlen(cmd);
                        memcpy(get_current_console_line_buff(&command_hist), cmd, cmd_len);
                        command_hist.cursor = cmd_len;
                    }
                } else {
                    log_debug_console(command_hist, "Command `%s` matched the following:", command_buff);
                    for (size_t i = 0; i < matched_commands_ids.count; ++i) {
                        int idx = matched_commands_ids.items[i];
                        log_debug_console(command_hist, "    - %s", commands[idx]);
                    }
                }
            }

            if (IsKeyPressed(KEY_TAB)) {
                Command_ids matched_commands_ids = match_command(get_current_console_line_buff(&command_hist), commands, commands_count);
                if (matched_commands_ids.count == 1) {
                    const char *cmd = commands[matched_commands_ids.items[0]];
                    size_t cmd_len = strlen(cmd);
                    memcpy(get_current_console_line_buff(&command_hist), cmd, cmd_len);
                    command_hist.cursor = cmd_len;
                } else if (matched_commands_ids.count > 1) {
                    log_debug_console(command_hist, "Command `%s` matched the following:", get_current_console_line_buff(&command_hist));
                    for (size_t i = 0; i < matched_commands_ids.count; ++i) {
                        int idx = matched_commands_ids.items[i];
                        log_debug_console(command_hist, "    - %s", commands[idx]);
                    }
                }
            }
        } else {
            if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_SEMICOLON)) {
                is_in_command = true;
                clear_current_console_line(&command_hist);
            }

            // Move camera
            if (IsKeyDown(KEY_LEFT_SHIFT) && IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
                mpos_from = m_world;
            }
            if (IsKeyDown(KEY_LEFT_SHIFT) && IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
                cam.target.x -= m_world.x - mpos_from.x;
                cam.target.y -= m_world.y - mpos_from.y;
            }

            // Zoom camera
            float scroll = GetMouseWheelMove();
            cam.zoom += scroll * 100.f * GetFrameTime();
            if (cam.zoom <= 0.1f)
                cam.zoom = 0.1f;

            if (IsKeyPressed(KEY_ZERO)) {
                cam.zoom = 1.f;
            }

#ifdef DEBUG
            // Change entity_save_version @DEBUG
            if (IsKeyPressed(KEY_MINUS) && entity_save_version > 0) {
                entity_save_version--;
            }
            if (IsKeyPressed(KEY_EQUAL)) {
                entity_save_version++;
            }
#endif

            // Mode-specific Input
            switch (current_mode) {
            case MODE_NORMAL: {
                // Select entities
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    selecting = true;
                    selection_start = m_world;
                    selection.x = m_world.x;
                    selection.y = m_world.y;
                }
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    if (m_world.x < selection_start.x) {
                        selection.x = m_world.x;
                        selection.width = selection_start.x - m_world.x;
                    } else {
                        selection.width = m_world.x - selection.x;
                    }
                    if (m_world.y < selection_start.y) {
                        selection.y = m_world.y;
                        selection.height = selection_start.y - m_world.y;
                    } else {
                        selection.height = m_world.y - selection.y;
                    }
                }
                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                    selecting = false;
                    for (int i = (int)entities.count - 1; i >= 0; --i) {
                        Entity* e = &entities.items[i];
                        if (e->state & (1 << ESTATE_DEAD))
                            continue;
                        if (rect_contains_point(selection, e->pos)) {
                            e->state |= (1 << ESTATE_SELECTED);
                        } else {
                            if (!IsKeyDown(KEY_LEFT_CONTROL))
                                e->state &= !(1 << ESTATE_SELECTED);
                        }
                    }
                    if (hovering_entity) {
                        hovering_entity->state |= (1 << ESTATE_SELECTED);
                    }
                }
                /// Keybinds to change modes
                if (IsKeyDown(KEY_LEFT_CONTROL)) {
                    if (IsKeyPressed(KEY_C)) {
                        CHANGE_MODE(MODE_COPY);
                    }

                    if (IsKeyPressed(KEY_I)) {
                        CHANGE_MODE(MODE_INTERACT);
                    }

                    if (IsKeyPressed(KEY_H)) {
                        CHANGE_MODE(MODE_CHANGE);
                    }

                    if (IsKeyPressed(KEY_N)) {
                        CHANGE_MODE(MODE_NORMAL);
                    }
                }


                // Change Entity kind
                if (IsKeyPressed(KEY_E)) {
                    selected_entity_kind = (selected_entity_kind + 1) % EK_COUNT;
                    ASSERT(load_texture(&tex_man, entity_texture_path_map[selected_entity_kind], &selected_entity_kind_tex), "THIS SHOULDNT FAIL!");
                }
                if (IsKeyPressed(KEY_Q)) {
                    if (selected_entity_kind == 0)
                        selected_entity_kind = EK_COUNT - 1;
                    else
                        selected_entity_kind--;
                    ASSERT(load_texture(&tex_man, entity_texture_path_map[selected_entity_kind], &selected_entity_kind_tex), "THIS SHOULDNT FAIL!");
                }

                // @DEBUG: Test ethernet frame transfer
                if (IsKeyPressed(KEY_J)) {
                    Entity* dst = NULL;
                    Entity* src = NULL;
                    for (int i = (int)entities.count - 1; i >= 0; --i) {
                        Entity* e = &entities.items[i];
                        if (e->state & (1 << ESTATE_DEAD))
                            continue;
                        if (e->state & (1 << ESTATE_SELECTED)) {
                            if (e->kind == EK_NIC) {
                                if (dst == NULL) {
                                    dst = e;
                                } else {
                                    if (e != dst) {
                                        src = e;
                                    }
                                }
                            }
                        }
                    }

                    if (dst && src) {
                        send_arp_ethernet_frame(dst, src);
                    }
                }

                // Add Entity
                if (IsKeyPressed(KEY_SPACE)) {
                    float64 tp1 = GetTime();
                    Entity e = make_entity(&entities, m_world, ENTITY_DEFAULT_RADIUS,
                        selected_entity_kind, &entity_arena, &temp_arena, &str_arena);
                    log_debug("make_entity() took %.2lfs", GetTime() - tp1);
                    add_entity(e);
                }

                if (IsKeyPressed(KEY_D)) {
                    if (IsKeyDown(KEY_LEFT_SHIFT)) {
                        // Delete Selected entities
                        for (int i = (int)entities.count - 1; i >= 0; --i) {
                            Entity* e = &entities.items[i];
                            if (e->state & (1 << ESTATE_DEAD))
                                continue;
                            if (e->state & (1 << ESTATE_SELECTED)) {
                                free_entity(e);
                                e->state |= (1 << ESTATE_DEAD);
                                // arr_remove(entities, Entity, &d, (int)i);
                                darr_append(free_entity_indices, i);
                                ASSERT(entities_count > 0,
                                    "We cant remove if there are no entities");
                                entities_count--;
                            }
                        }
                        if (entities_count == 0) {
                            log_debug("Resetting entity_arena");
                            arena_reset(&entity_arena);
                        }
                    } else {
                        // Disconnect connections of Selected entities
                        for (int i = (int)entities.count - 1; i >= 0; --i) {
                            Entity* e = &entities.items[i];
                            if (e->state & (1 << ESTATE_DEAD))
                                continue;
                            if (e->state & (1 << ESTATE_SELECTED)) {
                                disconnect_entity(e);
                            }
                        }
                    }
                }

                // Select/Deselect all
                if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_A)) {
                    for (int i = (int)entities.count - 1; i >= 0; --i) {
                        Entity* e = &entities.items[i];
                        if (e->state & (1 << ESTATE_DEAD))
                            continue;
                        if (IsKeyDown(KEY_LEFT_SHIFT)) {
                            e->state &= ~(1 << ESTATE_SELECTED);
                        } else {
                            e->state |= (1 << ESTATE_SELECTED);
                        }
                    }
                }

                // Move selected entities
                if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) || IsKeyPressed(KEY_Z)) {
                    for (int i = (int)entities.count - 1; i >= 0; --i) {
                        Entity* e = &entities.items[i];
                        if (e->state & (1 << ESTATE_DEAD))
                            continue;
                        e->offset = Vector2Subtract(e->pos, m_world);
                    }
                }

                if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || IsKeyDown(KEY_Z)) {
                    if (hovering_entity) {
                        hovering_entity->pos = Vector2Add(m_world, hovering_entity->offset);
                    } else {
                        for (int i = (int)entities.count - 1; i >= 0; --i) {
                            Entity* e = &entities.items[i];
                            if (e->state & (1 << ESTATE_DEAD))
                                continue;
                            if (e->state & (1 << ESTATE_SELECTED)) {
                                e->pos = Vector2Add(m_world, e->offset);
                            }
                        }
                    }
                }

                // Connect
                if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyPressed(KEY_X)) {
                    if (hovering_entity) {
                        connecting_from = hovering_entity;
                        connecting_from->state |= (1 << ESTATE_CONNECTING_FROM);
                    }
                }

                if ((IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsKeyDown(KEY_X)) && connecting_from) {
                    if (hovering_entity && hovering_entity != connecting_from) {
                        connecting_to = hovering_entity;
                        if (connecting_to)
                            connecting_to->state |= (1 << ESTATE_CONNECTING_TO);
                    }
                }

                if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) || IsKeyReleased(KEY_X)) {
                    if (connecting_from && connecting_to) {
                        connect_entity(&entities, connecting_from, connecting_to);
                    }
                    connecting_from = NULL;
                    connecting_to = NULL;
                }
            } break;
            case MODE_COPY: {
                copy_entity_info(hovering_entity);

                if (IsKeyPressed(KEY_ESCAPE)) {
                    current_mode = last_mode;
                }
            } break;
            case MODE_CHANGE: {
                if (!is_changing) {
                    if (IsKeyPressed(KEY_ONE)) {
                        is_changing = true;
                        changing_type = CHANGE_IPV4;
                    }
                    if (IsKeyPressed(KEY_TWO)) {
                        is_changing = true;
                        changing_type = CHANGE_SUBNET_MASK;
                    }
                }

                if (IsKeyPressed(KEY_ESCAPE)) {
                    current_mode = last_mode;
                    is_changing = false;
                    chars_buff_count = 0;
                }
            } break;
            case MODE_INTERACT: {
                if (active_switch_console) {
                    if (!active_switch->booted) {
                        boot_switch(active_switch, GetFrameTime());
                    } else {
                        if (input_to_console(active_switch_console)) {
                            char *buff = get_current_console_line_buff(active_switch_console);
                            String_view_array args = get_current_console_args(active_switch_console);

                            add_line_to_console_prefixed(active_switch_console, &temp_arena, buff, WHITE);

                            if (!parse_switch_console_cmd(active_switch, args)) {
                                clear_current_console_line(active_switch_console);
                                active_switch_console = NULL;
                                active_switch = NULL;
                            }
                            if (active_switch_console)
                                clear_current_console_line(active_switch_console);
                        }
                        if (IsKeyPressed(KEY_ESCAPE)) {
                            active_switch_console = NULL;
                            active_switch = NULL;
                        }
                    }

                } else if (hovering_entity) {
                    switch (hovering_entity->kind) {
                        case EK_NIC: {

                        } break;
                        case EK_SWITCH: {
                            if (IsKeyPressed(KEY_C)) {
                                active_switch_console = &hovering_entity->switchh->console;
                                active_switch = hovering_entity->switchh;

                                active_switch_console_rect.x = (width * 0.5f) - active_switch_console_rect.width * 0.5f;
                                active_switch_console_rect.y = (height * 0.5f) - active_switch_console_rect.height * 0.5f;
                            }
                        } break;
                        case EK_ACCESS_POINT: {
                            if (IsKeyPressed(KEY_P)) {
                                hovering_entity->ap->on = !hovering_entity->ap->on;
                            }
                        } break;
                        case EK_COUNT:
                        default: ASSERT(false, "UNREACHABLE!");
                    }
                }

                if (!active_switch_console && IsKeyPressed(KEY_ESCAPE)) {
                    current_mode = last_mode;
                    active_switch_console = NULL;
                    active_switch = NULL;
                }
            } break;
            case MODE_COUNT:
            default:
                ASSERT(false, "UNREACHABLE!");
            }
        }
        BeginTextureMode(ren_tex);
        ClearBackground(BLACK);

        // Update
        // Find hovering entity
        hovering_entity = NULL;
        for (int i = (int)entities.count - 1; i >= 0; --i) {
            Entity* e = &entities.items[i];
            if (e->state & (1 << ESTATE_DEAD))
                continue;
            float dist_sq = Vector2DistanceSqr(e->pos, m_world);
            // Clear states
            e->state &= ~(1 << ESTATE_HOVERING);
            if (e != connecting_from)
                e->state &= ~(1 << ESTATE_CONNECTING_FROM);
            if (e != connecting_to)
                e->state &= ~(1 << ESTATE_CONNECTING_TO);
            if (dist_sq <= e->radius * e->radius) {
                hovering_entity = e;
                hovering_entity->state |= (1 << ESTATE_HOVERING);
            }
        }

        // Mode-specific Update
        switch (current_mode) {
        case MODE_NORMAL: {
            // Update entities
            for (int i = (int)entities.count - 1; i >= 0; --i) {
                Entity* e = &entities.items[i];
                if (e->state & (1 << ESTATE_DEAD))
                    continue;
                
                update_entity(e);
            }

            // Update Wifi-waves
            for (size_t i = 0; i < wifi_waves.count; ++i) {
                Wifi_wave *ww = &wifi_waves.items[i];
                if (ww->radius >= ww->dead_zone) {
                    darr_delete(wifi_waves, Wifi_wave, i);
                } else {
                    ww->radius += GetFrameTime() * 100.f;
                }
                ww->color = ColorAlpha(ww->color, mapf(ww->radius, 0, ww->dead_zone, 1, 0));
            }
        } break;
        case MODE_COPY: {
        } break;
        case MODE_CHANGE: {
            if (hovering_entity == NULL) {
                is_changing = false;
                chars_buff_count = 0;
            } else {
                switch (changing_type) {
                    case CHANGE_IPV4: {
                        if (ipv4_from_input(hovering_entity, chars_buff,
                                &chars_buff_count, chars_buff_cap)) {
                            is_changing = false;
                        }
                    } break;
                    case CHANGE_SUBNET_MASK: {
                        if (subnet_mask_from_input(hovering_entity, chars_buff,
                                &chars_buff_count, chars_buff_cap)) {
                            is_changing = false;
                        }
                    } break;
                    case CHANGE_COUNT:
                    default:
                        ASSERT(false, "UNREACHABLE!");
                }
            }
        } break;
        case MODE_INTERACT: {
        } break;
        case MODE_COUNT:
        default:
            ASSERT(false, "UNREACHABLE!");
        }

        // Draw
        BeginMode2D(cam);
        // Draw Entities
        for (int i = (int)entities.count - 1; i >= 0; --i) {
            Entity* e = &entities.items[i];
            if (e->state & (1 << ESTATE_DEAD))
                continue;
            draw_entity(e, debug_draw);
        }

        // /// @DEBUG
        // DrawCircleV(v2(xoffset, 0.f), radius, blue ? BLUE : WHITE);
        // ///

        // Draw Wifi-waves
        for (size_t i = 0; i < wifi_waves.count; ++i) {
            Wifi_wave *ww = &wifi_waves.items[i];
            DrawCircleLinesV(ww->pos, ww->radius, ww->color);
        }

        //// DEBUG: Draw mpos_from and m_world - mpos_from
        // DrawCircle(mpos_from.x, mpos_from.y, 8, RED);
        // if (IsKeyDown(KEY_LEFT_SHIFT) &&
        // IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
        // {
        //    DrawLineV(m_world, mpos_from, WHITE);
        // }
        EndMode2D();

        int y = (ENTITY_DEFAULT_RADIUS * 0.5) * 2 + (2 * 2);
        if (debug_draw) {
            draw_text_aligned(GetFontDefault(), mode_as_str(current_mode), v2(2, 2),
                ENTITY_DEFAULT_RADIUS * 0.5, TEXT_ALIGN_V_TOP,
                TEXT_ALIGN_H_LEFT, GOLD);

            const char* hovering_entity_str = arena_alloc_str(temp_arena, "Hovering: %p", hovering_entity);
            const char* connecting_from_str = arena_alloc_str(temp_arena, "From: %p", connecting_from);
            const char* connecting_to_str = arena_alloc_str(temp_arena, "To: %p", connecting_to);
            draw_text(GetFontDefault(), hovering_entity_str, v2(2, y),
                ENTITY_DEFAULT_RADIUS * 0.5, WHITE);
            y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
            if (hovering_entity) {
                const char* hovering_entity_state_str = arena_alloc_str(
                    temp_arena, "Hovering state: " BYTE_TO_BINARY_PATTERN,
                    BYTE_TO_BINARY(hovering_entity->state & 0xFF));
                draw_text(GetFontDefault(), hovering_entity_state_str, v2(2, y),
                    ENTITY_DEFAULT_RADIUS * 0.5, WHITE);
                y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
                const char* hovering_id = arena_alloc_str(
                    temp_arena, "Hovering ID: %zu", hovering_entity->id);
                draw_text(GetFontDefault(), hovering_id, v2(2, y),
                    ENTITY_DEFAULT_RADIUS * 0.5, WHITE);
                y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;

                const char* hovering_pos = arena_alloc_str(temp_arena, "Hovering pos: %.2f, %.2f",
                    hovering_entity->pos.x, hovering_entity->pos.y);
                draw_text(GetFontDefault(), hovering_pos, v2(2, y),
                    ENTITY_DEFAULT_RADIUS * 0.5, WHITE);
                y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
            }
            draw_text(GetFontDefault(), connecting_from_str, v2(2, y),
                ENTITY_DEFAULT_RADIUS * 0.5, WHITE);
            y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
            draw_text(GetFontDefault(), connecting_to_str, v2(2, y),
                ENTITY_DEFAULT_RADIUS * 0.5, WHITE);
            y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;

            y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
            const char* entities_array_count_str = arena_alloc_str(temp_arena, "Entities.count: %zu", entities.count);
            draw_text(GetFontDefault(), entities_array_count_str, v2(2, y),
                ENTITY_DEFAULT_RADIUS * 0.5, WHITE);
            y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;

            const char* entities_count_str = arena_alloc_str(temp_arena, "Entities count: %zu", entities_count);
            draw_text(GetFontDefault(), entities_count_str, v2(2, y),
                ENTITY_DEFAULT_RADIUS * 0.5, WHITE);
            y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;

            const char* free_entity_indices_count_str = arena_alloc_str(temp_arena, "Free entity indices count: %zu",
                free_entity_indices.count);
            draw_text(GetFontDefault(), free_entity_indices_count_str, v2(2, y),
                ENTITY_DEFAULT_RADIUS * 0.5, WHITE);
            y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;

            if (hovering_entity) {
                if (hovering_entity->kind == EK_NIC) {
                    if (hovering_entity->nic->connected_entity) {
                        y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
                        const char* dst_str = arena_alloc_str(
                            temp_arena, "Hovering NIC connected to %s: %p", entity_kind_as_str(hovering_entity->nic->connected_entity->kind), 
                            hovering_entity->nic->connected_entity);
                        draw_text(GetFontDefault(), dst_str, v2(2, y),
                            ENTITY_DEFAULT_RADIUS * 0.5, WHITE);
                        y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
                    }
                }
            }

            if (active_switch_console) {
                const char* switch_console_str = arena_alloc_str(temp_arena, "Active Console: %p",
                    active_switch_console);
                draw_text(GetFontDefault(), switch_console_str, v2(2, y),
                    ENTITY_DEFAULT_RADIUS * 0.5, WHITE);
                y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
            }

            const char* e_arena_count_str = arena_alloc_str(
                temp_arena, "entity_arena.count: %zu",
                (size_t)((char*)entity_arena.ptr - (char*)entity_arena.buff));
            draw_text(GetFontDefault(), e_arena_count_str, v2(2, y),
                ENTITY_DEFAULT_RADIUS * 0.5, RED);
            y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;

            const char* free_mac_count_str = arena_alloc_str(
                temp_arena, "Freed MacAddr count: %zu", free_mac_addresses.count);
            draw_text(GetFontDefault(), free_mac_count_str, v2(2, y),
                ENTITY_DEFAULT_RADIUS * 0.5, RED);
            y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;

            const char* wifi_waves_count_str = arena_alloc_str(
                temp_arena, "Wifi waves count: %zu", wifi_waves.count);
            draw_text(GetFontDefault(), wifi_waves_count_str, v2(2, y),
                ENTITY_DEFAULT_RADIUS * 0.5, RED);
            y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;

            //// Right
            int yr = 0;
            const char* entity_save_version_str = arena_alloc_str(
                temp_arena, "Entity Save Version: %zu", entity_save_version);
            draw_text_aligned(GetFontDefault(), entity_save_version_str,
                v2(width, yr), ENTITY_DEFAULT_RADIUS * 0.5,
                TEXT_ALIGN_V_TOP, TEXT_ALIGN_H_RIGHT, YELLOW);
            yr += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
        }

        // Mode-specific Draw
        switch (current_mode) {
        case MODE_NORMAL: {
            const char* selected_entity_kind_str = arena_alloc_str(temp_arena, "Entity Kind: %s",
                entity_kind_as_str(selected_entity_kind));
            draw_text_aligned(GetFontDefault(), selected_entity_kind_str,
                v2(width * 0.5, 2), ENTITY_DEFAULT_RADIUS * 0.5,
                TEXT_ALIGN_V_TOP, TEXT_ALIGN_H_CENTER, WHITE);

            float xoffset_after_text = MeasureTextEx(GetFontDefault(), selected_entity_kind_str, ENTITY_DEFAULT_RADIUS * 0.5f, 2.5f).x / 2.f;

            // TODO: Draw Entity Kind Sprite
            DrawTextureV(selected_entity_kind_tex, v2(width*0.5f + xoffset_after_text, 2.f), WHITE);


            BeginMode2D(cam);
            if (connecting_from) {
                DrawLineBezier(connecting_from->pos, m_world, 1.0, GRAY);
            }

            if (selecting)
                DrawRectangleLinesEx(selection, 1.0, WHITE);
            EndMode2D();
        } break;
        case MODE_COPY: {
            draw_text(GetFontDefault(), "-- COPY KEYS --", v2(2, y),
                ENTITY_DEFAULT_RADIUS * 0.5, YELLOW);
            y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;

            {
                const char* a = arena_alloc_str(temp_arena, "%s", "[1]: Copy ipv4");
                draw_text(GetFontDefault(), a, v2(2, y), ENTITY_DEFAULT_RADIUS * 0.5,
                    YELLOW);
                y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
            }
            {
                const char* a = arena_alloc_str(temp_arena, "%s", "[2]: Copy subnet mask");
                draw_text(GetFontDefault(), a, v2(2, y), ENTITY_DEFAULT_RADIUS * 0.5,
                    YELLOW);
                y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
            }
            {
                const char* a = arena_alloc_str(temp_arena, "%s", "[3]: Copy mac address");
                draw_text(GetFontDefault(), a, v2(2, y), ENTITY_DEFAULT_RADIUS * 0.5,
                    YELLOW);
                y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
            }
            {
                const char* a = arena_alloc_str(temp_arena, "%s",
                    "[Esc]: Go back to previous mode");
                draw_text(GetFontDefault(), a, v2(2, y), ENTITY_DEFAULT_RADIUS * 0.5,
                    YELLOW);
                y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
            }
            draw_text(GetFontDefault(), "-----------------", v2(2, y),
                ENTITY_DEFAULT_RADIUS * 0.5, YELLOW);
            y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
        } break;
        case MODE_CHANGE: {
            const char* changing_str = arena_alloc_str(
                temp_arena, "Changing %s %s", is_changing ? changing_type_as_str(changing_type) : "nothing", hovering_entity ? "of hovering entity" : "of nothing");
            draw_text(GetFontDefault(), changing_str, v2(2, y),
                ENTITY_DEFAULT_RADIUS * 0.5, WHITE);
            y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;

            if (is_changing) {
                const char* changing_input_str = arena_alloc_str(
                    temp_arena, "%s: %.*s", changing_type_as_str(changing_type),
                    (int)chars_buff_count, (const char*)chars_buff);
                draw_text(GetFontDefault(), changing_input_str, v2(2, y),
                    ENTITY_DEFAULT_RADIUS * 0.5, BLUE);
                y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
            }

            {
                const char* a = arena_alloc_str(temp_arena, "%s", "[1]: Change ipv4");
                draw_text(GetFontDefault(), a, v2(2, y), ENTITY_DEFAULT_RADIUS * 0.5,
                    YELLOW);
                y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
            }
            {
                const char* a = arena_alloc_str(temp_arena, "%s", "[2]: Change subnet mask");
                draw_text(GetFontDefault(), a, v2(2, y), ENTITY_DEFAULT_RADIUS * 0.5,
                    YELLOW);
                y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
            }
        } break;
        case MODE_INTERACT: {
            {
                const char* a = arena_alloc_str(temp_arena, "Interacting with %s", hovering_entity ? entity_kind_as_str(hovering_entity->kind) : "Nothing");
                draw_text(GetFontDefault(), a, v2(2, y), ENTITY_DEFAULT_RADIUS * 0.5,
                    WHITE);
                y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
            }
            if (hovering_entity) {
                switch (hovering_entity->kind) {
                    case EK_NIC: {

                    } break;
                    case EK_SWITCH: {
                        {
                            const char* a = arena_alloc_str(temp_arena, "%s", "[C]: Toggle switch console");
                            draw_text(GetFontDefault(), a, v2(2, y), ENTITY_DEFAULT_RADIUS * 0.5,
                                YELLOW);
                            y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
                        }
                    } break;
                    case EK_ACCESS_POINT: {
                        {
                            const char* a = arena_alloc_str(temp_arena, "%s", "[P]: Toggle power");
                            draw_text(GetFontDefault(), a, v2(2, y), ENTITY_DEFAULT_RADIUS * 0.5,
                                YELLOW);
                            y += ENTITY_DEFAULT_RADIUS * 0.5 + 2;
                        }

                    } break;
                    case EK_COUNT:
                    default: ASSERT(false, "UNREACHABLE!");
                }
            }
            if (active_switch_console) {
                
                draw_console(active_switch_console, active_switch_console_rect, v2(8, -8), ENTITY_DEFAULT_RADIUS*0.5f);
                
            }
        } break;
        case MODE_COUNT:
        default:
            ASSERT(false, "UNREACHABLE!");
        }

        if (is_in_command) {

            int console_font_size = ENTITY_DEFAULT_RADIUS * 0.5f;

            Rectangle command_rect = {
                .x = 0.f,
                .y = height * 0.5f - console_font_size,
                .width = width,
                .height = height * 0.5f,
            };
            draw_console(&command_hist, command_rect, v2(8, -8), console_font_size);
        }

        EndTextureMode();
        draw_ren_tex(ren_tex, SCREEN_WIDTH, SCREEN_HEIGHT);
        EndDrawing();
    }

    cleanup();
    return 0;
}

bool set_var(Console *console, Var *var, String_view newvalue) {
    if (!var) return false;
    switch (var->type) {
        case VAR_TYPE_INT: {
            return set_int_var(console, var, newvalue);
        } break;
        case VAR_TYPE_FLOAT: {
            return set_float_var(console, var, newvalue);
        } break;
        case VAR_TYPE_STRING: {
            return set_str_var(console, var, newvalue);
        } break;
        case VAR_TYPE_CHAR: {
            return set_char_var(console, var, newvalue);
        } break;
        case VAR_TYPE_BOOL: {
            return set_bool_var(console, var, newvalue);
        } break;
        case VAR_TYPE_COUNT:
        default:
            ASSERT(false, "UNREACHABLE!");
    }
    return false;
}

bool set_int_var(Console *console, Var *var, String_view newvalue) {
    int v_count = -1;
    int v = sv_to_int(newvalue, &v_count, 10);
    if (v_count <= -1) {
        log_error_a(*console, "Failed to set `%s` to `"SV_FMT"`: is not an INT!", var->name, SV_ARG(newvalue));
        return false;
    }

    ASSERT(var->value_ptr, "THIS SHOULDN't HAPPEN!");

    int *value_ptr = (int*)var->value_ptr;
    *value_ptr = v;

    log_info_a(*console, "SET %s from %d to %d", var->name, *value_ptr, v);
    return true;
}

bool set_float_var(Console *console, Var *var, String_view newvalue) {
    int v_count = -1;
    float v = sv_to_float(newvalue, &v_count);
    if (v_count <= -1) {
        log_error_a(*console, "Failed to set `%s` to `"SV_FMT"`: is not an FLOAT!", var->name, SV_ARG(newvalue));
        return false;
    }

    ASSERT(var->value_ptr, "THIS SHOULDN't HAPPEN!");

    float *value_ptr = (float*)var->value_ptr;
    
    log_info_a(*console, "SET %s from %f to %f", var->name, *value_ptr, v);

    *value_ptr = v;

    return true;
}

bool set_str_var(Console *console, Var *var, String_view newvalue) {
    if (!var) return false;
    char **value_ptr = (char **)var->value_ptr;

    log_info_a(*console, "SET %s from %s to "SV_FMT, var->name, *value_ptr, SV_ARG(newvalue));

    *value_ptr = arena_alloc_str(str_arena, SV_FMT, SV_ARG(newvalue));

    return true;
}


bool set_char_var(Console *console, Var *var, String_view newvalue) {
    if (!var) return false;

    char *value_ptr = (char *)var->value_ptr;

    *value_ptr = newvalue.data[0];

    log_info_a(*console, "SET %s from %c to %c", var->name, *value_ptr, newvalue.data[0]);

    return true;
}

bool set_bool_var(Console *console, Var *var, String_view newvalue) {
    if (!var) return false;

    char *newvalue_str = sv_to_cstr(newvalue);
    size_t newvalue_str_len = strlen(newvalue_str);
    char *newvalue_str_lower = (char *)malloc(newvalue_str_len*sizeof(char) + 1);

    // tolower
    for (int i = 0; i < newvalue_str_len; ++i) {
        newvalue_str_lower[i] = tolower(newvalue_str[i]);
    }
    newvalue_str_lower[newvalue_str_len] = 0;

    bool istrue = strcmp(newvalue_str_lower, "true") == 0;

    log_info_a(*console, "SET %s from %s to %s", var->name, *((int*)var->value_ptr) ? "true" : "false", istrue ? "true" : "false");

    if (istrue) {
        *((int*)var->value_ptr) = 1;
    } else {
        *((int*)var->value_ptr) = 0;
    }

    free(newvalue_str);
    free(newvalue_str_lower);

    return true;
}


Var *get_var(String_view name) {
    for (size_t i = 0; i < vars.count; ++i) {
        Var *v = &vars.items[i];
        if (sv_equals(name, SV(v->name))) {
            return v;
        }
    }
    return NULL;
}


const char *get_var_value(Var *var) {
    if (!var) {
        log_error("Var is NULL!");
        return NULL;
    }
    if (!var->value_ptr) {
        log_error("Var's value_ptr is NULL!");
        return NULL;
    }
    switch (var->type) {
        case VAR_TYPE_INT: {
            return arena_alloc_str(temp_arena, "%d", *((int*)(var->value_ptr)));
        } break;
        case VAR_TYPE_FLOAT: {
            return arena_alloc_str(temp_arena, "%f", *((float*)(var->value_ptr)));
        } break;
        case VAR_TYPE_STRING: {
            return arena_alloc_str(temp_arena, "%s", *((char**)(var->value_ptr)));
        } break;
        case VAR_TYPE_CHAR: {
            return arena_alloc_str(temp_arena, "%c", *((char*)(var->value_ptr)));
        } break;
        case VAR_TYPE_BOOL: {
            return arena_alloc_str(temp_arena, "%s", *((int*)(var->value_ptr)) ? "true" : "false");
        } break;
        case VAR_TYPE_COUNT:
        default:
            ASSERT(false, "UNREACHABLE!");

    }
    return NULL;
}


