#include <entity.h>
#include <config.h>
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
        case EK_NETWORK_DEVICE: return "Network Device";
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
}

Entity make_entity(Vector2 pos, float radius, Entity_kind kind, Arena *arena, Arena *temp_arena) {
    Entity e = (Entity) {
        .pos = pos,
        .radius = radius,
        .kind = kind,
        .id = get_unique_id(),
        .state = 0,
        .arena = arena,
        .temp_arena = temp_arena,
    };

    switch (kind) {
        case EK_NONE: {
        } break;
        case EK_NETWORK_DEVICE: {
            e.network_device = (Network_device *)arena_alloc(arena, sizeof(Network_device));
            e.network_device->subnet_mask[0] = 255;
            e.network_device->subnet_mask[1] = 255;
            e.network_device->subnet_mask[2] = 255;
            e.network_device->subnet_mask[3] = 0;
            get_unique_mac_address(e.network_device->mac_address);
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
        case EK_NETWORK_DEVICE: {
            ASSERT(e->network_device, "We failed to allocate network_device!");
            DrawCircle(e->pos.x, e->pos.y, e->radius, BLUE);
            if (e->state & (1<<ESTATE_SELECTED)) {
                Vector2 p = v2(e->pos.x + e->radius*1.5, e->pos.y + e->radius*1.5);
                DrawLineV(e->pos, p, WHITE);
                ASSERT(e->temp_arena, "BRUH");

                draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                            "ipv4: %d.%d.%d.%d",
                            e->network_device->ipv4_address[0],
                            e->network_device->ipv4_address[1],
                            e->network_device->ipv4_address[2],
                            e->network_device->ipv4_address[3]),
                        ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                            "subnet mask: %d.%d.%d.%d",
                            e->network_device->subnet_mask[0],
                            e->network_device->subnet_mask[1],
                            e->network_device->subnet_mask[2],
                            e->network_device->subnet_mask[3]),
                        ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                            "mac: %02X:%02X:%02X:%02X:%02X:%02X",
                            e->network_device->mac_address[0],
                            e->network_device->mac_address[1],
                            e->network_device->mac_address[2],
                            e->network_device->mac_address[3],
                            e->network_device->mac_address[4],
                            e->network_device->mac_address[5]), ENTITY_DEFAULT_RADIUS*0.5, WHITE);
            }
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
    if (debug) draw_text_aligned(GetFontDefault(), entity_kind_as_str(e->kind), Vector2Subtract(e->pos, v2(0, ENTITY_DEFAULT_RADIUS*0.5)), ENTITY_DEFAULT_RADIUS * 0.5, TEXT_ALIGN_V_CENTER, TEXT_ALIGN_H_CENTER, WHITE);

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

    // Draw connections
    for (size_t i = 0; i < e->connections.count; ++i) {
        Connection *c = &e->connections.items[i];
        ASSERT(c->to, "We found a connection to NULL!");
        DrawLineBezier(e->pos, c->to->pos, 1.0, WHITE);
    }
}

void free_entity(Entity *e) {
    log_debug("Freed connections of Entity %zu", e->id);
    da_append(free_entity_ids, e->id);
    da_free(e->connections);
}

bool connect(Entity *from, Entity *to) {
    ASSERT(from, "Bruh pass a valid entity!");
    ASSERT(to,   "Bruh pass a valid entity!");

    for (size_t i = 0; i < from->connections.count; ++i) {
        Connection c = from->connections.items[i];
        if (c.to == to) {
            log_debug("We already connected these entities!");
            return false;
        }
    }

    Connection c = {
        .to = to
    };
    da_append(from->connections, c);

    for (size_t i = 0; i < to->connections.count; ++i) {
        Connection c = to->connections.items[i];
        if (c.to == to) {
            log_debug("We already connected these entities!");
            return false;
        }
    }

    c.to = from;
    da_append(to->connections, c);

    return true;
}
