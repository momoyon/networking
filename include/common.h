#ifndef _COMMON_H_
#define _COMMON_H_

#include <raylib.h>
#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

#include <engine.h>
#include <entity.h>
#include <ap.h>

#define log_info_a(console, fmt, ...) log_info_console((console), fmt, __VA_ARGS__); log_info(fmt, __VA_ARGS__)
#define log_warning_a(console, fmt, ...) log_warning_console((console), fmt, __VA_ARGS__); log_warning(fmt, __VA_ARGS__)
#define log_error_a(console, fmt, ...) log_error_console((console), fmt, __VA_ARGS__); log_error(fmt, __VA_ARGS__)

#define log_error_to_console(fmt, ...) log_error_console(error_console, fmt, ##__VA_ARGS__); log_error(fmt, ##__VA_ARGS__); error_console_activity = ERROR_CONSOLE_ACTIVITY_VALUE; error_console_alpha = 1.f;

typedef struct {
    int *items;
    size_t count;
    size_t capacity;
} Ids; // @darr

extern RenderTexture2D ren_tex;
extern Arena entity_arena;
extern Arena temp_arena;
extern Texture_manager tex_man;
extern size_t entity_save_version;
extern Wifi_waves wifi_waves;
extern Console error_console;
extern float error_console_activity;
extern float error_console_alpha;

void cleanup(void);
void crash(void);
Texture2D load_texture_checked(const char *filepath);
void add_entity(Entity e);
void emit_wifi(Vector2 pos, Color color, float dead_zone);

bool str_starts_with(const char *str, const char *suffix);
Ids match_command(const char *command, const char **commands, size_t commands_count);
bool parse_n_octet_from_data(int n, String_view *sv, uint8 *octets, size_t octets_count, bool for_mask);
bool parse_n_octet_with_mask_from_data(int n, String_view *sv, uint8 *octets, size_t octets_count, uint8 *mask);

#endif // _COMMON_H_
