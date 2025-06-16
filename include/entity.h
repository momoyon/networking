#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <switch.h>
#include <engine.h>
#include <nic.h>

typedef enum {
    EK_NIC,
    EK_SWITCH,
    EK_COUNT,
} Entity_kind;

const char *entity_kind_as_str(const Entity_kind k);

typedef struct {
    int *items;
    size_t count;
    size_t capacity;
} Entity_ids;

// NOTE: How we assign unique ids for entities:
// 1. We first check if `free_entity_ids` is empty:
//      1.true If empty: go 2.
//      1.false If not empty: pop last id as unique id
// 2. get `entity_id_counter` as unique id and increment `entity_id_counter`
extern size_t entity_id_counter;
extern Entity_ids free_entity_ids;

typedef enum {
    ESTATE_SELECTED,
    ESTATE_HOVERING,
    ESTATE_CONNECTING_FROM,
    ESTATE_CONNECTING_TO,
    ESTATE_COUNT,
} Entity_state_mask;

typedef struct {
    Entity *items;
    size_t count;
    size_t capacity;
} Entities;

struct Entity {
    Vector2 offset; // We use this to move with offset
    Vector2 pos;
    float radius;
    Entity_kind kind;
    size_t id;
    int state;

    Texture2D tex;

    Arena *arena; // All entity-related allocations
    Arena *temp_arena;

    // Kind specific
    Nic    *nic;
    Switch *switchh; // switch is a keyword in C

    Entities *entities;
};

Entity make_entity(Entities *entities, Vector2 pos, float radius, Entity_kind kind, Arena *arena, Arena *temp_arena);
void draw_entity(Entity *e, bool debug);
void free_entity(Entity *e);

bool connect(Entities *entities, Entity *a, Entity *b);
bool can_have_multiple_connections(Entity *a, Entity *b);

#endif // _ENTITY_H_
