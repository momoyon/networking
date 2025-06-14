#ifndef _COMMON_H_
#define _COMMON_H_

#include <raylib.h>
#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

extern RenderTexture2D ren_tex;
extern Arena entity_arena;
extern Arena temp_arena;

void cleanup(void);
void crash(void);
Texture2D load_texture_checked(const char *filepath);

#endif // _COMMON_H_
