#include <entity.h>
#include <config.h>
#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

int entity_id_counter = 0;

const char *entity_kind_as_str(const Entity_kind k) {
    switch (k) {
        case EK_NONE: return "None";
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
    return "You should not see this!";
}

Entity make_entity(Vector2 pos, Entity_kind kind) {
    return (Entity) {
        .pos = pos,
        .kind = kind,
        .id = entity_id_counter++,
    };
}

void draw_entity(Entity *e, bool debug) {
    switch (e->kind) {
        case EK_NONE: {
            DrawCircle(e->pos.x, e->pos.y, ENTITY_DEFAULT_SIZE, RED);
            if (debug) draw_text_aligned(GetFontDefault(), "Entity", e->pos, ENTITY_DEFAULT_SIZE * 0.5, TEXT_ALIGN_V_CENTER, TEXT_ALIGN_H_CENTER, WHITE);
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
}
