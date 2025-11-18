#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <switch.h>
#include <engine.h>
#include <nic.h>
#include <pc.h>
#include <ethernet_frame.h>

typedef struct Access_point Access_point;

// NOTE: Do not change the current order of enums! (Will affect current saves)
typedef enum {
    EK_SWITCH,
    EK_ACCESS_POINT,
    EK_PC,
    EK_PORT,
    EK_COUNT,
} Entity_kind;

const char *entity_kind_as_str(const Entity_kind k);

typedef struct {
    int *items;
    size_t count;
    size_t capacity;
} Entity_ids;

extern char *entity_texture_path_map[EK_COUNT];

typedef struct {
	int *items;
	size_t count;
	size_t capacity;
} Path_map;

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
    Entity **items;
    size_t count;
    size_t capacity;
} Entity_ptrs;

typedef struct {
	int *items;
	size_t count;
	size_t capacity;
} Entity_indices; // NOTE: are the indices in the entities static array @Darr

struct Entity {
    Vector2 offset; // We use this to move with offset
    Vector2 pos;
    float radius;
    Entity_kind kind;
    size_t id;
    int state;

    Texture2D tex;

    Arena *arena; // All entity-related allocations
    Arena *tmp_arena;
    Arena *str_arena;

    // Kind specific
    Nic    *nic;
    Switch *switchh; // switch is a keyword in C
    Access_point *ap;
    Pc *pc;
    Port *port;

    // // Specific for EK_PORT
    // Entity *parent_sw;
    // int parent_sw_id;
    // int port;
    // int module;

    Entities *entities;
};

Entity make_entity(Entities *entities, Vector2 pos, float radius, Entity_kind kind, Arena *arena, Arena *tmp_arena, Arena *str_arena);
void draw_entity(Entity *e, bool debug);
void update_entity_pos(Entity *e, Vector2 pos);
Entity_ptrs get_connected_entities(Entity *e);
void update_entity(Entity *e);
void update_ap(Entity *ap_e);
bool copy_entity_info(Entity *e);
uint8 *get_mac_address(Entity *e);
void disconnect_entity(Entity *e);
void disconnect_nic(Entity *e);
void disconnect_switch(Entity *e);
void disconnect_ap(Entity *e);
void disconnect_pc(Entity *e);
void free_entity(Entity *e);
void free_switch(Entity *e);
void free_ap(Entity *e);
void free_pc(Entity *e);

// Data-transfer
bool send_ethernet_frame(Entity *dst, Entity *src, Path_map *map);
bool receive(Entity *from, Entity *to, Ethernet_frame frame, int frame_id, Path_map *map);
bool receive_ether_frame__pc(Entity *from, Entity *pc_e, Ethernet_frame frame, int frame_id, Path_map *path);
bool receive_ether_frame__switch(Entity *from, Entity *sw_e, Ethernet_frame frame, int frame_id, Path_map *path);
bool receive_ether_frame__ap(Entity *from, Entity *sw_e, Ethernet_frame frame, int frame_id, Path_map *path);

bool is_entity_id_in_map(Path_map *map, int id);
Entity *get_entity_ptr_by_id(Entities *entities, int id);

bool connect_to_next_free_port(Entity *nic_e, Entity *switch_e);
bool connect_entity(Entities *entities, Entity *a, Entity *b);

// I/O
bool is_entities_saved(Entities *entities);
const char *save_entity_to_data(Entity *e, Arena *arena, Arena *tmp_arena, int version);
bool load_entity_from_file(Entity *e, const char *filepath);
bool save_entity_to_file(Entity *e, Arena *tmp_arena, const char *filepath, int version);
bool save_entities(Entities *entities, const char *filepath, size_t save_version);
bool load_entities(Entities *entities, const char *filepath, Arena *arena, Arena *tmp_arena, Arena *str_arena);

#endif // _ENTITY_H_
