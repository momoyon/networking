#include <entity.h>
#include <config.h>
#include <common.h>
#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

#include <switch.h>

#include <misc.h>

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

				Ipv4_class ipv4_class = determine_ipv4_class(e->nic->ipv4_address);
				const char *ipv4_class_info = ipv4_class_additional_info(ipv4_class);
                draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                            "ipv4: %d.%d.%d.%d (%s | %s) [%s]",
                            e->nic->ipv4_address[0],
                            e->nic->ipv4_address[1],
                            e->nic->ipv4_address[2],
                            e->nic->ipv4_address[3],
							ipv4_class_as_str(ipv4_class),
							ipv4_class_info,
							ipv4_type_as_str(determine_ipv4_type(e->nic->ipv4_address))),
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
				if (e->nic->drawing_connection) {
					DrawLineBezier(e->pos, e->nic->nic_entity->pos, 1, WHITE);
				}
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

				for (size_t i = 0; i < ARRAY_LEN(e->switchh->fa); ++i) {
					for (size_t j = 0; j < ARRAY_LEN(e->switchh->fa[i]); ++j) {
						Nic *nic = e->switchh->fa[i][j].nic;
						if (!nic) continue;
						draw_info_text(&p, arena_alloc_str(*e->temp_arena,
									"eth%zu/%zu: %d.%d.%d.%d | %d.%d.%d.%d", i, j,
									nic->ipv4_address[0],
									nic->ipv4_address[1],
									nic->ipv4_address[2],
									nic->ipv4_address[3],
									nic->subnet_mask[0],
									nic->subnet_mask[1],
									nic->subnet_mask[2],
									nic->subnet_mask[3]),
								ENTITY_DEFAULT_RADIUS*0.5, WHITE);
					}
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
			a->nic->drawing_connection = true;
			return true;
	    } break;
		case EK_SWITCH: {
			ASSERT(other->switchh, "bo");

			if (nic->nic->switch_entity != NULL && nic->nic->switch_entity != other) {
				log_error("Please disconnect the nic from any other switch!");
				return false;
			}


			bool found = false;

			for (size_t i = 0; i < ARRAY_LEN(other->switchh->fa); ++i) {
				for (size_t j = 0; j < ARRAY_LEN(other->switchh->fa[i]); ++j) {
					Nic *n = other->switchh->fa[i][j].nic;
					if (n == nic->nic) {
						found = true;
						break;
					}
				}
			}
			if (!found) { 
				if (!connect_to_next_free_port(nic, other)) {
					log_error("No free port available!");
					return false;
				}
			} else {
				log_debug("RAH");
			}
			nic->nic->switch_entity = other;
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
			make_nic(e, e->nic, arena);
			e->tex = load_texture_checked("resources/gfx/nic.png");
			ASSERT(IsTextureReady(e->tex), "Failed to load network interface image!");
			e->nic->nic_entity_id = -1;
        } break;
        case EK_SWITCH: {
            e->switchh = (Switch *)arena_alloc(arena, sizeof(Switch));
			make_switch(e->switchh, arena);
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

static bool is_mac_address_assigned(Entities *entities, uint8 *mac_address) {
	for (size_t i = 0; i < entities->count; ++i) {
		Entity *e = &entities->items[i];
		if (e->state & (1<<ESTATE_DEAD)) continue;

		if (e->kind == EK_NIC) {
			uint8 *nic_mac = e->nic->mac_address;
			if (memcmp(mac_address, nic_mac, 6) == 0) {
				return true;
			}
		}
	}
	return false;
}

void make_nic(Entity *e, Nic *nic, Arena *arena) {
	(void)arena;
	nic->ipv4_address[0] = 0;
	nic->ipv4_address[1] = 0;
	nic->ipv4_address[2] = 0;
	nic->ipv4_address[3] = 0;
	nic->subnet_mask[0] = 255;
	nic->subnet_mask[1] = 255;
	nic->subnet_mask[2] = 255;
	nic->subnet_mask[3] = 0;

	nic->id = e->id;
	// nic->self_entity = e;
	nic->nic_entity = NULL;
	nic->switch_entity = NULL;
	do {
		float64 tp1 = GetTime();
		get_unique_mac_address(nic->mac_address);
		log_debug("get_unique_mac_address() took %.2lfs", GetTime() - tp1);
	} while (is_mac_address_assigned(e->entities, nic->mac_address));
}

void make_switch(Switch *switch_out, Arena *arena) {
	Switch s = {0};

    make_switch_console(&s.console, arena);

	*switch_out = s;
}

void make_switch_console(Switch_console *console_out, Arena *arena) {
    (void)arena;

    Console_line l = {0};
    darr_append(console_out->lines, l);
}

bool input_to_console(Switch_console *console) {
	int ch = 0;
    Console_line *line = &console->lines.items[console->line];

    if (console->cursor < 0) console->cursor = 0;
    if (console->cursor > CONSOLE_LINE_BUFF_CAP-1) console->cursor = CONSOLE_LINE_BUFF_CAP-1;

	do {
		ch = GetCharPressed();

        if (IsKeyPressed(KEY_ENTER)) {
            log_debug("%s", line->buff);
            return true;
        }

        if (IsKeyPressed(KEY_BACKSPACE) ||
            IsKeyPressedRepeat(KEY_BACKSPACE)) {
            if (console->cursor > 0) {
                line->buff[--console->cursor] = '\0';
            }
        }

        if (line->count > CONSOLE_LINE_BUFF_CAP) {
            log_error("Exhausted line buff!");
            exit(1);
        }

        if (ch > 0) {
            log_debug("TYPED %c AT %d:%d", (char)ch, console->line, console->cursor);
            line->buff[console->cursor++] = (char)ch;
        }

        
	} while (ch > 0);

    return false;
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
		for (size_t i = 0; i < ARRAY_LEN(e->switchh->fa); ++i) {
			for (size_t j = 0; j < ARRAY_LEN(e->switchh->fa[i]); ++j) {
				Port *port = &e->nic->switch_entity->switchh->fa[i][j];
				if (port->nic && port->nic == e->nic) {
					port->nic = NULL;
					break; // We shouldn't have duplicate entries
				}
			}
		}
	}
	e->nic->switch_entity = NULL;
}

void disconnect_switch(Entity *e) {
	ASSERT(e->kind == EK_SWITCH, "BRO");
	for (size_t i = 0; i < ARRAY_LEN(e->switchh->fa); ++i) {
		for (size_t j = 0; j < ARRAY_LEN(e->switchh->fa[i]); ++j) {
			Port *port = &e->switchh->fa[i][j];
			if (port->nic) {
				if (port->nic->switch_entity == e) {
					port->nic->switch_entity = NULL;
				}
				port->nic = NULL;
			}
		}
	}
	log_debug("Disconnected switch with ID: %zu", e->id);
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
		for (size_t i = 0; i < ARRAY_LEN(e->switchh->fa); ++i) {
			for (size_t j = 0; j < ARRAY_LEN(e->switchh->fa[i]); ++j) {
				Nic *nic = e_switch->switchh->fa[i][j].nic;
				if (nic == e->nic) {
					nic->switch_entity = NULL;
				}
			}
		}
	}
}

void free_switch(Entity *e) {
	ASSERT(e->kind == EK_SWITCH, "Br");

	// Remove any reference to this switch from the connected NICs
	for (size_t i = 0; i < ARRAY_LEN(e->switchh->fa); ++i) {
		for (size_t j = 0; j < ARRAY_LEN(e->switchh->fa[i]); ++j) {
			Nic *nic = e->switchh->fa[i][j].nic;
			if (nic && nic->switch_entity == e) {
				nic->switch_entity = NULL;
			}
		}
	}
}

// Data-transfer
bool send_arp_ethernet_frame(Entity *dst, Entity *src) {
	Ethernet_frame eframe = {
		.ether_type_or_length = ETHER_TYPE_ARP,
		.payload = (uint8 *)"ARP PING TEST",
		.crc = 0,
	};

	memcpy(eframe.dst, dst->nic->mac_address, sizeof(uint8)*6);
	memcpy(eframe.src, src->nic->mac_address, sizeof(uint8)*6);

	return recieve(dst, src, eframe);
}

bool recieve(Entity *dst, Entity *src, Ethernet_frame frame) {
	(void)src;
	switch (dst->kind) {
		case EK_NIC: {
			if (dst->nic->nic_entity != src) {
				log_error("The dst NIC is not connected to the src NIC!");
				return false;
			}

			if (memcmp(frame.dst, dst->nic->mac_address, sizeof(uint8)*6) != 0) {
				log_warning("Ethernet Frame destined for "MAC_FMT" is dropped at src's connected NIC "MAC_FMT, MAC_ARG(frame.dst), MAC_ARG(dst->nic->mac_address));
				return false;
			}
			log_debug("Received Ethernet Frame from "MAC_FMT" to "MAC_FMT, MAC_ARG(frame.src), MAC_ARG(frame.dst));
			return true;
		} break;
		case EK_SWITCH: {
			ASSERT(false, "UNIMPLEMENTED!");
		} break;
		case EK_COUNT:
		default: ASSERT(false, "UNREACHABLE!");
	}
}

Entity *get_entity_ptr_by_id(Entities *entities, int id) {
	for (size_t i = 0; i < entities->count; ++i) {
		Entity *e = &entities->items[i];
		if (e->id == id) {
			return e;
		}
	}

	return NULL;
}

// I/O
// NOTE: Yea so we can't just dump the entities array and load it directly from the filesystem cuz the entities have memebers alloced on Arena memory... :(
bool is_entities_saved(Entities *entities) {
	(void)entities;
	ASSERT(false, "is_entities_saved() is unimplemented!");
	return false;
}

const char *entity_kind_save_format(Entity *e, Arena *temp_arena) {
	switch (e->kind) {
		case EK_NIC: {
			return arena_alloc_str(*temp_arena, "%d.%d.%d.%d %d.%d.%d.%d %d.%d.%d.%d.%d.%d", 
					e->nic->ipv4_address[0],
					e->nic->ipv4_address[1],
					e->nic->ipv4_address[2],
					e->nic->ipv4_address[3],
					e->nic->subnet_mask[0],
					e->nic->subnet_mask[1],
					e->nic->subnet_mask[2],
					e->nic->subnet_mask[3],
					e->nic->mac_address[0],
					e->nic->mac_address[1],
					e->nic->mac_address[2],
					e->nic->mac_address[3],
					e->nic->mac_address[4],
					e->nic->mac_address[5]);
		} break;
		case EK_SWITCH: {
			const char *res = (const char *)temp_arena->ptr;
			for (size_t i = 0; i < ARRAY_LEN(e->switchh->fa); ++i) {
				for (size_t j = 0; j < ARRAY_LEN(e->switchh->fa[i]); ++j) {
					Port *port = &e->switchh->fa[i][j];
					arena_alloc_str(*temp_arena, "%zu/%zu: %d ", i, j, (port->nic ? (int)(port->nic->id) : -1));
					temp_arena->ptr--;

					if (port->nic) {
						log_debug("Port %zu/%zu nic entity: (%d)", i, j, port->nic->id);
					}
				}
			}
			arena_alloc_str(*temp_arena, "%s", "|");
			log_debug("SWITCH KIND SAVE FMT: %s", res);
			return res;
		} break;
		case EK_COUNT:
		default: ASSERT(false, "UNREACHABLE!");
	}
	return "NOPE";
}

const char *save_entity_to_data(Entity *e, Arena *arena, Arena *temp_arena, int version) {
	char *s = "";
	switch (version) {
		case 1: {
			s = arena_alloc_str(*arena, "v%d %.2f %.2f %d %zu %d ", version, e->pos.x, e->pos.y, e->kind, e->id, e->state);
		} break;
		case 2: {
			s = arena_alloc_str(*arena, "v%d %.2f %.2f %d %zu %d %s ", version, e->pos.x, e->pos.y, e->kind, e->id, e->state, entity_kind_save_format(e, temp_arena));
		} break;
		case 3: {
			s = arena_alloc_str(*arena, "v%d %.2f %.2f %d %zu %d %s %d ",
					version, e->pos.x, e->pos.y, e->kind, e->id, e->state, entity_kind_save_format(e, temp_arena), e->nic && e->nic->nic_entity ? (int)e->nic->nic_entity->id : -1);
		} break;
		default: ASSERT(false, "UNREACHABLE!");
	}
	arena->ptr--; // remove \0
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
		log_error("state: Failed to convert state `"SV_FMT"` to int!!", SV_ARG(state_sv));
		return false;
	}

	log_debug("--------------------------------------------------");
	log_debug("Loading %s[%d] @ %.2f %.2f", entity_kind_as_str(kind), id, pos_x, pos_y);
	log_debug("--------------------------------------------------");

	e->pos.x = pos_x;
	e->pos.y = pos_y;
	e->kind = kind;
	e->id = id;
	e->state = state;

	init_entity(e, e->arena, e->temp_arena);

	return true;
}

static bool parse_nic_from_data(Nic *nic, String_view *sv) {
	sv_ltrim(sv);
	String_view ipv4_oct1_sv = sv_lpop_until_char(sv, '.');
	sv_lremove(sv, 1); // Remove .
	String_view ipv4_oct2_sv = sv_lpop_until_char(sv, '.');
	sv_lremove(sv, 1); // Remove .
	String_view ipv4_oct3_sv = sv_lpop_until_char(sv, '.');
	sv_lremove(sv, 1); // Remove .
	String_view ipv4_oct4_sv = sv_lpop_until_char(sv, ' ');
	sv_lremove(sv, 1); // Remove SPACE

	String_view subnet_mask_oct1_sv = sv_lpop_until_char(sv, '.');
	sv_lremove(sv, 1); // Remove .
					   //
	String_view subnet_mask_oct2_sv = sv_lpop_until_char(sv, '.');
	sv_lremove(sv, 1); // Remove .
	String_view subnet_mask_oct3_sv = sv_lpop_until_char(sv, '.');
	sv_lremove(sv, 1); // Remove .
	String_view subnet_mask_oct4_sv = sv_lpop_until_char(sv, ' ');
	sv_lremove(sv, 1); // Remove SPACE

	String_view mac_address_oct1_sv = sv_lpop_until_char(sv, '.');
	sv_lremove(sv, 1); // Remove .
	String_view mac_address_oct2_sv = sv_lpop_until_char(sv, '.');
	sv_lremove(sv, 1); // Remove .
	String_view mac_address_oct3_sv = sv_lpop_until_char(sv, '.');
	sv_lremove(sv, 1); // Remove .
	String_view mac_address_oct4_sv = sv_lpop_until_char(sv, '.');
	sv_lremove(sv, 1); // Remove .
	String_view mac_address_oct5_sv = sv_lpop_until_char(sv, '.');
	sv_lremove(sv, 1); // Remove .
	String_view mac_address_oct6_sv = sv_lpop_until_char(sv, ' ');
	sv_lremove(sv, 1); // Remove SPACE

	uint8 ipv4[4] = {0};
	int   ipv4_counts[4] = { -1, -1, -1, -1 };
	ipv4[0] = sv_to_uint(ipv4_oct1_sv, &ipv4_counts[0], 10);
	if (ipv4_counts[0] <= 0) {
		log_debug("Failed to convert ipv4 oct1`"SV_FMT"` to int!", SV_ARG(ipv4_oct1_sv));
		return false;
	}
	ipv4[1] = sv_to_uint(ipv4_oct2_sv, &ipv4_counts[1], 10);
	if (ipv4_counts[1] <= 0) {
		log_debug("Failed to convert ipv4 oct2`"SV_FMT"` to int!", SV_ARG(ipv4_oct2_sv));
		return false;
	}
	ipv4[2] = sv_to_uint(ipv4_oct3_sv, &ipv4_counts[2], 10);
	if (ipv4_counts[2] <= 0) {
		log_debug("Failed to convert ipv4 oct3`"SV_FMT"` to int!", SV_ARG(ipv4_oct3_sv));
		return false;
	}
	ipv4[3] = sv_to_uint(ipv4_oct4_sv, &ipv4_counts[3], 10);
	if (ipv4_counts[3] <= 0) {
		log_debug("Failed to convert ipv4 oct4`"SV_FMT"` to int!", SV_ARG(ipv4_oct4_sv));
		return false;
	}

	log_debug("--------------------------------------------------");
	log_debug("Parsed ipv4: %d.%d.%d.%d", ipv4[0], ipv4[1], ipv4[2], ipv4[3]);

	uint8 subnet_mask[4] = {0};
	int   subnet_mask_counts[4] = { -1, -1, -1, -1 };
	subnet_mask[0] = sv_to_uint(subnet_mask_oct1_sv, &subnet_mask_counts[0], 10);
	if (subnet_mask_counts[0] <= 0) {
		log_debug("Failed to convert subnet_mask oct1`"SV_FMT"` to int!", SV_ARG(subnet_mask_oct1_sv));
		return false;
	}
	subnet_mask[1] = sv_to_uint(subnet_mask_oct2_sv, &subnet_mask_counts[1], 10);
	if (subnet_mask_counts[1] <= 0) {
		log_debug("Failed to convert subnet_mask oct2`"SV_FMT"` to int!", SV_ARG(subnet_mask_oct2_sv));
		return false;
	}
	subnet_mask[2] = sv_to_uint(subnet_mask_oct3_sv, &subnet_mask_counts[2], 10);
	if (subnet_mask_counts[2] <= 0) {
		log_debug("Failed to convert subnet_mask oct3`"SV_FMT"` to int!", SV_ARG(subnet_mask_oct3_sv));
		return false;
	}
	subnet_mask[3] = sv_to_uint(subnet_mask_oct4_sv, &subnet_mask_counts[3], 10);
	if (subnet_mask_counts[3] <= 0) {
		log_debug("Failed to convert subnet_mask oct4`"SV_FMT"` to int!", SV_ARG(subnet_mask_oct4_sv));
		return false;
	}

	log_debug("Parsed subnet_mask: %d.%d.%d.%d", subnet_mask[0], subnet_mask[1], subnet_mask[2], subnet_mask[3]);

	uint8 mac_address[6] = {0};
	int   mac_address_counts[6] = { -1, -1, -1, -1, -1, -1 };
	mac_address[0] = sv_to_uint(mac_address_oct1_sv, &mac_address_counts[0], 10);
	if (mac_address_counts[0] <= 0) {
		log_debug("Failed to convert mac_address oct1`"SV_FMT"` to int!", SV_ARG(mac_address_oct1_sv));
		return false;
	}
	mac_address[1] = sv_to_uint(mac_address_oct2_sv, &mac_address_counts[1], 10);
	if (mac_address_counts[1] <= 0) {
		log_debug("Failed to convert mac_address oct2`"SV_FMT"` to int!", SV_ARG(mac_address_oct2_sv));
		return false;
	}
	mac_address[2] = sv_to_uint(mac_address_oct3_sv, &mac_address_counts[2], 10);
	if (mac_address_counts[2] <= 0) {
		log_debug("Failed to convert mac_address oct3`"SV_FMT"` to int!", SV_ARG(mac_address_oct3_sv));
		return false;
	}
	mac_address[3] = sv_to_uint(mac_address_oct4_sv, &mac_address_counts[3], 10);
	if (mac_address_counts[3] <= 0) {
		log_debug("Failed to convert mac_address oct4`"SV_FMT"` to int!", SV_ARG(mac_address_oct4_sv));
		return false;
	}
	mac_address[4] = sv_to_uint(mac_address_oct5_sv, &mac_address_counts[4], 10);
	if (mac_address_counts[4] <= 0) {
		log_debug("Failed to convert mac_address oct5`"SV_FMT"` to int!", SV_ARG(mac_address_oct5_sv));
		return false;
	}
	mac_address[5] = sv_to_uint(mac_address_oct6_sv, &mac_address_counts[5], 10);
	if (mac_address_counts[5] <= 0) {
		log_debug("Failed to convert mac_address oct6`"SV_FMT"` to int!", SV_ARG(mac_address_oct6_sv));
		return false;
	}


	log_debug("Parsed mac_address: %d.%d.%d.%d.%d.%d", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
	log_debug("--------------------------------------------------");

	sv_ltrim(sv);

	memcpy(nic->ipv4_address, ipv4, sizeof(uint8) * 4);
	memcpy(nic->subnet_mask, subnet_mask, sizeof(uint8) * 4);
	memcpy(nic->mac_address, mac_address, sizeof(uint8) * 6);

	return true;
}

static bool load_entity_from_data_v2(Entity *e, String_view *sv) {
	if (!load_entity_from_data_v1(e, sv)) {
		return false;
	}

	switch (e->kind) {
		case EK_NIC: {
			if (!parse_nic_from_data(e->nic, sv)) return false;
			return true;
		} break;
		case EK_SWITCH: {
			if (e->switchh == NULL) {
				log_error("Please allocate the switch before trying to load from data!");
				ASSERT(false, "DEBUG");
				return false;
			}

			log_debug("Parsing Switch...");

			sv_ltrim(sv);

			String_view switch_sv = sv_lpop_until_char(sv, '|');
			sv_lremove(sv, 1); // Remove |

			while (switch_sv.count > 0) {
				int i = -1;
				int j = -1;

				if (!parse_i_j_from_sv(&switch_sv, &i, &j)) {
					return false;
				}
				sv_ltrim(&switch_sv);

				String_view port_nic_id_sv = sv_lpop_until_char(&switch_sv, ' ');
				sv_ltrim(&switch_sv);

				int port_nic_id_count = -1;
				int port_nic_id = sv_to_int(port_nic_id_sv, &port_nic_id_count, 10);
				if (port_nic_id_count < 0) {
					log_error("Failed to convert port nic id `"SV_FMT"` to int!", SV_ARG(port_nic_id_sv));
					return false;
				}
				log_debug("Parsed port %d/%d: %d", i, j, port_nic_id);
				if (i < 0 || i > ARRAY_LEN(e->switchh->fa)-1) {
					log_error("Failed to parse switch fmt: i is outofbounds: %d (0 ~ %zu)", i, ARRAY_LEN(e->switchh->fa));
				}
				if (j < 0 || j > ARRAY_LEN(e->switchh->fa[0])-1) {
					log_error("Failed to parse switch fmt: j is outofbounds: %d (0 ~ %zu)", j, ARRAY_LEN(e->switchh->fa[0]));
				}
				e->switchh->fa[i][j].nic_id = port_nic_id;
			}
			return true;
		} break;
		case EK_COUNT:
		default: ASSERT(false, "UNREACHABLE!");
	}
	return false;
}

static bool load_entity_from_data_v3(Entity *e, String_view *sv) {
	if (!load_entity_from_data_v2(e, sv)) {
		return false;
	}

	sv_ltrim(sv);
	String_view nic_id_sv = sv_lpop_until_char(sv, ' ');
	sv_ltrim(sv);

	int nic_id_count = -1;
	int nic_id = sv_to_int(nic_id_sv, &nic_id_count, 10);
	if (nic_id_count < 0) {
		log_error("Failed to convert nic id `"SV_FMT"` to int!", SV_ARG(nic_id_sv));
		return false;
	}

	log_debug("Parsed nic_entity_id %d", nic_id);

	ASSERT(e->nic, "We should have parsed and allocated nic in v2 or v1");
	e->nic->nic_entity_id = nic_id;

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
		log_error("Failed to convert version `"SV_FMT"` to int!", SV_ARG(version_sv));
		return false;
	}

	switch (version) {
		case 1: return load_entity_from_data_v1(e, data_sv);
		case 2: return load_entity_from_data_v2(e, data_sv);
		case 3: return load_entity_from_data_v3(e, data_sv);
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

	Arena a = arena_make(0);
	const char *s = save_entity_to_data(e, temp_arena, &a, version);
	size_t s_len = strlen(s);

	size_t wrote = fwrite(s, sizeof(char), s_len, f);
	if (wrote < s_len) {
		log_debug("Failed to fwrite!");
		fclose(f);
		return false;
	}

	temp_arena->ptr -= s_len; // Dealloc the string;

	fclose(f);

	arena_free(&a);

	return true;
}

bool save_entities(Entities *entities, const char *filepath, size_t save_version) {
	Arena entities_arena = arena_make(0);
	Arena temp_arena = arena_make(0);

	for (size_t i = 0; i < entities->count; ++i) {
		Entity *e = &entities->items[i];
		if (e->state & (1<<ESTATE_DEAD)) continue;

		if (!save_entity_to_data(e, &entities_arena, &temp_arena, save_version)) {
			arena_free(&entities_arena);
			arena_free(&temp_arena);
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
	arena_free(&temp_arena);
	return true;
}

bool load_entities(Entities *entities, const char *filepath, Arena *arena, Arena *temp_arena) {
	// Reset before loading new entities
	entities_count = 0;
	entities->count = 0;
	arena_reset(arena);

	int file_size = -1;
	const char *file = read_file(filepath, &file_size);

	if (file_size == -1) {
		return false;
	}

	String_view sv = SV(file);

	while (sv.count > 0) {
		Entity e = make_entity(entities, v2xx(0), ENTITY_DEFAULT_RADIUS, EK_NIC, arena, temp_arena);
		sv_trim(&sv);
		if (!load_entity_from_data(&e, &sv)) {
			free((void*)file);
			return false;
		}
		// init_entity(&e, arena, temp_arena);
		add_entity(e);
	}

	log_debug("Entities.count after loading: %zu", entities->count);

	// Assign the nic pointers to the switch ports using the parsed nic_id
	for (size_t i = 0; i < entities->count; ++i) {
		Entity *e = &entities->items[i];
		if (e->kind == EK_SWITCH) {
			for (size_t i = 0; i < ARRAY_LEN(e->switchh->fa); ++i) {
				for (size_t j = 0; j < ARRAY_LEN(e->switchh->fa[i]); ++j) {
					Port *port = &e->switchh->fa[i][j];
					if (port->nic_id >= 0) {
						Entity *port_nic_e = get_entity_ptr_by_id(entities, port->nic_id);
						if (port_nic_e == NULL) {
							log_error("Cannot find NIC with id %d", port->nic_id);
							return false;
						}
						port->nic = port_nic_e->nic;
						port_nic_e->nic->switch_entity = e;
					}
				}
			}
		}
	}

	// Assign the nic pointers to the nic_entity using the parsed nic_id
	for (size_t i = 0; i < entities->count; ++i) {
		Entity *e = &entities->items[i];
		if (e->kind == EK_NIC) {
			if (e->nic->nic_entity_id >= 0) {
				Entity *nic_entity = get_entity_ptr_by_id(entities, e->nic->nic_entity_id);
				if (!e) {
					log_error("Failed to get entity from nic_id %d that NIC %zu was connected to...", e->nic->nic_entity_id, e->id);
					return false;
				}
				e->nic->nic_entity = nic_entity;
			}
		}
	}

	free((void*)file);
	return true;
}

static bool four_octect_from_input(uint8 *four_octect, char *chars_buff, size_t *chars_buff_count, size_t chars_buff_cap) {
	// if (*chars_buff_count == 1) {
	// 	(*chars_buff_count)--;
	// }
	int ch = 0;
	do {
		ch = GetCharPressed();
		if (IsKeyPressed(KEY_ENTER)) {
			String_view four_octect_sv = (String_view) {
				.data  = chars_buff,
				.count = *chars_buff_count,
			};
			// TODO: Refactor to a func
			String_view oct1_sv = sv_lpop_until_char(&four_octect_sv, '.');
			int oct1_count = -1;
			uint oct1 = sv_to_uint(oct1_sv, &oct1_count, 10);
			if (oct1_count < 0) {
				log_error("Failed to convert `"SV_FMT"` to a number!", SV_ARG(oct1_sv));
				return false;
			}
			if (oct1 > 255) {
				log_error("Octets must be in the range 0-255!");
				return false;
			}
			sv_lremove(&four_octect_sv, 1); // Remove .
			String_view oct2_sv = sv_lpop_until_char(&four_octect_sv, '.');
			int oct2_count = -1;
			uint oct2 = sv_to_uint(oct2_sv, &oct2_count, 10);
			if (oct2_count < 0) {
				log_error("Failed to convert `"SV_FMT"` to a number!", SV_ARG(oct2_sv));
				return false;
			}
			if (oct1 > 255) {
				log_error("Octets must be in the range 0-255!");
				return false;
			}
			sv_lremove(&four_octect_sv, 1); // Remove .
			String_view oct3_sv = sv_lpop_until_char(&four_octect_sv, '.');
			int oct3_count = -1;
			uint oct3 = sv_to_uint(oct3_sv, &oct3_count, 10);
			if (oct3_count < 0) {
				log_error("Failed to convert `"SV_FMT"` to a number!", SV_ARG(oct3_sv));
				return false;
			}
			if (oct1 > 255) {
				log_error("Octets must be in the range 0-255!");
				return false;
			}
			sv_lremove(&four_octect_sv, 1); // Remove .
			String_view oct4_sv = four_octect_sv;
			int oct4_count = -1;
			uint oct4 = sv_to_uint(oct4_sv, &oct4_count, 10);
			if (oct4_count < 0) {
				log_error("Failed to convert `"SV_FMT"` to a number!", SV_ARG(oct4_sv));
				return false;
			}
			if (oct1 > 255) {
				log_error("Octets must be in the range 0-255!");
				return false;
			}

			four_octect[0] = oct1;
			four_octect[1] = oct2;
			four_octect[2] = oct3;
			four_octect[3] = oct4;

			*chars_buff_count = 0;
			return true;
		}

		if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && (*chars_buff_count) > 0) {
			(*chars_buff_count)--;
		}
		if (('0' <= ch && ch <= '9') || ch == '.') {
			if (*chars_buff_count >= chars_buff_cap) {
				log_error("Exhausted chars buff!");
				exit(1);
			}
			chars_buff[(*chars_buff_count)++] = (char)ch;
			// log_debug("CHAR: %c(%d)", (char)ch, ch);
		}
	} while (ch > 0);
	return false;
	// log_debug("TYPED %zu chars!", chars_count);

}

bool ipv4_from_input(Entity *e, char *chars_buff, size_t *chars_buff_count, size_t chars_buff_cap) {
	if (e->kind != EK_NIC) {
		log_debug("Can only change the ipv4 of a NIC!");
		return true;
	}

	return four_octect_from_input(e->nic->ipv4_address, chars_buff, chars_buff_count, chars_buff_cap);
}

bool subnet_mask_from_input(Entity *e, char *chars_buff, size_t *chars_buff_count, size_t chars_buff_cap) {
	if (e->kind != EK_NIC) {
		log_debug("Can only change the ipv4 of a NIC!");
		return true;
	}

	return four_octect_from_input(e->nic->subnet_mask, chars_buff, chars_buff_count, chars_buff_cap);
}

bool connect_to_next_free_port(Entity *nic_e, Entity *switch_e) {

	if (!nic_e || nic_e->kind != EK_NIC) {
		log_error("Cannot connect to port: the NIC is not valid!");
		return false;
	}
	if (!switch_e || switch_e->kind != EK_SWITCH) {
		log_error("Cannot connect to port: the Switch is not valid!");
		return false;
	}

	for (size_t i = 0; i < ARRAY_LEN(switch_e->switchh->fa); ++i) {
		for (size_t j = 0; j < ARRAY_LEN(switch_e->switchh->fa[i]); ++j) {
			Port *port = &switch_e->switchh->fa[i][j];
			if (port->nic == NULL) {
				port->nic = nic_e->nic;
				return true;
			}
		}
	}
	return false;
}
