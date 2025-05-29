
#include <entity.h>

#define COMMONLIB_REMOVE_PREFIX
#define COMMONLIB_IMPLEMENTATION
#include <commonlib.h>

#define ENGINE_IMPLEMENTATION
#include <engine.h>

#define FACTOR 105
#define SCREEN_WIDTH  (16*FACTOR)
#define SCREEN_HEIGHT (9*FACTOR)
#define SCREEN_SCALE  0.5

int main(void) {
    int width = 0;
    int height = 0;

#if defined(DEBUG)
    bool debug_draw = true;
#else
    bool debug_draw = false;
#endif // defined(DEBUG)

    RenderTexture2D ren_tex = init_window(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_SCALE, "Networking", &width, &height);

    // Font font = GetFontDefault();

    Entities entities = {0};

    while (!WindowShouldClose()) {
        BeginDrawing();
        Vector2 m = get_mpos_scaled(SCREEN_SCALE);

        // Input
        if (IsMouseButtonPressed(MOUSE_BUTTON_EXTRA)) {
            Entity e = make_entity(m, EK_NONE);
            da_append(entities, e);
            log_debug("Added Entity %zu at %f, %f", e.id, e.pos.x, e.pos.y);
        }

        BeginTextureMode(ren_tex);
            ClearBackground(BLACK);

            // Draw
            for (size_t i = 0; i < entities.count; ++i) {
                Entity *e = &entities.items[i];
                draw_entity(e, debug_draw);
            }
        EndTextureMode();
        draw_ren_tex(ren_tex, SCREEN_WIDTH, SCREEN_HEIGHT);
        EndDrawing();
    }

    close_window(ren_tex);
    return 0;
}
