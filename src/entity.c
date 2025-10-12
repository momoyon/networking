#include <entity.h>
#include <config.h>
#include <common.h>
#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

#include <switch.h>
#include <ap.h>

#include <misc.h>

size_t entity_id_counter = 0;
Entity_ids free_entity_ids = {0};
size_t entities_count = 0;
Entities entities = {0};
Entity_indices free_entity_indices = {0};

char *entity_texture_path_map[EK_COUNT] = {
    [EK_NIC] = "resources/gfx/nic.png",
    [EK_SWITCH] = "resources/gfx/switch.png",
    [EK_ACCESS_POINT] = "resources/gfx/ap.png",
};

static size_t get_unique_id(void) {
    if (free_entity_ids.count > 0) {
        int last_free_id = -1;
        darr_remove(free_entity_ids, int, &last_free_id, (int)free_entity_ids.count-1);
        ASSERT(last_free_id != -1, "We failed to get last free id!");
        return last_free_id;
    } else {
        return entity_id_counter++;
    }
}

const char *entity_kind_as_str(const Entity_kind k) {
    switch (k) {
        case EK_NIC: return "NIC";
        case EK_SWITCH: return "Switch";
        case EK_ACCESS_POINT: return "Access Point";
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
}

static void draw_info_text(Vector2 *p, const char *text, int font_size, Color color) {
    draw_text(GetFontDefault(), text, *p, font_size, color);
    p->y += font_size + 2;
}

void draw_entity(Entity *e, bool debug) {
    switch (e->kind) {
        case EK_NIC: {
            ASSERT(e->nic, "We failed to allocate nic!");
            // DrawCircle(e->pos.x, e->pos.y, e->radius, BLUE);

            if (e->state & (1<<ESTATE_SELECTED)) {
                Vector2 p = v2(e->pos.x + e->radius*1.5, e->pos.y + e->radius*1.5);
                DrawLineV(e->pos, p, WHITE);
                ASSERT(e->temp_arena, "BRUH");

                Ipv4_class ipv4_class = determine_ipv4_class(e->nic->ipv4_address);
                const char *ipv4_class_info = ipv4_class_additional_info(ipv4_class);
                draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                            "ipv4: %d.%d.%d.%d (%s | %s) [%s]",
                            e->nic->ipv4_address[0],
                            e->nic->ipv4_address[1],
                            e->nic->ipv4_address[2],
                            e->nic->ipv4_address[3],
                            ipv4_class_as_str(ipv4_class),
                            ipv4_class_info,
                            ipv4_type_as_str(determine_ipv4_type(e->nic->ipv4_address))),
                        ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                            "subnet mask: %d.%d.%d.%d",
                            e->nic->subnet_mask[0],
                            e->nic->subnet_mask[1],
                            e->nic->subnet_mask[2],
                            e->nic->subnet_mask[3]),
                        ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                            "mac: %02X:%02X:%02X:%02X:%02X:%02X",
                            e->nic->mac_address[0],
                            e->nic->mac_address[1],
                            e->nic->mac_address[2],
                            e->nic->mac_address[3],
                            e->nic->mac_address[4],
                            e->nic->mac_address[5]), ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                Entity *switch_ptr = NULL;
                if (e->nic->connected_entity && e->nic->connected_entity->kind == EK_SWITCH) {
                    switch_ptr = e->nic->connected_entity;
                }
                draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                            "switch: %p",
                            switch_ptr),
                        ENTITY_DEFAULT_RADIUS*0.5, WHITE);

            }

            // Draw connections
            if (e->nic->connected_entity) {
                if (e->nic->connected_entity->kind == EK_NIC) {
                    if (e->nic->drawing_connection) {
                        DrawLineBezier(e->pos, e->nic->connected_entity->pos, 1, WHITE);
                    }
                } else if (e->nic->connected_entity->kind == EK_SWITCH) {
                    DrawLineBezier(e->pos, e->nic->connected_entity->pos, 1, WHITE);
                } else {
                    ASSERT(false, "THIS SHOULDN'T HAPPEN!");
                }
            }
        } break;
        case EK_SWITCH: {
            ASSERT(e->switchh, "We failed to allocate switch!");
            if (e->state & (1<<ESTATE_SELECTED)) {
                Vector2 p = v2(e->pos.x + e->radius*1.5, e->pos.y + e->radius*1.5);
                DrawLineV(e->pos, p, WHITE);
                ASSERT(e->temp_arena, "BRUH");
                draw_info_text(&p, arena_alloc_str(*e->temp_arena, "%s", switch_model_as_str(e->switchh->model)), ENTITY_DEFAULT_RADIUS*0.5, WHITE);

                for (size_t i = 0; i < ARRAY_LEN(e->switchh->fe); ++i) {
                    for (size_t j = 0; j < ARRAY_LEN(e->switchh->fe[i]); ++j) {
                        Entity *conn = e->switchh->fe[i][j].conn;
                        if (!conn) continue;
                        if (conn->kind == EK_NIC) {
                            Nic *nic = conn->nic;
                            draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                                        "eth%zu/%zu: %d.%d.%d.%d | %d.%d.%d.%d", i, j,
                                        nic->ipv4_address[0],
                                        nic->ipv4_address[1],
                                        nic->ipv4_address[2],
                                        nic->ipv4_address[3],
                                        nic->subnet_mask[0],
                                        nic->subnet_mask[1],
                                        nic->subnet_mask[2],
                                        nic->subnet_mask[3]),
                                    ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                        } else if (conn->kind == EK_ACCESS_POINT) {
                            Access_point *ap = conn->ap;
                            draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                                        "eth%zu/%zu: "IPV4_FMT" | "SUBNET_MASK_FMT, i, j,
                                        IPV4_ARG(ap->mgmt_ipv4),
                                        SUBNET_MASK_ARG(ap->mgmt_subnet_mask)),
                                    ENTITY_DEFAULT_RADIUS*0.5, WHITE);
                        }
                    }
                }
            }
        } break;
        case EK_ACCESS_POINT: {
            Vector2 p = v2(e->pos.x + e->radius*1.5, e->pos.y + e->radius*1.5);
            DrawLineV(e->pos, p, WHITE);
            draw_info_text(&p, arena_alloc_str(*e->temp_arena, "mac address:"MAC_FMT, MAC_ARG(e->ap->mac_address)), ENTITY_DEFAULT_RADIUS*0.5, WHITE);
            draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                        "mgmt ipv4: "IPV4_FMT" "SUBNET_MASK_FMT, IPV4_ARG(e->ap->mgmt_ipv4), SUBNET_MASK_ARG(e->ap->mgmt_subnet_mask)),
                    ENTITY_DEFAULT_RADIUS*0.5, WHITE);
            draw_info_text(&p, arena_alloc_str(*e->temp_arena,
                        "Power: %s", e->ap->on ? "On" : "Off"),
                    ENTITY_DEFAULT_RADIUS*0.5, WHITE);

            // Draw connections
            if (e->ap->connected_entity) {
                if (e->ap->connected_entity->kind == EK_SWITCH) {
                    DrawLineBezier(e->pos, e->ap->connected_entity->pos, 1, WHITE);
                } else {
                    ASSERT(false, "THIS SHOULDN'T HAPPEN!");
                }
            }
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
    if (IsTextureReady(e->tex)) {
        Rectangle src = {
            .x = 0,
            .y = 0,
            .width = e->tex.width,
            .height = e->tex.height,
        };

        Rectangle dst = {
            .x = e->pos.x,
            .y = e->pos.y,
            .width  = e->tex.width*ENTITY_TEXTURE_SCALE,
            .height = e->tex.height*ENTITY_TEXTURE_SCALE,
        };
        // NOTE: Honestly no idea why i have to add this.
        static const int o = 4;
        DrawTexturePro(e->tex, src, dst, v2((ENTITY_DEFAULT_RADIUS/2)+o, (ENTITY_DEFAULT_RADIUS/2)+o), 0.0, WHITE);
    } else {
        DrawCircle(e->pos.x, e->pos.y, e->radius, WHITE);
    }
    if (debug) draw_text_aligned(GetFontDefault(), entity_kind_as_str(e->kind), Vector2Subtract(e->pos, v2(0, ENTITY_DEFAULT_RADIUS*1.5)), ENTITY_DEFAULT_RADIUS * 0.5, TEXT_ALIGN_V_CENTER, TEXT_ALIGN_H_CENTER, WHITE);

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
}

void update_entity(Entity *e) {
    switch (e->kind) {
        case EK_NIC: {
            // update_nic(e);
        } break;
        case EK_SWITCH: {
            // update_switch(e);
        } break;
        case EK_ACCESS_POINT: {
            update_ap(e);
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
}

void update_ap(Entity *ap_e) {
    Access_point *ap = ap_e->ap;
    ASSERT(ap, "BRUH");
    if (ap->on) {
        if (on_alarm(&ap->wifi_wave_alarm, GetFrameTime())) {
            Wifi_wave ww = {
                .pos = ap_e->pos,
                .color = WHITE,
                .dead_zone = 100.f,
            };
            darr_append(wifi_waves, ww);
        }
    }
}

static bool copy_ipv4(Entity *e) {
    const char *ipv4_str = NULL;
    switch (e->kind) {
        case EK_NIC: {
            ipv4_str = arena_alloc_str(*e->temp_arena, IPV4_FMT, IPV4_ARG(e->nic->ipv4_address));
        } break;
        case EK_SWITCH: {
            log_error("Cannot copy ipv4 from a Switch!");
            return false;
        } break;
        case EK_ACCESS_POINT: {
            ipv4_str = arena_alloc_str(*e->temp_arena, IPV4_FMT, IPV4_ARG(e->ap->mgmt_ipv4));
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
    ASSERT(ipv4_str != NULL, "BUH!");
    log_debug("COPIED IPV4: %s", ipv4_str);
    SetClipboardText(ipv4_str);
    return true;
}

static bool copy_subnet_mask(Entity *e) {
    const char *submask_str = NULL;
    switch (e->kind) {
        case EK_NIC: {
            submask_str = arena_alloc_str(*e->temp_arena, SUBNET_MASK_FMT, SUBNET_MASK_ARG(e->nic->subnet_mask));
        } break;
        case EK_SWITCH: {
            log_error("Cannot copy subnet mask from a Switch!");
            return false;
        } break;
        case EK_ACCESS_POINT: {
            submask_str = arena_alloc_str(*e->temp_arena, SUBNET_MASK_FMT, SUBNET_MASK_ARG(e->ap->mgmt_subnet_mask));
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
    ASSERT(submask_str != NULL, "BUH!");
    log_debug("COPIED SUBNET_MASK: %s", submask_str);
    SetClipboardText(submask_str);
    return true;
}

static bool copy_mac_address(Entity *e) {
    const char *mac_str = NULL;
    switch (e->kind) {
        case EK_NIC: {
            mac_str = arena_alloc_str(*e->temp_arena, MAC_FMT, MAC_ARG(e->nic->mac_address));
        } break;
        case EK_SWITCH: {
            log_error("Cannot copy mac address from a Switch!");
            return false;
        } break;
        case EK_ACCESS_POINT: {
            ASSERT(false, "EK_ACCESS_POINT copy_mac_address is UNIMPLEMENTED!");
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
    ASSERT(mac_str != NULL, "BUH!");
    log_debug("COPIED MAC_ADDRESS: %s", mac_str);
    SetClipboardText(mac_str);
    return true;
}

bool copy_entity_info(Entity *e) {
    if (IsKeyPressed(KEY_ONE)) {
        if (e == NULL) {
            log_warning("Please hover over the entity you want to copy info of!");
            return false;
        }
        if (!copy_ipv4(e)) return false;
    }
    if (IsKeyPressed(KEY_TWO)) {
        if (e == NULL) {
            log_warning("Please hover over the entity you want to copy info of!");
            return false;
        }
        if (!copy_subnet_mask(e)) return false;
    }
    if (IsKeyPressed(KEY_THREE)) {
        if (e == NULL) {
            log_warning("Please hover over the entity you want to copy info of!");
            return false;
        }
        if (!copy_mac_address(e)) return false;
    }
    return true;
}

static bool connect_nic_to(Entity *nic, Entity *other) {
    if (nic->kind != EK_NIC) {
        log_debug("That isn't a NIC brotato _/\\_");
        return false;
    }

    if (nic->nic->connected_entity != NULL) {
        log_error("This nic is already connected to something!");
        return false;
    }

    switch (other->kind) {
        case EK_NIC: {
            Entity *a = nic;
            Entity *b = other;

            if (b->nic->connected_entity != NULL && b->nic->connected_entity != a) {
                log_error("The other NIC is already connected to something!");
                return false;
            }

            a->nic->connected_entity = b;
            b->nic->connected_entity = a;
            a->nic->drawing_connection = true;
            return true;
        } break;
        case EK_SWITCH: {
            ASSERT(other->switchh, "bo");

            if (nic->nic->connected_entity != NULL && nic->nic->connected_entity->kind == EK_SWITCH && nic->nic->connected_entity != other) {
                log_error("Please disconnect the nic from any other switch!");
                return false;
            }

            bool found = false;

            for (size_t i = 0; i < ARRAY_LEN(other->switchh->fe); ++i) {
                for (size_t j = 0; j < ARRAY_LEN(other->switchh->fe[i]); ++j) {
                    Entity *conn = other->switchh->fe[i][j].conn;
                    if (conn && conn->nic == nic->nic) {
                        found = true;
                        break;
                    }
                }
            }
            if (!found) { 
                if (!connect_to_next_free_port(nic, other)) {
                    log_error("No free port available!");
                    return false;
                }
            } else {
                log_debug("RAH");
            }

            nic->nic->connected_entity = other;
            return true;
        } break;
        case EK_ACCESS_POINT: {
            ASSERT(false, "EK_ACCESS_POINT connect_nic_to is UNIMPLEMENTED!");
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }

    return false;
}

static bool connect_ap_to(Entity *ap, Entity *other) {
    if (!ap && ap->kind != EK_ACCESS_POINT) {
        log_warning("That isn't a AP lil vro _/\\_");
        return false;
    }

    switch (other->kind) {
        case EK_NIC: {
            return connect_nic_to(other, ap); // We can do this 
        } break;
        case EK_SWITCH: {
            ASSERT(other->switchh, "bo");

            if (ap->ap->connected_entity != NULL && ap->ap->connected_entity->kind == EK_SWITCH && ap->ap->connected_entity != other) {
                log_error("Please disconnect the AP from any other switch!");
                return false;
            }

            bool found = false;

            for (size_t i = 0; i < ARRAY_LEN(other->switchh->fe); ++i) {
                for (size_t j = 0; j < ARRAY_LEN(other->switchh->fe[i]); ++j) {
                    Entity *conn = other->switchh->fe[i][j].conn;
                    if (conn && conn->ap == ap->ap) {
                        found = true;
                        break;
                    }
                }
            }
            if (!found) { 
                if (!connect_to_next_free_port(ap, other)) {
                    log_error("No free port available!");
                    return false;
                }
            } else {
                log_debug("RAH");
            }

            ap->ap->connected_entity = other;
            return false;
        } break;
        case EK_ACCESS_POINT: {
            log_warning("Cannot connect two APs directly bro");
            return false;
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
    return false;
}

static bool connect_switch_to(Entity *switchh, Entity *other) {
    if (switchh->kind != EK_SWITCH) {
        log_warning("That isn't a NIC brochacho _/\\_");
        return false;
    }

    switch (other->kind) {
        case EK_NIC: {
            return connect_nic_to(other, switchh); // We can do this 
        } break;
        case EK_SWITCH: {
            log_warning("We can't connect two switches directly!");
            return false;
        } break;
        case EK_ACCESS_POINT: {
            return connect_ap_to(other, switchh);
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
    return false;
}

bool connect_entity(Entities *entities, Entity *a, Entity *b) {
    ASSERT(entities, "Bro pass an array of entities!");

    ASSERT(a && b, "BRUH");

    bool connected = false;

    switch (a->kind) {
        case EK_NIC: {
            connected = connect_nic_to(a, b);
        } break;
        case EK_SWITCH: {
            connected = connect_switch_to(a, b);
            return false;
        } break;
        case EK_ACCESS_POINT: {
            connected = connect_ap_to(a, b);
            return false;
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }

    if (connected) {
        // Connection conn = {
        //     .from = &a->pos,
        //     .to   = &b->pos,
        // };
        // darr_append(connections, conn);
    }

    return true;
}

static void init_entity(Entity *e, Arena *arena, Arena *temp_arena, Arena *str_arena) {
    (void)temp_arena;
    switch (e->kind) {
        case EK_NIC: {
            e->nic = (Nic *)arena_alloc(arena, sizeof(Nic));
            make_nic(e, e->nic, arena);
            e->tex = load_texture_checked(entity_texture_path_map[EK_NIC]);
            ASSERT(IsTextureReady(e->tex), "Failed to load network interface image!");
            e->nic->nic_entity_id = -1;
        } break;
        case EK_SWITCH: {
            e->switchh = (Switch *)arena_alloc(arena, sizeof(Switch));
            // TODO: Take switch model as input
            make_switch(SW_MODEL_MOMO_SW_2025_A, "1.0.0", e->switchh, arena, temp_arena, str_arena);
            e->tex = load_texture_checked(entity_texture_path_map[EK_SWITCH]);
        } break;
        case EK_ACCESS_POINT: {
            e->ap = (Access_point *)arena_alloc(arena, sizeof(Access_point));
            make_ap(e, e->ap, arena);
            e->tex = load_texture_checked(entity_texture_path_map[EK_ACCESS_POINT]);
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
}

// Makers
Entity make_entity(Entities *entities, Vector2 pos, float radius, Entity_kind kind, Arena *arena, Arena *temp_arena, Arena *str_arena) {
    Entity e = (Entity) {
        .pos = pos,
        .radius = radius,
        .kind = kind,
        .id = get_unique_id(),
        .state = 0,
        .arena = arena,
        .temp_arena = temp_arena,
        .str_arena = str_arena,
        .entities = entities,
    };

    init_entity(&e, arena, temp_arena, str_arena);

    return e;
}

static bool is_mac_address_assigned(Entities *entities, uint8 *mac_address) {
    for (size_t i = 0; i < entities->count; ++i) {
        Entity *e = &entities->items[i];
        if (e->state & (1<<ESTATE_DEAD)) continue;

        if (e->kind == EK_NIC) {
            uint8 *nic_mac = e->nic->mac_address;
            if (memcmp(mac_address, nic_mac, 6) == 0) {
                return true;
            }
        }
    }
    return false;
}

void make_nic(Entity *e, Nic *nic, Arena *arena) {
    (void)arena;
    nic->ipv4_address[0] = 0;
    nic->ipv4_address[1] = 0;
    nic->ipv4_address[2] = 0;
    nic->ipv4_address[3] = 0;
    nic->subnet_mask[0] = 255;
    nic->subnet_mask[1] = 255;
    nic->subnet_mask[2] = 255;
    nic->subnet_mask[3] = 0;

    // nic->id = e->id;
    // nic->self_entity = e;
    nic->connected_entity = NULL;
    do {
        float64 tp1 = GetTime();
        get_unique_mac_address(nic->mac_address);
        log_debug("get_unique_mac_address() took %.2lfs", GetTime() - tp1);
    } while (is_mac_address_assigned(e->entities, nic->mac_address));
}

void make_switch(Switch_model model, const char *version, Switch *switch_out, Arena *arena, Arena *tmp_arena, Arena *str_arena) {
    Switch s = {.model = model};
    s.tmp_arena = tmp_arena;
    s.str_arena = str_arena;
    s.version = version;
    s.hostname = "Switch";

    s.boot_load_alarm.alarm_time = 0.1f;

    make_switch_console(&s.console, arena);

    switch_change_mode(&s, SW_CNSL_MODE_USER);

    *switch_out = s;
}

void make_switch_console(Console *console_out, Arena *arena) {
    (void)arena;

    Console_line l = {0};
    console_out->font = GetFontDefault();
    // console_out->prefix = "Switch>";
    darr_append(console_out->lines, l);
}

void make_ap(Entity *e, Access_point *ap_out, Arena *arena) {
    (void)arena;
    memset(ap_out, 0, sizeof(*ap_out));
    ap_out->wifi_wave_alarm.alarm_time = AP_WIFI_WAVE_ALARM_TIME;

    do {
        float64 tp1 = GetTime();
        get_unique_mac_address(ap_out->mac_address);
        log_debug("get_unique_mac_address() took %.2lfs", GetTime() - tp1);
    } while (is_mac_address_assigned(e->entities, ap_out->mac_address));
}

// Disconnect-ers
void disconnect_entity(Entity *e) {
    switch (e->kind) {
        case EK_NIC: {
            disconnect_nic(e);
        } break;
        case EK_SWITCH: {
            disconnect_switch(e);
        } break;
        case EK_ACCESS_POINT: {
            disconnect_ap(e);
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
}

bool match_port_kind(Entity *e, Port *port) {
    if (!port || !port->conn) return false;
    if (e->kind == EK_NIC) return port->conn->nic == e->nic;
    if (e->kind == EK_ACCESS_POINT) return port->conn->ap == e->ap;
    return false;
}

void set_connected_entity(Entity *e, Entity *to) {
    switch (e->kind) {
        case EK_NIC: {
            e->nic->connected_entity = to;
        } break;
        case EK_SWITCH: {
            log_warning("Switch doesn't have a `connected_entity` member!");
            // e->nic->connected_entity = to;
        } break;
        case EK_ACCESS_POINT: {
            e->ap->connected_entity = to;
        } break;
        case EK_COUNT: 
        default: ASSERT(false, "UNREACHABLE!");
    }
}

Entity *get_connected_entity(Entity *e) {
    switch (e->kind) {
        case EK_NIC: return e->nic->connected_entity;
        case EK_SWITCH: return NULL;
        case EK_ACCESS_POINT: return e->ap->connected_entity;
        case EK_COUNT: 
        default: ASSERT(false, "UNREACHABLE!");
    }
    return NULL;
}

void disconnect_port(Port *port) {
    if (port && port->conn) {
        set_connected_entity(port->conn, NULL);
        port->conn = NULL;
    }
}

static void disconnect_connected_entity(Entity *e) {
    Entity *connected_entity = get_connected_entity(e);

    if (connected_entity && 
        connected_entity->kind == EK_NIC &&
        connected_entity->nic->connected_entity == e) {
        connected_entity->nic->connected_entity = NULL;
    }
    if (connected_entity && 
        connected_entity->kind == EK_SWITCH) {
        for (size_t i = 0; i < ARRAY_LEN(connected_entity->switchh->fe); ++i) {
            for (size_t j = 0; j < ARRAY_LEN(connected_entity->switchh->fe[i]); ++j) {
                Port *port = &connected_entity->switchh->fe[i][j];
                if (match_port_kind(e, port)) {
                    disconnect_port(port);
                    break; // We shouldn't have duplicate entries
                }
            }
        }
    }

    set_connected_entity(e, NULL);
}

void disconnect_nic(Entity *e) {
    ASSERT(e->kind == EK_NIC, "BRO");
    disconnect_connected_entity(e);
    log_debug("Disconnected NIC with ID: %zu", e->id);
}

void disconnect_switch(Entity *e) {
    ASSERT(e->kind == EK_SWITCH, "BRO");
    for (size_t i = 0; i < ARRAY_LEN(e->switchh->fe); ++i) {
        for (size_t j = 0; j < ARRAY_LEN(e->switchh->fe[i]); ++j) {
            Port *port = &e->switchh->fe[i][j];
            disconnect_port(port);
        }
    }
    log_debug("Disconnected switch with ID: %zu", e->id);
}

void disconnect_ap(Entity *e) {
    ASSERT(e->kind == EK_ACCESS_POINT, "BRO");
    disconnect_connected_entity(e);
    log_debug("Disconnected ap with ID: %zu", e->id);
}

// Free-ers
void free_entity(Entity *e) {
    darr_append(free_entity_ids, e->id);

    switch (e->kind) {
        case EK_NIC: {
            free_nic(e);
        } break;
        case EK_SWITCH: {
            free_switch(e);
        } break;
        case EK_ACCESS_POINT: {
            // @Pass
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
}

void free_nic(Entity *e) {
    ASSERT(e->kind == EK_NIC, "Br");
    if (e->nic->connected_entity != NULL) {
        Entity *e_conn = e->nic->connected_entity;
        if (e_conn->nic->connected_entity == e) {
            e_conn->nic->connected_entity = NULL;
        }
    }

    log_debug("IS THIS EVER CALLED?");
    
    // Add free mac_address so it can be reused
    Mac_address m = {0};
    m.addr[0] = e->nic->mac_address[0];
    m.addr[1] = e->nic->mac_address[1];
    m.addr[2] = e->nic->mac_address[2];
    m.addr[3] = e->nic->mac_address[3];
    m.addr[4] = e->nic->mac_address[4];
    m.addr[5] = e->nic->mac_address[5];
    darr_append(free_mac_addresses, m);

    if (e->nic->connected_entity != NULL && e->nic->connected_entity->kind == EK_SWITCH) {
        Entity *e_switch = e->nic->connected_entity;
        for (size_t i = 0; i < ARRAY_LEN(e_switch->switchh->fe); ++i) {
            for (size_t j = 0; j < ARRAY_LEN(e_switch->switchh->fe[i]); ++j) {
                Entity *conn = e_switch->switchh->fe[i][j].conn;
                if (conn && conn->nic == e->nic) {
                    e_switch->switchh->fe[i][j].conn = NULL;
                    // nic->connected_entity = NULL;
                }
            }
        }
    }
    e->nic->connected_entity = NULL;
}

void free_switch(Entity *e) {
    ASSERT(e->kind == EK_SWITCH, "Br");

    // Remove any reference to this switch from the connected NICs
    for (size_t i = 0; i < ARRAY_LEN(e->switchh->fe); ++i) {
        for (size_t j = 0; j < ARRAY_LEN(e->switchh->fe[i]); ++j) {
            Entity *conn = e->switchh->fe[i][j].conn;
            if (conn && conn->nic && conn->nic->connected_entity == e && conn->nic->connected_entity->kind == EK_SWITCH) {
                conn->nic->connected_entity = NULL;
            }
        }
    }
}

// Data-transfer
bool send_arp_ethernet_frame(Entity *dst, Entity *src) {
    Ethernet_frame eframe = {
        .ether_type_or_length = ETHER_TYPE_ARP,
        .payload = (uint8 *)"ARP PING TEST",
        .crc = 0,
    };

    memcpy(eframe.dst, dst->nic->mac_address, sizeof(uint8)*6);
    memcpy(eframe.src, src->nic->mac_address, sizeof(uint8)*6);

    return recieve(dst, src, eframe);
}
//
// static bool is_frame_for_conn(Entity *conn, Ethernet_frame frame) {
//     if (!conn) {
//         log_error("%s: Please pass a non-NULL conn!", __func__);
//         return false;
//     }
//
//     Mac_address conn_mac_addr = {0};
//     if (conn->kind == EK_NIC) {
//         memcpy(conn_mac_addr.addr, conn->nic->mac_address, sizeof(uint8) * 6);
//     } else if (conn->kind == EK_ACCESS_POINT) {
//         memcpy(conn_mac_addr.addr, conn->ap->, sizeof(uint8) * 6);
//     } else {
//         log_error("%s: Invalid conn kind!", __func__);
//     }
//     return false;
// }
//
// static bool forward_frame_via_switch(Entity *se, Ethernet_frame frame) {
//     ASSERT(se->kind == EK_SWITCH, "brother");
//
//     for (size_t i = 0; i < ARRAY_LEN(se->switchh->fe); ++i) {
//         for (size_t j = 0; j < ARRAY_LEN(se->switchh->fe[i]); ++j) {
//             Entity *conn = e->switchh->fe[i][j].conn;
//             if (is_frame_for_conn(conn, frame)) {
//                 log_debug("Received Ethernet Frame from "MAC_FMT" to "MAC_FMT, MAC_ARG(frame.src), MAC_ARG(frame.dst));
//                 return true;
//             }
//         }
//     }
//
//     return false;
// }

bool recieve(Entity *dst, Entity *src, Ethernet_frame frame) {
    (void)src;
    switch (dst->kind) {
        case EK_NIC: {
            if (dst->nic->connected_entity != src) {
                log_error("The dst NIC is not connected to the src NIC!");
                // if (dst->nic->connected_entity != NULL && dst->nic->connected_entity == EK_SWITCH) {
                //     return forward_frame_via_switch(dst->nic->connected_entity, frame);
                // } else {
                //     log_error("The dst NIC is not connected to a switch either");
                // }
                return false;
            }

            if (memcmp(frame.dst, dst->nic->mac_address, sizeof(uint8)*6) != 0) {
                log_warning("Ethernet Frame destined for "MAC_FMT" is dropped at src's connected NIC "MAC_FMT, MAC_ARG(frame.dst), MAC_ARG(dst->nic->mac_address));
                return false;
            }
            log_debug("Received Ethernet Frame from "MAC_FMT" to "MAC_FMT, MAC_ARG(frame.src), MAC_ARG(frame.dst));
            return true;
        } break;
        case EK_SWITCH: {
            ASSERT(false, "EK_SWITCH recieve is UNIMPLEMENTED!");
        } break;
        case EK_ACCESS_POINT: {
            ASSERT(false, "EK_ACCESS_POINT recieve is UNIMPLEMENTED!");
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
}

Entity *get_entity_ptr_by_id(Entities *entities, int id) {
    for (size_t i = 0; i < entities->count; ++i) {
        Entity *e = &entities->items[i];
        if (e->id == id) {
            return e;
        }
    }

    return NULL;
}

// I/O
// NOTE: Yea so we can't just dump the entities array and load it directly from the filesystem cuz the entities have memebers alloced on Arena memory... :(
bool is_entities_saved(Entities *entities) {
    (void)entities;
    ASSERT(false, "is_entities_saved() is unimplemented!");
    return false;
}

const char *entity_kind_save_format(Entity *e, Arena *temp_arena) {
    switch (e->kind) {
        case EK_NIC: {
            return arena_alloc_str(*temp_arena, IPV4_FMT" "SUBNET_MASK_FMT" %d.%d.%d.%d.%d.%d %d", 
                    IPV4_ARG(e->nic->ipv4_address),
                    SUBNET_MASK_ARG(e->nic->subnet_mask),
                    e->nic->mac_address[0],
                    e->nic->mac_address[1],
                    e->nic->mac_address[2],
                    e->nic->mac_address[3],
                    e->nic->mac_address[4],
                    e->nic->mac_address[5],
                    e->nic && e->nic->connected_entity && e->nic->connected_entity->kind == EK_NIC ? (int)e->nic->connected_entity->id : -1);
        } break;
        case EK_SWITCH: {
            const char *res = (const char *)temp_arena->ptr;
            for (size_t i = 0; i < ARRAY_LEN(e->switchh->fe); ++i) {
                for (size_t j = 0; j < ARRAY_LEN(e->switchh->fe[i]); ++j) {
                    Port *port = &e->switchh->fe[i][j];
                    arena_alloc_str(*temp_arena, "%zu/%zu: %d ", i, j, (port->conn ? (int)port->conn->id : -1));
                    temp_arena->ptr--;

                    if (port->conn) {
                        int id = port->conn->id;
                        ASSERT(id >= 0, "Ig the port isn't connected to anything? in that case it should be NULL tho?");
                        log_debug("Port %zu/%zu %s entity: (%d)", i, j, entity_kind_as_str(port->conn->kind), id);
                    }
                }
            }
            arena_alloc_str(*temp_arena, "%s", "|");
            log_debug("SWITCH KIND SAVE FMT: %s", res);
            return res;
        } break;
        case EK_ACCESS_POINT: {
            return arena_alloc_str(*temp_arena, 
                    IPV4_FMT" "SUBNET_MASK_FMT" %d.%d.%d.%d.%d.%d %d ",
                    IPV4_ARG(e->ap->mgmt_ipv4), 
                    SUBNET_MASK_ARG(e->ap->mgmt_subnet_mask),
                    MAC_ARG(e->ap->mac_address),
                    e->ap->on ? 1 : 0);
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
    return "NOPE";
}

const char *save_entity_to_data(Entity *e, Arena *arena, Arena *temp_arena, int version) {
    char *s = "";
    switch (version) {
        case 1: {
            s = arena_alloc_str(*arena, "v%d %.2f %.2f %d %zu %d ", version, e->pos.x, e->pos.y, e->kind, e->id, e->state);
        } break;
        case 2: {
            s = arena_alloc_str(*arena, "v%d %.2f %.2f %d %zu %d %s ", version, e->pos.x, e->pos.y, e->kind, e->id, e->state, entity_kind_save_format(e, temp_arena));
        } break;
        default: ASSERT(false, "UNREACHABLE!");
    }
    arena->ptr--; // remove \0
    log_debug("BRO: %s", s);

    return s;
}

static bool load_entity_from_data_v1(Entity *e, String_view *sv) {
    String_view pos_x_sv = sv_lpop_until_char(sv, ' ');
    sv_lremove(sv, 1); // Remove space
    String_view pos_y_sv = sv_lpop_until_char(sv, ' ');
    sv_lremove(sv, 1); // Remove space

    String_view kind_sv = sv_lpop_until_char(sv, ' ');
    sv_lremove(sv, 1); // Remove space

    String_view id_sv = sv_lpop_until_char(sv, ' ');
    sv_lremove(sv, 1); // Remove space

    String_view state_sv = sv_lpop_until_char(sv, ' ');
    sv_lremove(sv, 1); // Remove space

    int pos_x_count = -1;
    float pos_x = sv_to_float(pos_x_sv, &pos_x_count);
    if (pos_x_count < 0) {
        log_error("pos.x: Failed to convert `"SV_FMT"` to float!", SV_ARG(pos_x_sv));
        return false;
    }
    int pos_y_count = -1;
    float pos_y = sv_to_float(pos_y_sv, &pos_y_count);
    if (pos_y_count < 0) {
        log_error("pos.y: Failed to convert `"SV_FMT"` to float!", SV_ARG(pos_y_sv));
        return false;
    }

    int kind_count = -1;
    int kind = sv_to_uint(kind_sv, &kind_count, 10);
    if (kind_count < 0) {
        log_error("kind: Failed to convert `"SV_FMT"` to uint!!", SV_ARG(kind_sv));
        return false;
    }

    int id_count = -1;
    int id = sv_to_uint(id_sv, &id_count, 10);
    if (id_count < 0) {
        log_error("id: Failed to convert `"SV_FMT"` to uint!!", SV_ARG(id_sv));
        return false;
    }

    int state_count = -1;
    int state = sv_to_int(state_sv, &state_count, 10);
    if (state_count < 0) {
        log_error("state: Failed to convert state `"SV_FMT"` to int!!", SV_ARG(state_sv));
        return false;
    }

    log_debug("--------------------------------------------------");
    log_debug("Loading %s[%d] @ %.2f %.2f", entity_kind_as_str(kind), id, pos_x, pos_y);
    log_debug("--------------------------------------------------");

    e->pos.x = pos_x;
    e->pos.y = pos_y;
    e->kind = kind;
    e->id = id;
    e->state = state;

    init_entity(e, e->arena, e->temp_arena, e->str_arena);

    return true;
}

static bool parse_n_octet_from_data(int n, String_view *sv, uint8 *octets, size_t octets_count) {
    if (octets_count < n) {
        log_error("%s: octets count passed is less than n: %zu < %d", __func__, octets_count, n);
        return false;
    }

    for (int i = 0; i < n; ++i) {
        char c = i == n-1 ? ' ' : '.';
        String_view oct_sv = sv_lpop_until_char(sv, c);
        int oct_count = -1;
        uint oct = sv_to_uint(oct_sv, &oct_count, 10);
        if (oct_count < 0) {
            log_error("Failed to convert oct%d `"SV_FMT"` to a number!", i+1, SV_ARG(oct_sv));
            return false;
        }
        if (oct > 255) {
            log_error("Octets must be in the range 0-255!");
            return false;
        }
        if (i < n-1) {
            sv_lremove(sv, 1); // Remove .
        }

        octets[i] = oct;
    }
    return true;
}

static bool parse_four_octet_from_data(String_view *sv, uint8 four_octet[4]) {
    return parse_n_octet_from_data(4, sv, four_octet, 4);
}

static bool parse_nic_from_data(Nic *nic, String_view *sv) {
    uint8 ipv4[4] = {0};
    uint8 subnet_mask[4] = {0};
    uint8 mac_address[6] = {0};
    if (!parse_four_octet_from_data(sv, ipv4)) {
        return false;
    }

    if (!parse_four_octet_from_data(sv, subnet_mask)) {
        return false;
    }

    if (!parse_n_octet_from_data(6, sv, mac_address, 6)) {
        return false;
    }

    log_debug("--------------------------------------------------");
    log_debug("Parsed ipv4: %d.%d.%d.%d", ipv4[0], ipv4[1], ipv4[2], ipv4[3]);
    log_debug("Parsed subnet_mask: %d.%d.%d.%d", subnet_mask[0], subnet_mask[1], subnet_mask[2], subnet_mask[3]);
    log_debug("Parsed mac_address: %d.%d.%d.%d.%d.%d", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
    log_debug("--------------------------------------------------");

    sv_ltrim(sv);

    memcpy(nic->ipv4_address, ipv4, sizeof(uint8) * 4);
    memcpy(nic->subnet_mask, subnet_mask, sizeof(uint8) * 4);
    memcpy(nic->mac_address, mac_address, sizeof(uint8) * 6);

    sv_ltrim(sv);
    String_view nic_id_sv = sv_lpop_until_char(sv, ' ');
    sv_ltrim(sv);

    int nic_id_count = -1;
    int nic_id = sv_to_int(nic_id_sv, &nic_id_count, 10);
    if (nic_id_count < 0) {
        log_error("Failed to convert nic id `"SV_FMT"` to int!", SV_ARG(nic_id_sv));
        return false;
    }

    log_debug("Parsed nic_entity_id %d", nic_id);

    nic->nic_entity_id = nic_id;

    return true;
}

static bool load_entity_from_data_v2(Entity *e, String_view *sv) {
    if (!load_entity_from_data_v1(e, sv)) {
        return false;
    }

    switch (e->kind) {
        case EK_NIC: {
            if (!parse_nic_from_data(e->nic, sv)) return false;
            return true;
        } break;
        case EK_SWITCH: {
            if (e->switchh == NULL) {
                log_error("Please allocate the switch before trying to load from data!");
                ASSERT(false, "DEBUG");
                return false;
            }

            log_debug("Parsing Switch...");

            sv_ltrim(sv);

            String_view switch_sv = sv_lpop_until_char(sv, '|');
            sv_lremove(sv, 1); // Remove |

            while (switch_sv.count > 0) {
                int i = -1;
                int j = -1;

                if (!parse_i_j_from_sv(&switch_sv, &i, &j)) {
                    return false;
                }
                sv_ltrim(&switch_sv);

                String_view port_conn_id_sv = sv_lpop_until_char(&switch_sv, ' ');
                sv_ltrim(&switch_sv);

                int port_conn_id_count = -1;
                int port_conn_id = sv_to_int(port_conn_id_sv, &port_conn_id_count, 10);
                if (port_conn_id_count < 0) {
                    log_error("Failed to convert port conn id `"SV_FMT"` to int!", SV_ARG(port_conn_id_sv));
                    return false;
                }
                log_debug("Parsed port %d/%d: %d", i, j, port_conn_id);
                if (i < 0 || i > ARRAY_LEN(e->switchh->fe)-1) {
                    log_error("Failed to parse switch fmt: i is outofbounds: %d (0 ~ %zu)", i, ARRAY_LEN(e->switchh->fe));
                }
                if (j < 0 || j > ARRAY_LEN(e->switchh->fe[0])-1) {
                    log_error("Failed to parse switch fmt: j is outofbounds: %d (0 ~ %zu)", j, ARRAY_LEN(e->switchh->fe[0]));
                }
                e->switchh->fe[i][j].conn_id = port_conn_id;
            }
            return true;
        } break;
        case EK_ACCESS_POINT: {
            uint8 ipv4[4] = {0};
            uint8 subnet_mask[4] = {0};
            uint8 mac_address[6] = {0};

            if (!parse_four_octet_from_data(sv, ipv4)) {
                return false;
            }
            if (!parse_four_octet_from_data(sv, subnet_mask)) {
                return false;
            }
            if (!parse_n_octet_from_data(6, sv, mac_address, 6)) {
                return false;
            }
            sv_ltrim(sv);

            String_view on_sv = sv_lpop_until_char(sv, ' ');
            if (on_sv.data[0] == '0') {
                e->ap->on = false;
            } else if (on_sv.data[0] == '1') {
                e->ap->on = true;
            } else {
                ASSERT(false, "WE GOT NEITHER 0 NOR 1 FOR AP POWER!");
            }

            memcpy(e->ap->mac_address, mac_address, sizeof(uint8)*6);
            memcpy(e->ap->mgmt_ipv4, ipv4, sizeof(uint8)*4);
            memcpy(e->ap->mgmt_subnet_mask, subnet_mask, sizeof(uint8)*4);

            return true;
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
    return false;
}

bool load_entity_from_data(Entity *e, String_view *data_sv) {
    sv_ltrim(data_sv);
    String_view version_sv = sv_lpop_until_char(data_sv, ' ');
    sv_lremove(data_sv, 1); // Remove space
    sv_lremove(&version_sv, 1); // Remove v
    int version_count = -1;
    int version = sv_to_int(version_sv, &version_count, 10);
    if (version_count < 0) {
        log_error("Failed to convert version `"SV_FMT"` to int!", SV_ARG(version_sv));
        return false;
    }

    switch (version) {
        case 1: return load_entity_from_data_v1(e, data_sv);
        case 2: return load_entity_from_data_v2(e, data_sv);
        default: {
            log_debug("Got version %d", version);
            ASSERT(false, "UNREACHABLE!");
        } break;
    }
    return false;
}

bool load_entity_from_file(Entity *e, const char *filepath) {
    int file_size = -1;
    const char *file = read_file(filepath, &file_size);
    if (file_size == -1) {
        return false;
    }
    String_view sv = SV(file);
    
    if (!load_entity_from_data(e, &sv)) {
        return false;
    }

    free((void*)file);
    return true;
}

bool save_entity_to_file(Entity *e, Arena *temp_arena, const char *filepath, int version) {
    FILE *f = fopen(filepath, "w");

    if (!f) { 
        return false;
    }

    Arena a = arena_make(0);
    const char *s = save_entity_to_data(e, temp_arena, &a, version);
    size_t s_len = strlen(s);

    size_t wrote = fwrite(s, sizeof(char), s_len, f);
    if (wrote < s_len) {
        log_debug("Failed to fwrite!");
        fclose(f);
        return false;
    }

    temp_arena->ptr -= s_len; // Dealloc the string;

    fclose(f);

    arena_free(&a);

    return true;
}

bool save_entities(Entities *entities, const char *filepath, size_t save_version) {
    Arena entities_arena = arena_make(0);
    Arena temp_arena = arena_make(0);

    for (size_t i = 0; i < entities->count; ++i) {
        Entity *e = &entities->items[i];
        if (e->state & (1<<ESTATE_DEAD)) continue;

        if (!save_entity_to_data(e, &entities_arena, &temp_arena, save_version)) {
            arena_free(&entities_arena);
            arena_free(&temp_arena);
            return false;
        }
    }

    int c = (int)((char*)entities_arena.ptr - (char*)entities_arena.buff);
    // log_debug("Alloced %zu bytes in entities_arena", c);
    log_debug("Entities: %.*s", c, (char*)entities_arena.buff);

    FILE *f = fopen(filepath, "wb");
    size_t wrote = fwrite(entities_arena.buff, sizeof(char), c, f);
    if (wrote < c) {
        log_error("Failed to write entities data to `%s`", filepath);
        fclose(f);
        return false;
    }
    fclose(f);

    arena_free(&entities_arena);
    arena_free(&temp_arena);
    return true;
}

bool load_entities(Entities *entities, const char *filepath, Arena *arena, Arena *temp_arena, Arena *str_arena) {
    // Reset before loading new entities
    entities_count = 0;
    entities->count = 0;
    arena_reset(arena);

    int file_size = -1;
    const char *file = read_file(filepath, &file_size);

    if (file_size == -1) {
        return false;
    }

    String_view sv = SV(file);

    while (sv.count > 0) {
        Entity e = make_entity(entities, v2xx(0), ENTITY_DEFAULT_RADIUS, EK_NIC, arena, temp_arena, str_arena);
        sv_trim(&sv);
        if (!load_entity_from_data(&e, &sv)) {
            free((void*)file);
            return false;
        }
        // init_entity(&e, arena, temp_arena);
        add_entity(e);
    }

    log_debug("Entities.count after loading: %zu", entities->count);

    // Assign the conn pointers to the switch ports using the parsed conn_id
    for (size_t i = 0; i < entities->count; ++i) {
        Entity *e = &entities->items[i];
        if (e->kind == EK_SWITCH) {
            for (size_t i = 0; i < ARRAY_LEN(e->switchh->fe); ++i) {
                for (size_t j = 0; j < ARRAY_LEN(e->switchh->fe[i]); ++j) {
                    Port *port = &e->switchh->fe[i][j];
                    if (port->conn_id >= 0) {
                        Entity *conn = get_entity_ptr_by_id(entities, port->conn_id);
                        if (conn == NULL) {
                            log_error("Cannot find CONN with id %d", port->conn_id);
                            return false;
                        }
                        port->conn = conn;
                        if (conn->kind == EK_NIC) {
                            conn->nic->connected_entity = e;
                        } else if (conn->kind == EK_ACCESS_POINT) {
                            conn->ap->connected_entity = e;
                        }
                    }
                }
            }
        }
    }

    // Assign the nic pointers to the nic_entity using the parsed nic_id
    for (size_t i = 0; i < entities->count; ++i) {
        Entity *e = &entities->items[i];
        if (e->kind == EK_NIC) {
            if (e->nic->nic_entity_id >= 0) {
                Entity *nic_entity = get_entity_ptr_by_id(entities, e->nic->nic_entity_id);
                if (!e) {
                    log_error("Failed to get entity from nic_id %d that NIC %zu was connected to...", e->nic->nic_entity_id, e->id);
                    return false;
                }
                e->nic->connected_entity = nic_entity;
            }
        }
    }

    free((void*)file);
    return true;
}

static bool four_octet_from_input(uint8 *four_octet, char *chars_buff, size_t *chars_buff_count, size_t chars_buff_cap) {
    // if (*chars_buff_count == 1) {
    //     (*chars_buff_count)--;
    // }
    int ch = 0;
    do {
        ch = GetCharPressed();
        if (IsKeyPressed(KEY_ENTER)) {
            String_view four_octet_sv = (String_view) {
                .data  = chars_buff,
                .count = *chars_buff_count,
            };
            if (!parse_four_octet_from_data(&four_octet_sv, four_octet)) {
                return false;
            }

            *chars_buff_count = 0;
            return true;
        }

        if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && (*chars_buff_count) > 0) {
            (*chars_buff_count)--;
        }
        if (('0' <= ch && ch <= '9') || ch == '.') {
            if (*chars_buff_count >= chars_buff_cap) {
                log_error("Exhausted chars buff!");
                exit(1);
            }
            chars_buff[(*chars_buff_count)++] = (char)ch;
            // log_debug("CHAR: %c(%d)", (char)ch, ch);
        }
    } while (ch > 0);
    return false;
    // log_debug("TYPED %zu chars!", chars_count);

}

bool ipv4_from_input(Entity *e, char *chars_buff, size_t *chars_buff_count, size_t chars_buff_cap) {
    switch (e->kind) {
        case EK_NIC:
            return four_octet_from_input(e->nic->ipv4_address, chars_buff, chars_buff_count, chars_buff_cap);
        case EK_ACCESS_POINT:
            return four_octet_from_input(e->ap->mgmt_ipv4, chars_buff, chars_buff_count, chars_buff_cap);
        case EK_SWITCH: {
            log_warning("Cannot change the ipv4 of a switch!");
            return false;
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
    return false;
}

bool subnet_mask_from_input(Entity *e, char *chars_buff, size_t *chars_buff_count, size_t chars_buff_cap) {
    switch (e->kind) {
        case EK_NIC:
            return four_octet_from_input(e->nic->subnet_mask, chars_buff, chars_buff_count, chars_buff_cap);
        case EK_ACCESS_POINT:
            return four_octet_from_input(e->ap->mgmt_subnet_mask, chars_buff, chars_buff_count, chars_buff_cap);
        case EK_SWITCH: {
            log_warning("Cannot change the subnet mask of a switch!");
            return false;
        } break;
        case EK_COUNT:
        default: ASSERT(false, "UNREACHABLE!");
    }
    return false;
}

bool connect_to_next_free_port(Entity *e, Entity *switch_e) {
    if (!e || (e->kind != EK_NIC && e->kind != EK_ACCESS_POINT)) {
        log_error("Cannot connect to port: the NIC or AP is not valid!");
        return false;
    }
    if (!switch_e || switch_e->kind != EK_SWITCH) {
        log_error("Cannot connect to port: the Switch is not valid!");
        return false;
    }

    for (size_t i = 0; i < ARRAY_LEN(switch_e->switchh->fe); ++i) {
        for (size_t j = 0; j < ARRAY_LEN(switch_e->switchh->fe[i]); ++j) {
            Port *port = &switch_e->switchh->fe[i][j];
            if (e->kind == EK_NIC || e->kind == EK_ACCESS_POINT) {
                if (port->conn == NULL) {
                    port->conn = (Entity *)arena_alloc(e->arena, sizeof(Entity));
                    memcpy(port->conn, e, sizeof(Entity));
                    return true;
                }
            }
        }
    }
    return false;
}
