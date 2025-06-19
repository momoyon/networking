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
        darr_remove(free_entity_ids, int, &last_free_id, (int)free_entity_ids.count-1);
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

static void init_entity(Entity *e, Arena *arena, Arena *temp_arena) {
	(void)temp_arena;
    switch (e->kind) {
        case EK_NIC: {
            e->nic = (Nic *)arena_alloc(arena, sizeof(Nic));
			make_nic(e->nic, arena);
			e->tex = load_texture_checked("resources/gfx/nic.png");
			ASSERT(IsTextureReady(e->tex), "Failed to load network interface image!");
        } break;
        case EK_SWITCH: {
            e->switchh = (Switch *)arena_alloc(arena, sizeof(Switch));
			make_switch(e->switchh, arena, 4);
			e->tex = load_texture_checked("resources/gfx/switch.png");
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
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

	init_entity(&e, arena, temp_arena);

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
	float64 tp1 = GetTime();
	get_unique_mac_address(nic->mac_address);
	log_debug("get_unique_mac_address() took %.2lfs", GetTime() - tp1);
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
	
	// Add free mac_address so it can be reused
	Mac_address m = {0};
	m.addr[0] = e->nic->mac_address[0];
	m.addr[1] = e->nic->mac_address[1];
	m.addr[2] = e->nic->mac_address[2];
	m.addr[3] = e->nic->mac_address[3];
	m.addr[4] = e->nic->mac_address[4];
	m.addr[5] = e->nic->mac_address[5];
	darr_append(free_mac_addresses, m);

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
	char *s = "";
	switch (version) {
		case 1: {
			s = arena_alloc_str(*temp_arena, "v1 %.2f %.2f %d %zu %d ", e->pos.x, e->pos.y, e->kind, e->id, e->state);
		} break;
		default: ASSERT(false, "UNREACHABLE!");
	}
	temp_arena->ptr--; // remove \0
	log_debug("BRO: %s", s);

	return s;
}

static bool load_entity_from_data_v1(Entity *e, String_view *sv) {
	String_view pos_x_sv = sv_lpop_until_char(sv, ' ');
	sv_lremove(sv, 1); // Remove space
	String_view pos_y_sv = sv_lpop_until_char(sv, ' ');
	sv_lremove(sv, 1); // Remove space

	String_view kind_sv = sv_lpop_until_char(sv, ' ');
	sv_lremove(sv, 1); // Remove space

	String_view id_sv = sv_lpop_until_char(sv, ' ');
	sv_lremove(sv, 1); // Remove space

	String_view state_sv = sv_lpop_until_char(sv, ' ');
	sv_lremove(sv, 1); // Remove space

	int pos_x_count = -1;
	float pos_x = sv_to_float(pos_x_sv, &pos_x_count);
	if (pos_x_count < 0) {
		log_error("pos.x: Failed to convert `"SV_FMT"` to float!", SV_ARG(pos_x_sv));
		return false;
	}
	int pos_y_count = -1;
	float pos_y = sv_to_float(pos_y_sv, &pos_y_count);
	if (pos_y_count < 0) {
		log_error("pos.y: Failed to convert `"SV_FMT"` to float!", SV_ARG(pos_y_sv));
		return false;
	}

	int kind_count = -1;
	int kind = sv_to_uint(kind_sv, &kind_count, 10);
	if (kind_count < 0) {
		log_error("kind: Failed to convert `"SV_FMT"` to uint!!", SV_ARG(kind_sv));
		return false;
	}

	int id_count = -1;
	int id = sv_to_uint(id_sv, &id_count, 10);
	if (id_count < 0) {
		log_error("id: Failed to convert `"SV_FMT"` to uint!!", SV_ARG(id_sv));
		return false;
	}

	int state_count = -1;
	int state = sv_to_int(state_sv, &state_count, 10);
	if (state_count < 0) {
		log_error("state: Failed to convert `"SV_FMT"` to int!!", SV_ARG(state_sv));
		return false;
	}

	log_debug("Loading %s[%d] @ %.2f %.2f", entity_kind_as_str(kind), id, pos_x, pos_y);

	e->pos.x = pos_x;
	e->pos.y = pos_y;
	e->kind = kind;
	e->id = id;
	e->state = state;

	return true;
}

bool load_entity_from_data(Entity *e, String_view *data_sv) {
	sv_ltrim(data_sv);
	String_view version_sv = sv_lpop_until_char(data_sv, ' ');
	sv_lremove(data_sv, 1); // Remove space
	sv_lremove(&version_sv, 1); // Remove v
	int version_count = -1;
	int version = sv_to_int(version_sv, &version_count, 10);
	if (version_count < 0) {
		log_error("Failed to convert `"SV_FMT"` to int!", SV_ARG(version_sv));
		return false;
	}

	switch (version) {
		case 1: return load_entity_from_data_v1(e, data_sv);
		default: {
			log_debug("Got version %d", version);
			ASSERT(false, "UNREACHABLE!");
	    } break;
	}
	return false;
}


bool load_entity_from_file(Entity *e, const char *filepath) {
	int file_size = -1;
	const char *file = read_file(filepath, &file_size);
	if (file_size == -1) {
		return false;
	}
	String_view sv = SV(file);
	
	if (!load_entity_from_data(e, &sv)) {
		return false;
	}

	free((void*)file);
	return true;
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

bool save_entities(Entities *entities, const char *filepath) {
	Arena entities_arena = arena_make(0);

	for (size_t i = 0; i < entities->count; ++i) {
		Entity *e = &entities->items[i];
		if (e->state & (1<<ESTATE_DEAD)) continue;

		if (!save_entity_to_data(e, &entities_arena, ENTITY_SAVE_VERSION)) {
			arena_free(&entities_arena);
			return false;
		}
	}

	int c = (int)((char*)entities_arena.ptr - (char*)entities_arena.buff);
	// log_debug("Alloced %zu bytes in entities_arena", c);
	log_debug("Entities: %.*s", c, (char*)entities_arena.buff);

	FILE *f = fopen(filepath, "wb");
	size_t wrote = fwrite(entities_arena.buff, sizeof(char), c, f);
	if (wrote < c) {
		log_error("Failed to write entities data to `%s`", filepath);
		fclose(f);
		return false;
	}
	fclose(f);

	arena_free(&entities_arena);
	return true;
}

// TODO: Loading will mess up the free_mac_address thing
bool load_entities(Entities *entities, const char *filepath, Arena *arena, Arena *temp_arena) {
	// Reset before loading new entities
	entities_count = 0;
	entities->count = 0;
	void *was = arena->ptr;
	arena_reset(arena);

	int file_size = -1;
	const char *file = read_file(filepath, &file_size);

	if (file_size == -1) {
		return false;
	}

	String_view sv = SV(file);

	while (sv.count > 0) {
		Entity e = make_entity(entities, v2xx(0), ENTITY_DEFAULT_RADIUS, EK_NIC, arena, temp_arena);
		if (!load_entity_from_data(&e, &sv)) {
			free((void*)file);
			return false;
		}
		init_entity(&e, arena, temp_arena);
		add_entity(e);
	}

	free((void*)file);
	return true;
}
