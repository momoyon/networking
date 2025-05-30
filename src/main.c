#include <config.h>
#include <entity.h>
#include <connection.h>

#define COMMONLIB_REMOVE_PREFIX
#define COMMONLIB_IMPLEMENTATION
#include <commonlib.h>

#define ENGINE_IMPLEMENTATION
#include <engine.h>

#define FACTOR 105
#define SCREEN_WIDTH  (16*FACTOR)
#define SCREEN_HEIGHT (9*FACTOR)
#define SCREEN_SCALE  0.5

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0')

typedef enum {
    MODE_NORMAL,
    MODE_COUNT,
} Mode;

const char *mode_as_str(const Mode m) {
    switch (m) {
        case MODE_NORMAL: return  "Normal";
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
    Mode current_mode = MODE_NORMAL;

    Entity_kind selected_entity_kind = EK_NETWORK_DEVICE;
    Entity *hovering_entity = NULL;
    Entity *connecting_from = NULL;
    Entity *connecting_to = NULL;

    Rectangle selection = {0};
    Vector2 selection_start = {0};
    bool selecting = false;

    Arena entity_arena = arena_make(32*1024);

    Arena temp_arena = arena_make(0);

    while (!WindowShouldClose()) {
        arena_reset(&temp_arena);
        BeginDrawing();
        Vector2 m = get_mpos_scaled(SCREEN_SCALE);

        // Input
        if (IsKeyPressed(KEY_TAB)) {
            current_mode = (current_mode + 1) % MODE_COUNT;
        }
        if (IsKeyPressed(KEY_GRAVE)) {
            debug_draw = !debug_draw;
        }

        // Mode-specific input
        switch (current_mode) {
            case MODE_NORMAL: {
                if (IsKeyPressed(KEY_E)) {
                    selected_entity_kind = (selected_entity_kind + 1) % EK_COUNT;
                }
                if (IsKeyPressed(KEY_Q)) {
                    if (selected_entity_kind==0)
                        selected_entity_kind = EK_COUNT-1;
                    else
                        selected_entity_kind--;
                }

                if (IsKeyPressed(KEY_SPACE)) {
                    Entity e = make_entity(m, ENTITY_DEFAULT_RADIUS, selected_entity_kind, &entity_arena, &temp_arena);
                    da_append(entities, e);
                    log_debug("Added %s %zu at %f, %f", entity_kind_as_str(e.kind), e.id, e.pos.x, e.pos.y);
                }

                // Select/Deselect all
                if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_A)) {
                    for (size_t i = 0; i < entities.count; ++i) {
                        Entity *e = &entities.items[i];
                        if (IsKeyDown(KEY_LEFT_SHIFT)) {
                            e->state &= ~(1<<ESTATE_SELECTED);
                        } else {
                            e->state |= (1<<ESTATE_SELECTED);
                        }
                    }
                }

                // Moving selected entities
                if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
                    for (size_t i = 0; i < entities.count; ++i) {
                        Entity *e = &entities.items[i];
                        e->offset = Vector2Subtract(e->pos, m);
                    }
                }
                if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
                    if (hovering_entity) {
                        hovering_entity->pos = Vector2Add(m, hovering_entity->offset);
                    } else {
                        for (size_t i = 0; i < entities.count; ++i) {
                            Entity *e = &entities.items[i];
                            if (e->state & (1<<ESTATE_SELECTED)) {
                                e->pos = Vector2Add(m, e->offset);
                            }
                        }
                    }
                }

                if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                    if (hovering_entity) {
                        connecting_from = hovering_entity;
                        connecting_from->state |= (1<<ESTATE_CONNECTING_FROM);
                    }
                }

                if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && connecting_from) {
                    if (hovering_entity != connecting_from) {
                        connecting_to = hovering_entity;
                        if (connecting_to)
                            connecting_to->state |= (1<<ESTATE_CONNECTING_TO);
                    }
                }

                if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
                    if (connecting_from && connecting_to) {
                        connect(connecting_from, connecting_to);
                    }
                    connecting_from = NULL;
                    connecting_to = NULL;
                }

            } break;
            case MODE_COUNT:
            default: ASSERT(false, "UNREACHABLE!");
        }

        BeginTextureMode(ren_tex);
            ClearBackground(BLACK);

            // Edit
            // Find hovering entity
            hovering_entity = NULL;
            for (size_t i = 0; i < entities.count; ++i) {
                Entity *e = &entities.items[i];
                float dist_sq = Vector2DistanceSqr(e->pos, m);
                // Clear states
                e->state &= ~(1<<ESTATE_HOVERING);
                if (e != connecting_from)
                    e->state &= ~(1<<ESTATE_CONNECTING_FROM);
                if (e != connecting_to)
                    e->state &= ~(1<<ESTATE_CONNECTING_TO);
                if (dist_sq <= e->radius*e->radius) {
                    hovering_entity = e;
                    hovering_entity->state |= (1<<ESTATE_HOVERING);
                }
            }
            // Mode-specific edit
            switch (current_mode) {
                case MODE_NORMAL: {
                    // Select entities
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        selecting = true;
                        selection_start = m;
                        selection.x = m.x;
                        selection.y = m.y;
                    }
                    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                        if (m.x < selection_start.x) {
                            selection.x = m.x;
                            selection.width = selection_start.x - m.x;
                        } else {
                            selection.width = m.x - selection.x;
                        }
                        if (m.y < selection_start.y) {
                            selection.y = m.y;
                            selection.height = selection_start.y - m.y;
                        } else {
                            selection.height = m.y - selection.y;
                        }
                    }
                    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                        selecting = false;
                        for (size_t i = 0; i < entities.count; ++i) {
                            Entity *e = &entities.items[i];
                            if (rect_contains_point(selection, e->pos)) {
                                e->state |= (1<<ESTATE_SELECTED);
                            } else {
                                e->state &= !(1<<ESTATE_SELECTED);
                            }
                        }
                        if (hovering_entity) {
                            hovering_entity->state |= (1<<ESTATE_SELECTED);
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


                int y = (ENTITY_DEFAULT_RADIUS*0.5) * 2 + (2*2);
                const char *hovering_entity_str = arena_alloc_str(temp_arena, "Hovering: %p", hovering_entity);
                const char *connecting_from_str = arena_alloc_str(temp_arena, "From: %p", connecting_from);
                const char *connecting_to_str = arena_alloc_str(temp_arena, "To: %p", connecting_to);
                draw_text(GetFontDefault(), hovering_entity_str, v2(2, y), ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                y += ENTITY_DEFAULT_RADIUS*0.5 + 2;
                if (hovering_entity) {
                    const char *hovering_entity_state_str = arena_alloc_str(temp_arena, "Hovering state: "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(hovering_entity->state & 0xFF));
                    draw_text(GetFontDefault(), hovering_entity_state_str, v2(2, y), ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                    y += ENTITY_DEFAULT_RADIUS*0.5 + 2;
                    const char *hovering_entity_connections_str = arena_alloc_str(temp_arena, "Hovering connections: %zu", hovering_entity->connections.count);
                    draw_text(GetFontDefault(), hovering_entity_connections_str, v2(2, y), ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                    y += ENTITY_DEFAULT_RADIUS*0.5 + 2;
                }
                draw_text(GetFontDefault(), connecting_from_str, v2(2, y), ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                y += ENTITY_DEFAULT_RADIUS*0.5 + 2;
                draw_text(GetFontDefault(), connecting_to_str, v2(2, y), ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                y += ENTITY_DEFAULT_RADIUS*0.5 + 2;
            }

            // Mode-specific draw
            switch (current_mode) {
                case MODE_NORMAL: {
                    const char *selected_entity_kind_str = arena_alloc_str(temp_arena, "Entity Kind: %s", entity_kind_as_str(selected_entity_kind));
                    draw_text_aligned(GetFontDefault(), selected_entity_kind_str, v2(width*0.5, 2), ENTITY_DEFAULT_RADIUS*0.5, TEXT_ALIGN_V_TOP, TEXT_ALIGN_H_CENTER, WHITE);
                    if (connecting_from) {
                        DrawLineBezier(connecting_from->pos, m, 1.0, GRAY);
                    }

                    if (selecting)
                        DrawRectangleLinesEx(selection, 1.0, WHITE);
                } break;
                case MODE_COUNT:
                default: ASSERT(false, "UNREACHABLE!");
            }

        EndTextureMode();
        draw_ren_tex(ren_tex, SCREEN_WIDTH, SCREEN_HEIGHT);
        EndDrawing();
    }

    arena_free(&entity_arena);
    arena_free(&temp_arena);
    close_window(ren_tex);
    return 0;
}
