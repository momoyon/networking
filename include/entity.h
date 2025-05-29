#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <engine.h>

typedef enum {
    EK_NONE,
    EK_NETWORK_DEVICE,
    EK_COUNT,
} Entity_kind;

const char *entity_kind_as_str(const Entity_kind k);

extern int entity_id_counter;

typedef struct {
    Vector2 pos;
    float radius;
    Entity_kind kind;
    size_t id;
    bool selected;
} Entity;

Entity make_entity(Vector2 pos, float radius, Entity_kind kind);
void draw_entity(Entity *e, bool debug);

typedef struct {
    Entity *items;
    size_t count;
    size_t capacity;
} Entities;

#endif // _ENTITY_H_
