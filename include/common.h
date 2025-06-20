#ifndef _COMMON_H_
#define _COMMON_H_

#include <raylib.h>
#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

#include <engine.h>
#include <entity.h>

extern RenderTexture2D ren_tex;
extern Arena entity_arena;
extern Arena temp_arena;
extern Texture_manager tex_man;
extern size_t entity_save_version;

void cleanup(void);
void crash(void);
Texture2D load_texture_checked(const char *filepath);
void add_entity(Entity e);

#endif // _COMMON_H_
