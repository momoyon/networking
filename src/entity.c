#include <entity.h>
#include <config.h>
#include <common.h>
#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

size_t entity_id_counter = 0;
Entity_ids free_entity_ids = {0};

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
        case EK_NONE: return "None";
        case EK_NETWORK_INTERFACE: return "Network Interface";
        case EK_SWITCH: return "Switch";
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
}

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
        case EK_NONE: {
        } break;
        case EK_NETWORK_INTERFACE: {
            e.network_interface = (Network_interface *)arena_alloc(arena, sizeof(Network_interface));
            e.network_interface->subnet_mask[0] = 255;
            e.network_interface->subnet_mask[1] = 255;
            e.network_interface->subnet_mask[2] = 255;
            e.network_interface->subnet_mask[3] = 0;
			e.network_interface->dst = NULL;
            get_unique_mac_address(e.network_interface->mac_address);
			e.tex = load_texture_checked("resources/gfx/network_interface.png");
			ASSERT(IsTextureReady(e.tex), "Failed to load network interface image!");
        } break;
        case EK_SWITCH: {
            e.switchh = (Switch *)arena_alloc(arena, sizeof(Switch));
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }

    return e;
}

static void draw_info_text(Vector2 *p, const char *text, int font_size, Color color) {
    draw_text(GetFontDefault(), text, *p, font_size, color);
    p->y += font_size + 2;
}

void draw_entity(Entity *e, bool debug) {
    switch (e->kind) {
        case EK_NONE: {
            DrawCircle(e->pos.x, e->pos.y, e->radius, RED);
        } break;
        case EK_NETWORK_INTERFACE: {
            ASSERT(e->network_interface, "We failed to allocate network_interface!");
            // DrawCircle(e->pos.x, e->pos.y, e->radius, BLUE);

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
			DrawTexturePro(e->tex, src, dst, CLITERAL(Vector2) { (ENTITY_DEFAULT_RADIUS/2)+1, (ENTITY_DEFAULT_RADIUS/2)+1 }, 0.0, WHITE); // Draw a part of a texture defined by a rectangle with 'pro' parameters
            if (e->state & (1<<ESTATE_SELECTED)) {
                Vector2 p = v2(e->pos.x + e->radius*1.5, e->pos.y + e->radius*1.5);
                DrawLineV(e->pos, p, WHITE);
                ASSERT(e->temp_arena, "BRUH");

                draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                            "ipv4: %d.%d.%d.%d",
                            e->network_interface->ipv4_address[0],
                            e->network_interface->ipv4_address[1],
                            e->network_interface->ipv4_address[2],
                            e->network_interface->ipv4_address[3]),
                        ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                            "subnet mask: %d.%d.%d.%d",
                            e->network_interface->subnet_mask[0],
                            e->network_interface->subnet_mask[1],
                            e->network_interface->subnet_mask[2],
                            e->network_interface->subnet_mask[3]),
                        ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                            "mac: %02X:%02X:%02X:%02X:%02X:%02X",
                            e->network_interface->mac_address[0],
                            e->network_interface->mac_address[1],
                            e->network_interface->mac_address[2],
                            e->network_interface->mac_address[3],
                            e->network_interface->mac_address[4],
                            e->network_interface->mac_address[5]), ENTITY_DEFAULT_RADIUS*0.5, WHITE);
            }
            if (e->network_interface->dst != NULL) {
				DrawLineBezier(e->pos, e->network_interface->dst->pos, 1.0, WHITE);
            }
        } break;
        case EK_SWITCH: {
            ASSERT(e->switchh, "We failed to allocate switch!");
            DrawCircle(e->pos.x, e->pos.y, e->radius, SKYBLUE);
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
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

void free_entity(Entity *e) {
    darr_append(free_entity_ids, e->id);
}

bool connect(Entities *entities, Entity *a, Entity *b) {
    ASSERT(entities, "Bro pass an array of entities!");

    ASSERT(a && b, "BRUH");

    if (a->kind != EK_NETWORK_INTERFACE ||
        b->kind != EK_NETWORK_INTERFACE) {
        log_debug("We can only connect network interfaces!");
        return false;
    }

	// Check if they are already connected to other nics
	if (a->network_interface->dst != NULL) {
		Entity *a_conn = a->network_interface->dst;
		// The other one is connected to this
		if (a_conn->network_interface->dst == a) {
			a_conn->network_interface->dst = NULL;
		}
	}

	if (b->network_interface->dst != NULL) {
		Entity *b_conn = b->network_interface->dst;
		// The other one is connected to this
		if (b_conn->network_interface->dst == b) {
			b_conn->network_interface->dst = NULL;
		}
	}

    ASSERT(a->network_interface && b->network_interface, "Fucked up");

    a->network_interface->dst = b;
    b->network_interface->dst = a;
    return true;
}
