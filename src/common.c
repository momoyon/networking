#include <common.h>
#include <config.h>
#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>
#include <engine.h>

void cleanup(void) {
    arena_free(&entity_arena);
    arena_free(&temp_arena);
    close_window(ren_tex);
}

void crash(void) {
	cleanup();
	exit(1);
}

Texture2D load_texture_checked(const char *filepath) {
	Texture2D tex = {0};

	if (!load_texture(&tex_man, filepath, &tex)) {
		log_debug("Failed to load texture '%s'", filepath);
		crash();
	}

	if (tex.width != ENTITY_SIZE_IN_PIXELS) {
		log_error("The width of '%s' is incorrect! Please make all the textures %d pixels!", filepath, ENTITY_SIZE_IN_PIXELS);
		UnloadTexture(tex);
		crash();
	}
	if (tex.height != ENTITY_SIZE_IN_PIXELS) {
		log_error("The height of '%s' is incorrect! Please make all the textures %d pixels!", filepath, ENTITY_SIZE_IN_PIXELS);
		UnloadTexture(tex);
		crash();
	}

	// log_debug("Loaded Texture '%s' (%dx%d)", filepath, tex.width, tex.height);

	return tex;
}

void add_entity(Entity e) {
	if (free_entity_indices.count == 0) {
		arr_append(entities, e);
	} else {
		int free_index = -1;
		darr_remove(free_entity_indices, int, &free_index, free_entity_indices.count-1);
		ASSERT(free_index >= 0, "This shouldn't happen!");
		entities.items[free_index] = e;
	}
	entities_count++;
    if (entities.count < entities_count) entities.count = entities_count;
	log_debug("Added %s %zu at %f, %f", entity_kind_as_str(e.kind), e.id, e.pos.x, e.pos.y);
}

void emit_wifi(Vector2 pos, Color color, float dead_zone) {
    Wifi_wave ww = {
        .pos = pos,
        .radius = 0.f,
        .color = color,
        .dead_zone = dead_zone,
    };
    darr_append(wifi_waves, ww);
}

bool str_starts_with(const char *str, const char *suffix) {
    if (str == NULL) return false;
    while (*str != 0 && *suffix != 0) {
        if (*suffix++ != *str++) {
            return false;
        }
    }
    return true;
}

Ids match_command(const char *command, const char **commands, size_t commands_count) {
    Ids matched_command_ids = {0};
    size_t command_len = strlen(command);
    char command_lower[1024] = {0};

    //
    size_t max = sizeof(command_lower) - 1;
    size_t copy_len = command_len > max ? max : command_len;
    for (size_t i=0;i<copy_len;++i) command_lower[i]=tolower((unsigned char)command[i]);
    command_lower[copy_len]=0;
    if (command_len > max) command_len = copy_len; // or treat as truncated
    //

    for (int i = 0; i < commands_count; ++i) {
        const char *c = commands[i];
        size_t c_len = strlen(c);
        if (str_starts_with(c, command_lower)) {
            if ((command_len > c_len && command_lower[c_len] == ' ')
              || command_len <= c_len) {
                darr_append(matched_command_ids, i);
            }
        }
    }

    return matched_command_ids;
}

Ids match_switch_console_command(const char *command, Switch_console_cmd *commands, size_t commands_count) {
    char **command_names = (char **)malloc(sizeof(char *) * commands_count);

    for (size_t i = 0; i < commands_count; ++i) {
        size_t command_name_len = strlen(commands[i].name);
        command_names[i] = (char *)malloc(sizeof(char) * command_name_len+1);
        memcpy(command_names[i], commands[i].name, command_name_len);
    }

    Ids res = match_command(command, (const char**)command_names, commands_count);

    for (size_t i = 0; i < commands_count; ++i) {
        free(command_names[i]);
    }

    free(command_names);

    return res;
}
