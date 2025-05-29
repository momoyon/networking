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

Entity make_entity(Vector2 pos, float radius, Entity_kind kind) {
    return (Entity) {
        .pos = pos,
        .radius = radius,
        .kind = kind,
        .id = entity_id_counter++,
    };
}

void draw_entity(Entity *e, bool debug) {
    switch (e->kind) {
        case EK_NONE: {
            DrawCircle(e->pos.x, e->pos.y, e->radius, RED);
        } break;
        case EK_NETWORK_DEVICE: {
            DrawCircle(e->pos.x, e->pos.y, e->radius, BLUE);
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
