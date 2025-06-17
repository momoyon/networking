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
//      - true If empty: go 2.
//      - false If not empty: pop last id as unique id
// 2. get `entity_id_counter` as unique id and increment `entity_id_counter`
extern size_t entity_id_counter;
extern Entity_ids free_entity_ids;

typedef enum {
    ESTATE_SELECTED,
    ESTATE_HOVERING,
    ESTATE_CONNECTING_FROM,
    ESTATE_CONNECTING_TO,
	ESTATE_DEAD,
    ESTATE_COUNT,
} Entity_state_mask;

typedef struct {
    Entity *items;
    size_t count;
    size_t capacity;
} Entities;

typedef struct {
	int *items;
	size_t count;
	size_t capacity;
} Entity_indices; // NOTE: are the indices in the entities static array @Darr

extern Entity_indices free_entity_indices;

// NOTE: How we add new entities:
//		- If `free_entity_indices` empty: arr_append(entities)
//		- If `free_entity_indices` not empty: pop last index from free_entity_indices and insert new entity there.

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
void free_nic(Entity *e);
void free_switch(Entity *e);

bool connect(Entities *entities, Entity *a, Entity *b);
bool can_have_multiple_connections(Entity *a, Entity *b);

#endif // _ENTITY_H_
