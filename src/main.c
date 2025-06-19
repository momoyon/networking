#include<predecls.h>
#include <config.h>
#include <nic.h>
#include <entity.h>
#include <common.h>

#define COMMONLIB_REMOVE_PREFIX
#define COMMONLIB_IMPLEMENTATION
#include <commonlib.h>

#define ENGINE_IMPLEMENTATION
#include <engine.h>

#define STB_DS_IMPLEMENTATION
#include <stb_ds.h>

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

// Externs from common.h
RenderTexture2D ren_tex;
Arena entity_arena;
Arena temp_arena;
Texture_manager tex_man;

int main(void) {
    int width = 0;
    int height = 0;

#if defined(DEBUG)
    bool debug_draw = true;
#else
    bool debug_draw = false;
#endif // defined(DEBUG)

    const char *window_name = "Networking";
    ren_tex = init_window(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_SCALE, window_name, &width, &height);

    // Font font = GetFontDefault();
    //
	const char *entities_save_path = "test.entities";

    arr_heap_init(entities, ENTITIES_MAX_CAP);

    Mode current_mode = MODE_NORMAL;

    Entity_kind selected_entity_kind = EK_NIC;
    Entity *hovering_entity = NULL;
    Entity *connecting_from = NULL;
    Entity *connecting_to = NULL;

    Rectangle selection = {0};
    Vector2 selection_start = {0};
    bool selecting = false;

    entity_arena = arena_make(32*1024);
    temp_arena = arena_make(0);

	Camera2D cam = {
		.zoom = 1.0,
		.offset = CLITERAL(Vector2) { width / 2, height / 2 },
	};
	Vector2 mpos_from = {0};

    while (!WindowShouldClose()) {
        arena_reset(&temp_arena);

        const char *title_str = arena_alloc_str(temp_arena, "%s | %d FPS", window_name, GetFPS());

        SetWindowTitle(title_str);

        BeginDrawing();
        Vector2 m = get_mpos_scaled(SCREEN_SCALE);
		Vector2 m_world = GetScreenToWorld2D(m, cam);

        // Input
        if (IsKeyPressed(KEY_TAB)) {
            current_mode = (current_mode + 1) % MODE_COUNT;
        }
        if (IsKeyPressed(KEY_GRAVE)) {
            debug_draw = !debug_draw;
        }

		// Move camera
		if (IsKeyDown(KEY_LEFT_SHIFT) &&
			IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
			mpos_from = m_world;
		}
		if (IsKeyDown(KEY_LEFT_SHIFT) &&
			IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
			cam.target.x -= m_world.x - mpos_from.x;
			cam.target.y -= m_world.y - mpos_from.y;
		}

		// Zoom camera
		
		// Save entities
		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
			if (save_entities(&entities, entities_save_path)) {
				log_debug("Successfully saved entities to `%s`", entities_save_path);
			} else {
				log_debug("Failed to save entities to `%s`", entities_save_path);
			}
			
			/// DEBUG
			if (hovering_entity) {
				save_entity_to_file(hovering_entity, &temp_arena, "test.entity", ENTITY_SAVE_VERSION);
			}
		}
		// Load entities
		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L)) {
			if (load_entities(&entities, entities_save_path, &entity_arena, &temp_arena)) {
				log_debug("Successfully loaded entities from `%s`", entities_save_path);
			} else {
				log_debug("Failed to load entities from `%s`", entities_save_path);
			}
			/// DEBUG
			
			// Entity e = make_entity(&entities, m_world, ENTITY_DEFAULT_RADIUS, selected_entity_kind, &entity_arena, &temp_arena);
			// if (load_entity_from_file(&e, "test.entity")) {
			// add_entity(e);
			// }
		}

        // Mode-specific input
        switch (current_mode) {
            case MODE_NORMAL: {
                // Change Entity kind
                if (IsKeyPressed(KEY_E)) {
                    selected_entity_kind = (selected_entity_kind + 1) % EK_COUNT;
                }
                if (IsKeyPressed(KEY_Q)) {
                    if (selected_entity_kind==0)
                        selected_entity_kind = EK_COUNT-1;
                    else
                        selected_entity_kind--;
                }

                // Add Entity
                if (IsKeyPressed(KEY_SPACE)) {
					float64 tp1 = GetTime();
                    Entity e = make_entity(&entities, m_world, ENTITY_DEFAULT_RADIUS, selected_entity_kind, &entity_arena, &temp_arena);
					log_debug("make_entity() took %.2lfs", GetTime() - tp1);
					add_entity(e);
                }

                if (IsKeyPressed(KEY_D)) {
					if (IsKeyDown(KEY_LEFT_SHIFT)) {
						// Delete Selected entities
						for (int i = (int)entities.count-1; i >= 0; --i) {
							Entity *e = &entities.items[i];
							if (e->state & (1<<ESTATE_DEAD)) continue;
							if (e->state & (1<<ESTATE_SELECTED)) {
								free_entity(e);

								e->state |= (1<<ESTATE_DEAD);
								// arr_remove(entities, Entity, &d, (int)i);
								darr_append(free_entity_indices, i);
								ASSERT(entities_count > 0, "We cant remove if there are no entities");
								entities_count--;
							}
						}
					} else {
						// Disconnect connections of Selected entities
						for (int i = (int)entities.count-1; i >= 0; --i) {
							Entity *e = &entities.items[i];
							if (e->state & (1<<ESTATE_DEAD)) continue;
							if (e->state & (1<<ESTATE_SELECTED)) {
								disconnect_entity(e);
							}
						}
					}
                }

                // Select/Deselect all
                if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_A)) {
                    for (int i = (int)entities.count-1; i >= 0; --i) {
                        Entity *e = &entities.items[i];
						if (e->state & (1<<ESTATE_DEAD)) continue;
                        if (IsKeyDown(KEY_LEFT_SHIFT)) {
                            e->state &= ~(1<<ESTATE_SELECTED);
                        } else {
                            e->state |= (1<<ESTATE_SELECTED);
                        }
                    }
                }

                // Move selected entities
                if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) ||
                        IsKeyPressed(KEY_Z)) {
                    for (int i = (int)entities.count-1; i >= 0; --i) {
                        Entity *e = &entities.items[i];
						if (e->state & (1<<ESTATE_DEAD)) continue;
                        e->offset = Vector2Subtract(e->pos, m_world);
                    }
                }

                if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) ||
                        IsKeyDown(KEY_Z)) {
                    if (hovering_entity) {
                        hovering_entity->pos = Vector2Add(m_world, hovering_entity->offset);
                    } else {
                        for (int i = (int)entities.count-1; i >= 0; --i) {
                            Entity *e = &entities.items[i];
							if (e->state & (1<<ESTATE_DEAD)) continue;
                            if (e->state & (1<<ESTATE_SELECTED)) {
                                e->pos = Vector2Add(m_world, e->offset);
                            }
                        }
                    }
                }

				// Connect 
                if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) ||
                        IsKeyPressed(KEY_X)) {
                    if (hovering_entity) {
                        connecting_from = hovering_entity;
                        connecting_from->state |= (1<<ESTATE_CONNECTING_FROM);
                    }
                }

                if ((IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsKeyDown(KEY_X)) && connecting_from) {
                    if (hovering_entity && hovering_entity != connecting_from) {
                        connecting_to = hovering_entity;
                        if (connecting_to)
                            connecting_to->state |= (1<<ESTATE_CONNECTING_TO);
                    }
                }

                if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) ||
                        IsKeyReleased(KEY_X)) {
                    if (connecting_from && connecting_to) {
						connect_entity(&entities, connecting_from, connecting_to);
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
            for (int i = (int)entities.count-1; i >= 0; --i) {
                Entity *e = &entities.items[i];
				if (e->state & (1<<ESTATE_DEAD)) continue;
                float dist_sq = Vector2DistanceSqr(e->pos, m_world);
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
                        selection_start = m_world;
                        selection.x = m_world.x;
                        selection.y = m_world.y;
                    }
                    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                        if (m_world.x < selection_start.x) {
                            selection.x = m_world.x;
                            selection.width = selection_start.x - m_world.x;
                        } else {
                            selection.width = m_world.x - selection.x;
                        }
                        if (m_world.y < selection_start.y) {
                            selection.y = m_world.y;
                            selection.height = selection_start.y - m_world.y;
                        } else {
                            selection.height = m_world.y - selection.y;
                        }
                    }
                    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                        selecting = false;
                        for (int i = (int)entities.count-1; i >= 0; --i) {
                            Entity *e = &entities.items[i];
							if (e->state & (1<<ESTATE_DEAD)) continue;
                            if (rect_contains_point(selection, e->pos)) {
                                e->state |= (1<<ESTATE_SELECTED);
                            } else {
                                if (!IsKeyDown(KEY_LEFT_CONTROL))
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
			BeginMode2D(cam);
				for (int i = (int)entities.count-1; i >= 0; --i) {
					Entity *e = &entities.items[i];
					if (e->state & (1<<ESTATE_DEAD)) continue;
					draw_entity(e, debug_draw);
				}

				//// DEBUG: Draw mpos_from and m_world - mpos_from
				// DrawCircle(mpos_from.x, mpos_from.y, 8, RED);
				// if (IsKeyDown(KEY_LEFT_SHIFT) && IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
				// 	DrawLineV(m_world, mpos_from, WHITE);
				// }
			EndMode2D();

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
                    const char *hovering_id = arena_alloc_str(temp_arena, "Hovering ID: %zu", hovering_entity->id);
                    draw_text(GetFontDefault(), hovering_id, v2(2, y), ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                    y += ENTITY_DEFAULT_RADIUS*0.5 + 2;

                    const char *hovering_pos = arena_alloc_str(temp_arena, "Hovering ID: %.2f, %.2f", hovering_entity->pos.x, hovering_entity->pos.y);
                    draw_text(GetFontDefault(), hovering_pos, v2(2, y), ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                    y += ENTITY_DEFAULT_RADIUS*0.5 + 2;
                }
                draw_text(GetFontDefault(), connecting_from_str, v2(2, y), ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                y += ENTITY_DEFAULT_RADIUS*0.5 + 2;
                draw_text(GetFontDefault(), connecting_to_str, v2(2, y), ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                y += ENTITY_DEFAULT_RADIUS*0.5 + 2;

                y += ENTITY_DEFAULT_RADIUS*0.5 + 2;
                const char *entities_count_str = arena_alloc_str(temp_arena, "Entities count: %zu", entities_count);
                draw_text(GetFontDefault(), entities_count_str, v2(2, y), ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                y += ENTITY_DEFAULT_RADIUS*0.5 + 2;

				if (hovering_entity) {
					if (hovering_entity->kind == EK_NIC) {
						const char *dst_str = arena_alloc_str(temp_arena, "Hovering dst: %p", hovering_entity->nic->nic_entity);
						draw_text(GetFontDefault(), dst_str, v2(2, y), ENTITY_DEFAULT_RADIUS*0.5, WHITE);
						y += ENTITY_DEFAULT_RADIUS*0.5 + 2;

						const char *switch_str = arena_alloc_str(temp_arena, "Hovering switch: %p", hovering_entity->nic->switch_entity);
						draw_text(GetFontDefault(), switch_str, v2(2, y), ENTITY_DEFAULT_RADIUS*0.5, WHITE);
						y += ENTITY_DEFAULT_RADIUS*0.5 + 2;
					}
				}

                const char *e_arena_count_str = arena_alloc_str(temp_arena, "entity_arena.count: %zu", (size_t)((char*)entity_arena.ptr - (char*)entity_arena.buff));
                draw_text(GetFontDefault(), e_arena_count_str, v2(2, y), ENTITY_DEFAULT_RADIUS*0.5, RED);
                y += ENTITY_DEFAULT_RADIUS*0.5 + 2;

                const char *free_mac_count_str = arena_alloc_str(temp_arena, "Freed MacAddr count: %zu", free_mac_addresses.count);
                draw_text(GetFontDefault(), free_mac_count_str, v2(2, y), ENTITY_DEFAULT_RADIUS*0.5, RED);
                y += ENTITY_DEFAULT_RADIUS*0.5 + 2;
            }

            // Mode-specific Draw
            switch (current_mode) {
                case MODE_NORMAL: {
					const char *selected_entity_kind_str = arena_alloc_str(temp_arena, "Entity Kind: %s", entity_kind_as_str(selected_entity_kind));
					draw_text_aligned(GetFontDefault(), selected_entity_kind_str, v2(width*0.5, 2), ENTITY_DEFAULT_RADIUS*0.5, TEXT_ALIGN_V_TOP, TEXT_ALIGN_H_CENTER, WHITE);
					BeginMode2D(cam);
						if (connecting_from) {
							DrawLineBezier(connecting_from->pos, m_world, 1.0, GRAY);
						}

						if (selecting)
							DrawRectangleLinesEx(selection, 1.0, WHITE);
					EndMode2D();
                } break;
                case MODE_COUNT:
                default: ASSERT(false, "UNREACHABLE!");
            }

        EndTextureMode();
        draw_ren_tex(ren_tex, SCREEN_WIDTH, SCREEN_HEIGHT);
        EndDrawing();
    }

	cleanup();
    return 0;
}
