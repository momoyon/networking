#include <config.h>
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


typedef enum {
    MODE_NONE,
    MODE_EDIT,
    MODE_COUNT,
} Mode;

const char *mode_as_str(const Mode m) {
    switch (m) {
        case MODE_NONE: return "None";
        case MODE_EDIT: return "Edit";
        case MODE_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
}

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

    Mode current_mode = MODE_NONE;

    Entity_kind selected_entity_kind = EK_NONE;

    Arena temp_arena = arena_make(0);

    while (!WindowShouldClose()) {
        arena_reset(&temp_arena);
        BeginDrawing();
        Vector2 m = get_mpos_scaled(SCREEN_SCALE);

        // Input
        if (IsMouseButtonPressed(MOUSE_BUTTON_EXTRA)) {
            Entity e = make_entity(m, ENTITY_DEFAULT_RADIUS, selected_entity_kind);
            da_append(entities, e);
            log_debug("Added Entity %zu at %f, %f", e.id, e.pos.x, e.pos.y);
        }

        if (IsKeyPressed(KEY_GRAVE)) {
            current_mode = (current_mode + 1) % MODE_COUNT;
        }

        // Mode-specific input
        switch (current_mode) {
            case MODE_NONE: {
            } break;
            case MODE_EDIT: {
                if (IsKeyPressed(KEY_E)) {
                    selected_entity_kind = (selected_entity_kind + 1) % EK_COUNT;
                }
            } break;
            case MODE_COUNT:
            default: ASSERT(false, "UNREACHABLE!");
        }

        BeginTextureMode(ren_tex);
            ClearBackground(BLACK);

            // Edit
            // Mode-specific edit
            switch (current_mode) {
                case MODE_NONE: {
                } break;
                case MODE_EDIT: {
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        for (size_t i = 0; i < entities.count; ++i) {
                            Entity *e = &entities.items[i];
                            float dist_sq = Vector2DistanceSqr(e->pos, m);
                            if (dist_sq <= e->radius*e->radius) e->selected = !e->selected;
                        }
                    }
                } break;
                case MODE_COUNT:
                default: ASSERT(false, "UNREACHABLE!");
            }

            // Draw
            for (size_t i = 0; i < entities.count; ++i) {
                Entity *e = &entities.items[i];
                draw_entity(e, debug_draw);
            }

            if (debug_draw) {
                draw_text_aligned(GetFontDefault(), mode_as_str(current_mode), v2(2, 2), ENTITY_DEFAULT_RADIUS*0.5, TEXT_ALIGN_V_TOP, TEXT_ALIGN_H_LEFT, WHITE);
            }

            // Mode-specific draw
            switch (current_mode) {
                case MODE_NONE: {
                } break;
                case MODE_EDIT: {
                    const char *selected_entity_kind_str = arena_alloc_str(temp_arena, "Entity Kind: %s", entity_kind_as_str(selected_entity_kind));
                    draw_text_aligned(GetFontDefault(), selected_entity_kind_str, v2(width*0.5, 2), ENTITY_DEFAULT_RADIUS*0.5, TEXT_ALIGN_V_TOP, TEXT_ALIGN_H_CENTER, WHITE);
                } break;
                case MODE_COUNT:
                default: ASSERT(false, "UNREACHABLE!");
            }

        EndTextureMode();
        draw_ren_tex(ren_tex, SCREEN_WIDTH, SCREEN_HEIGHT);
        EndDrawing();
    }

    arena_free(&temp_arena);
    close_window(ren_tex);
    return 0;
}
