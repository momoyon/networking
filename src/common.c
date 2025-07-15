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

// TODO: The logic for adding or deleting entities is messed up. entities.count and entities_count have to be the same when loading.
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
	log_debug("Added %s %zu at %f, %f", entity_kind_as_str(e.kind), e.id, e.pos.x, e.pos.y);
}
