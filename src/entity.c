#include <entity.h>
#include <config.h>
#include <common.h>
#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

#include <switch.h>

size_t entity_id_counter = 0;
Entity_ids free_entity_ids = {0};
size_t entities_count = 0;
Entities entities = {0};
Entity_indices free_entity_indices = {0};

static size_t get_unique_id(void) {
    if (free_entity_ids.count > 0) {
        int last_free_id = -1;
        da_remove(free_entity_ids, int, &last_free_id, (int)free_entity_ids.count-1);
        ASSERT(last_free_id != -1, "We failed to get last free id!");
        return last_free_id;
    } else {
        return entity_id_counter++;
    }
}

const char *entity_kind_as_str(const Entity_kind k) {
    switch (k) {
        case EK_NIC: return "NIC";
        case EK_SWITCH: return "Switch";
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
}

static void draw_info_text(Vector2 *p, const char *text, int font_size, Color color) {
    draw_text(GetFontDefault(), text, *p, font_size, color);
    p->y += font_size + 2;
}

void draw_entity(Entity *e, bool debug) {
    switch (e->kind) {
        case EK_NIC: {
            ASSERT(e->nic, "We failed to allocate nic!");
            // DrawCircle(e->pos.x, e->pos.y, e->radius, BLUE);

            if (e->state & (1<<ESTATE_SELECTED)) {
                Vector2 p = v2(e->pos.x + e->radius*1.5, e->pos.y + e->radius*1.5);
                DrawLineV(e->pos, p, WHITE);
                ASSERT(e->temp_arena, "BRUH");

                draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                            "ipv4: %d.%d.%d.%d",
                            e->nic->ipv4_address[0],
                            e->nic->ipv4_address[1],
                            e->nic->ipv4_address[2],
                            e->nic->ipv4_address[3]),
                        ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                            "subnet mask: %d.%d.%d.%d",
                            e->nic->subnet_mask[0],
                            e->nic->subnet_mask[1],
                            e->nic->subnet_mask[2],
                            e->nic->subnet_mask[3]),
                        ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                            "mac: %02X:%02X:%02X:%02X:%02X:%02X",
                            e->nic->mac_address[0],
                            e->nic->mac_address[1],
                            e->nic->mac_address[2],
                            e->nic->mac_address[3],
                            e->nic->mac_address[4],
                            e->nic->mac_address[5]), ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                            "switch: %p",
							e->nic->switch_entity),
						ENTITY_DEFAULT_RADIUS*0.5, WHITE);

            }

			// Draw connections
			if (e->nic->nic_entity) {
				// TODO: This is happen twice in a connection
				DrawLineBezier(e->pos, e->nic->nic_entity->pos, 1, WHITE);
			}

			if (e->nic->switch_entity) {
				DrawLineBezier(e->pos, e->nic->switch_entity->pos, 1, WHITE);
			}
        } break;
        case EK_SWITCH: {
            ASSERT(e->switchh, "We failed to allocate switch!");
            if (e->state & (1<<ESTATE_SELECTED)) {
                Vector2 p = v2(e->pos.x + e->radius*1.5, e->pos.y + e->radius*1.5);
                DrawLineV(e->pos, p, WHITE);
                ASSERT(e->temp_arena, "BRUH");

				for (size_t i = 0; i < e->switchh->nic_ptrs.count; ++i) {
					Nic *nic = e->switchh->nic_ptrs.items[i];
					if (nic == NULL) continue;
					draw_info_text(&p, arena_alloc_str(*e->temp_arena,
								"eth%zu: %d.%d.%d.%d (%p)", i, 
								nic->ipv4_address[0],
								nic->ipv4_address[1],
								nic->ipv4_address[2],
								nic->ipv4_address[3],
								nic),
							ENTITY_DEFAULT_RADIUS*0.5, WHITE);
				}
            }
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
	if (IsTextureReady(e->tex)) {
		Rectangle src = {
			.x = 0,
			.y = 0,
			.width = e->tex.width,
			.height = e->tex.height,
		};

		Rectangle dst = {
			.x = e->pos.x,
			.y = e->pos.y,
			.width  = e->tex.width*ENTITY_TEXTURE_SCALE,
			.height = e->tex.height*ENTITY_TEXTURE_SCALE,
		};
		DrawTexturePro(e->tex, src, dst, CLITERAL(Vector2) { (ENTITY_DEFAULT_RADIUS/2)+1, (ENTITY_DEFAULT_RADIUS/2)+1 }, 0.0, WHITE);
	} else {
		DrawCircle(e->pos.x, e->pos.y, e->radius, WHITE);
	}
    if (debug) draw_text_aligned(GetFontDefault(), entity_kind_as_str(e->kind), Vector2Subtract(e->pos, v2(0, ENTITY_DEFAULT_RADIUS*1.5)), ENTITY_DEFAULT_RADIUS * 0.5, TEXT_ALIGN_V_CENTER, TEXT_ALIGN_H_CENTER, WHITE);

    // Draw outline if selected
    if (e->state & (1<<ESTATE_SELECTED)) {
        DrawCircleLines(e->pos.x, e->pos.y, e->radius+2, WHITE);
    }
    if (e->state & (1<<ESTATE_HOVERING)) {
        DrawCircleLines(e->pos.x, e->pos.y, e->radius+4, GRAY);
    }

    ASSERT(!(e->state & (1<<ESTATE_CONNECTING_FROM) && e->state & (1<<ESTATE_CONNECTING_TO)), "I can't connect to myself");
    if (e->state & (1<<ESTATE_CONNECTING_FROM)) {
        DrawCircleLines(e->pos.x, e->pos.y, e->radius+4, RED);
    }

    if (e->state & (1<<ESTATE_CONNECTING_TO)) {
        DrawCircleLines(e->pos.x, e->pos.y, e->radius+4, GREEN);
    }
}

static bool connect_nic_to(Entity *nic, Entity *other) {
	if (nic->kind != EK_NIC) {
		log_debug("That isn't a NIC brotato _/\\_");
		return false;
	}

	switch (other->kind) {
		case EK_NIC: {
			Entity *a = nic;
			Entity *b = other;
			// Check if they are already connected to other nics
			if (a->nic->nic_entity != NULL) {
				Entity *a_conn = a->nic->nic_entity;
				// The other one is connected to this
				if (a_conn->nic->nic_entity == a) {
					a_conn->nic->nic_entity = NULL;
				}
			}

			if (b->nic->nic_entity != NULL) {
				Entity *b_conn = b->nic->nic_entity;
				// The other one is connected to this
				if (b_conn->nic->nic_entity == b) {
					b_conn->nic->nic_entity = NULL;
				}
			}

			a->nic->nic_entity = b;
			b->nic->nic_entity = a;
			return true;
	    } break;
		case EK_SWITCH: {
			ASSERT(other->switchh, "bo");

			if (nic->nic->switch_entity != NULL && nic->nic->switch_entity != other) {
				log_error("Please disconnect the nic from any other switch!");
				return false;
			}

			nic->nic->switch_entity = other;

			bool found = false;

			for (size_t i = 0; i < other->switchh->nic_ptrs.count; ++i) {
				Nic *nic_ptr = other->switchh->nic_ptrs.items[i];
				if (nic_ptr == nic->nic) {
					found = true;
					break;
				}
			}
			if (!found) { 
				arr_append(other->switchh->nic_ptrs, nic->nic);
			} else {
				log_debug("RAH");
			}
			return true;
	    } break;
		case EK_COUNT:
		default: ASSERT(false, "UNREACHABLE!");
	}

	return false;
}

static bool connect_switch_to(Entity *switchh, Entity *other) {
	if (switchh->kind != EK_SWITCH) {
		log_debug("That isn't a NIC brochacho _/\\_");
		return false;
	}

	switch (other->kind) {
		case EK_NIC: {
			return connect_nic_to(other, switchh); // We can do this 
	    } break;
		case EK_SWITCH: {
			log_debug("We can't connect two switched directly!");
			return false;
	    } break;
		case EK_COUNT:
		default: ASSERT(false, "UNREACHABLE!");
	}
	return false;
}

bool connect_entity(Entities *entities, Entity *a, Entity *b) {
    ASSERT(entities, "Bro pass an array of entities!");

    ASSERT(a && b, "BRUH");

	bool connected = false;

	switch (a->kind) {
		case EK_NIC: {
			connected = connect_nic_to(a, b);
	    } break;
		case EK_SWITCH: {
			connected = connect_switch_to(a, b);
			return false;
	    } break;
		case EK_COUNT:
		default: ASSERT(false, "UNREACHABLE!");
	}

	if (connected) {
		// Connection conn = {
		// 	.from = &a->pos,
		// 	.to   = &b->pos,
		// };
		// darr_append(connections, conn);
	}

    return true;
}

// Makers
Entity make_entity(Entities *entities, Vector2 pos, float radius, Entity_kind kind, Arena *arena, Arena *temp_arena) {
    Entity e = (Entity) {
        .pos = pos,
        .radius = radius,
        .kind = kind,
        .id = get_unique_id(),
        .state = 0,
        .arena = arena,
        .temp_arena = temp_arena,
        .entities = entities,
    };

    switch (kind) {
        case EK_NIC: {
            e.nic = (Nic *)arena_alloc(arena, sizeof(Nic));
			make_nic(e.nic, arena);
			e.tex = load_texture_checked("resources/gfx/nic.png");
			ASSERT(IsTextureReady(e.tex), "Failed to load network interface image!");
        } break;
        case EK_SWITCH: {
            e.switchh = (Switch *)arena_alloc(arena, sizeof(Switch));
			make_switch(e.switchh, arena, 4);
			e.tex = load_texture_checked("resources/gfx/switch.png");
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }

    return e;
}

void make_nic(Nic *nic, Arena *arena) {
	(void)arena;
	nic->subnet_mask[0] = 255;
	nic->subnet_mask[1] = 255;
	nic->subnet_mask[2] = 255;
	nic->subnet_mask[3] = 0;
	nic->nic_entity = NULL;
	nic->switch_entity = NULL;
	get_unique_mac_address(nic->mac_address);
}

void make_switch(Switch *switch_out, Arena *arena, size_t nic_count) {
	(void)arena;
	Switch s = {0};

	// TODO: Alloc this in the arena
	arr_heap_init(s.nic_ptrs, nic_count);

	memset(s.nic_ptrs.items, 0, sizeof(Nic*) * nic_count);

	log_debug("Switch.nic_ptrs.capacity: %zu | nic_count: %zu", s.nic_ptrs.capacity, nic_count);

	*switch_out = s;
}

// Disconnect-ers
void disconnect_entity(Entity *e) {
	switch (e->kind) {
		case EK_NIC: {
			disconnect_nic(e);
		} break;
		case EK_SWITCH: {
			disconnect_switch(e);
		} break;
		case EK_COUNT:
		default: ASSERT(false, "UNREACHABLE!");
	}
}
void disconnect_nic(Entity *e) {
	ASSERT(e->kind == EK_NIC, "BRO");
	if (e->nic->nic_entity && e->nic->nic_entity->nic->nic_entity == e) {
		e->nic->nic_entity->nic->nic_entity = NULL;
	}
	e->nic->nic_entity = NULL;
	if (e->nic->switch_entity) {
		for (size_t i = 0; i < e->nic->switch_entity->switchh->nic_ptrs.count; ++i) {
			Nic *nic_ptr = e->nic->switch_entity->switchh->nic_ptrs.items[i];
			if (nic_ptr == e->nic) {
				arr_delete(e->nic->switch_entity->switchh->nic_ptrs, Nic *, i);
				break; // We shouldn't have duplicate entries
			}
		}
	}
	e->nic->switch_entity = NULL;
}
void disconnect_switch(Entity *e) {
	ASSERT(e->kind == EK_SWITCH, "BRO");
	for (size_t i = 0; i < e->switchh->nic_ptrs.count; ++i) {
		Nic *nic_ptr = e->switchh->nic_ptrs.items[i];
		if (nic_ptr && nic_ptr->switch_entity == e) {
			nic_ptr->switch_entity = NULL;
		}
	}
	e->switchh->nic_ptrs.count = 0;
}

// Free-ers
void free_entity(Entity *e) {
    darr_append(free_entity_ids, e->id);

	switch (e->kind) {
		case EK_NIC: {
			free_nic(e);
		} break;
		case EK_SWITCH: {
			free_switch(e);
		} break;
		case EK_COUNT:
		default: ASSERT(false, "UNREACHABLE!");
	}
}

void free_nic(Entity *e) {
	ASSERT(e->kind == EK_NIC, "Br");
	if (e->nic->nic_entity != NULL) {
		Entity *e_conn = e->nic->nic_entity;
		if (e_conn->nic->nic_entity == e) {
			e_conn->nic->nic_entity = NULL;
		}
		e->nic->nic_entity = NULL;
	}

	if (e->nic->switch_entity != NULL) {
		Entity *e_switch = e->nic->switch_entity;
		for (size_t i = 0; i < e_switch->switchh->nic_ptrs.count; ++i) {
			Nic *nic_ptr = e_switch->switchh->nic_ptrs.items[i];
			if (nic_ptr == e->nic) {
				arr_delete(e_switch->switchh->nic_ptrs, Nic *, i);
				break; // We shouldn't have duplicate entries
			}
		}
	}
}

void free_switch(Entity *e) {
	ASSERT(e->kind == EK_SWITCH, "Br");

	// Remove any reference to this switch from the connected NICs
	for (size_t i = 0; i < e->switchh->nic_ptrs.count; ++i) {
		Nic *nic_ptr = e->switchh->nic_ptrs.items[i];
		if (nic_ptr && nic_ptr->switch_entity == e) {
			nic_ptr->switch_entity = NULL;
		}
	}
	arr_free(e->switchh->nic_ptrs);
}

// I/O
// NOTE: Yea so we can't just dump the entities array and load it directly from the filesystem cuz the entities have memebers alloced on Arena memory... :(
bool is_entities_saved(Entities *entities) {
	(void)entities;
	ASSERT(false, "is_entities_saved() is unimplemented!");
	return false;
}

const char *save_entity_to_data(Entity *e, Arena *temp_arena, int version) {
	char *s = arena_alloc_str(*temp_arena, "v%d %.2f %.2f %d %zu %d", version, e->pos.x, e->pos.y, e->kind, e->id, e->state);
	log_debug("BRO: %s", s);

	return s;
}

bool load_entity_from_data(Entity *e, const char *data, int version) {
	String_view sv = SV(data);

	String_view version_sv = sv_lpop_until_char(&sv, ' ');
	sv_lremove(&sv, 1); // Remove space

	int version = sv_to_int(sv_lremove(&version_sv, 1));

	switch (version) {
		case 1: return load_entity_from_data_v1(e, data);
		default: ASSERT(false, "UNREACHABLE!");
	}
}

bool load_entity_from_data_v1(Entity *e, const char *data) {

	if (!sv_equals(version_sv, SV("v1"))) {
		log_error("Version mismatch! Loading using: v1, data using: "SV_FMT, SV_ARG(version_sv));
		return false;
	}

	String_view pos_x_sv = sv_lpop_until_char(&sv, ' ');
	sv_lremove(&sv, 1); // Remove space
	String_view pos_y_sv = sv_lpop_until_char(&sv, ' ');
	sv_lremove(&sv, 1); // Remove space

	String_view kind_sv = sv_lpop_until_char(&sv, ' ');
	sv_lremove(&sv, 1); // Remove space

	String_view id_sv = sv_lpop_until_char(&sv, ' ');
	sv_lremove(&sv, 1); // Remove space

	String_view state_sv = sv_lpop_until_char(&sv, ' ');
	sv_lremove(&sv, 1); // Remove space

	if (sv.count > 0) {
		log_warning("Excess data remaining after parsing with version v0.1! (Excess %zu bytes)", sv.count);
	}
	return true;
}

bool load_entity_from_file(Entity *e, const char *filepath) {
	int file_size = -1;
	const char *file = read_file(filepath, &file_size);
	if (file_size == -1) {
		return false;
	}
	
	return load_entity_from_data(e, file);
}

bool save_entity_to_file(Entity *e, Arena *temp_arena, const char *filepath, int version) {
	FILE *f = fopen(filepath, "w");

	if (!f) { 
		return false;
	}

	const char *s = save_entity_to_data(e, temp_arena, version);
	size_t s_len = strlen(s);


	size_t wrote = fwrite(s, sizeof(char), s_len, f);
	if (wrote < s_len) {
		log_debug("Failed to fwrite!");
		fclose(f);
		return false;
	}

	temp_arena->ptr -= s_len; // Dealloc the string;

	fclose(f);

	return true;
}

