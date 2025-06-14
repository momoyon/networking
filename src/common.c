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
		log_error("The width of '%s' is incorrect! Please make all the textures %d pixels!", ENTITY_SIZE_IN_PIXELS);
		UnloadTexture(tex);
		crash();
	}
	if (tex.height != ENTITY_SIZE_IN_PIXELS) {
		log_error("The height of '%s' is incorrect! Please make all the textures %d pixels!", ENTITY_SIZE_IN_PIXELS);
		UnloadTexture(tex);
		crash();
	}

	// log_debug("Loaded Texture '%s' (%dx%d)", filepath, tex.width, tex.height);

	return tex;
}
