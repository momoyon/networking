#define COMMONLIB_IMPLEMENTATION
#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

#define ENGINE_IMPLEMENTATION
#include <engine.h>

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720
#define SCREEN_SCALE  0.5

int main(void) {
    int width = 0;
    int height = 0;

    RenderTexture2D ren_tex = init_window(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_SCALE, "Networking", &width, &height);

    Font font = GetFontDefault();

    while (!WindowShouldClose()) {
        BeginDrawing();
        BeginTextureMode(ren_tex);
            ClearBackground(BLACK);
        EndTextureMode();
        draw_ren_tex(ren_tex, SCREEN_WIDTH, SCREEN_HEIGHT);
        EndDrawing();
    }

    close_window(ren_tex);
    return 0;
}
