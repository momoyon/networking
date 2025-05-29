#include <entity.h>
#include <config.h>
#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

int entity_id_counter = 0;

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
        .id = entity_id_counter++,
        .selected = false,
        .arena = arena,
        .temp_arena = temp_arena,
    };

    switch (kind) {
        case EK_NONE: {
        } break;
        case EK_NETWORK_DEVICE: {
            e.network_device = (Network_device *)arena_alloc(arena, sizeof(Network_device));
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
            if (e->selected) {
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
    if (debug) draw_text_aligned(GetFontDefault(), entity_kind_as_str(e->kind), e->pos, ENTITY_DEFAULT_RADIUS * 0.5, TEXT_ALIGN_V_CENTER, TEXT_ALIGN_H_CENTER, WHITE);

    // Draw outline if selected
    if (e->selected) {
        DrawCircleLines(e->pos.x, e->pos.y, e->radius+2, WHITE);

    }
}
