#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <network_device.h>
#include <engine.h>

typedef enum {
    EK_NONE,
    EK_NETWORK_DEVICE,
    EK_COUNT,
} Entity_kind;

const char *entity_kind_as_str(const Entity_kind k);

extern int entity_id_counter;

typedef enum {
    ESTATE_SELECTED,
    ESTATE_HOVERING,
    ESTATE_COUNT,
} Entity_state_mask;

typedef struct {
    Vector2 offset; // We use this to move with offset
    Vector2 pos;
    float radius;
    Entity_kind kind;
    size_t id;
    int state;

    Arena *arena; // All entity-related allocations
    Arena *temp_arena;

    // Kind specific
    Network_device *network_device;
} Entity;

Entity make_entity(Vector2 pos, float radius, Entity_kind kind, Arena *arena, Arena *temp_arena);
void draw_entity(Entity *e, bool debug);

typedef struct {
    Entity *items;
    size_t count;
    size_t capacity;
} Entities;

#endif // _ENTITY_H_
