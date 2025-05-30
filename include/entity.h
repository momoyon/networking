#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <network_device.h>
#include <engine.h>
#include <connection.h>

typedef struct {
    Connection *items;
    size_t count;
    size_t capacity;
} Connections;

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
    ESTATE_CONNECTING_FROM,
    ESTATE_CONNECTING_TO,
    ESTATE_COUNT,
} Entity_state_mask;

struct Entity {
    Vector2 offset; // We use this to move with offset
    Vector2 pos;
    float radius;
    Entity_kind kind;
    size_t id;
    int state;

    Arena *arena; // All entity-related allocations
    Arena *temp_arena;

    Connections connections;

    // Kind specific
    Network_device *network_device;
};

Entity make_entity(Vector2 pos, float radius, Entity_kind kind, Arena *arena, Arena *temp_arena);
void draw_entity(Entity *e, bool debug);
void free_entity(Entity *e);

typedef struct {
    Entity *items;
    size_t count;
    size_t capacity;
} Entities;

bool connect(Entity *from, Entity *to);

#endif // _ENTITY_H_
