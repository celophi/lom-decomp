#ifndef FIELD_ACTOR_ROUTES_H
#define FIELD_ACTOR_ROUTES_H

#include "common.h"

/** @brief One signed X/Z point in a sampled route. */
typedef struct
{
    s16 x, z;
} FieldRoutePoint;

/** @brief Partial 0x54-byte actor record used for following, collision, and animation. */
typedef struct
{
    s32 x, y, z;
    u8 pad_c[10];
    s16 motion_divisor;
    u8 pad_18[4];
    union
    {
        u32 word;
        struct
        {
            u16 low;
            u16 high;
        } half;
    } flags;
    u8 pad_20;
    u8 animation;
    u8 pad_22[2];
    u8 animation_active, state, unknown_0x26, animation_frame;
    u8 pad_28[2];
    s16 action_id;
    u8 pad_2c[7];
    u8 movement_mode;
    u8 pad_34[2];
    s8 motion_remainder;
    u8 pad_37[3];
    u8 object_index, resource_index;
    u8 pad_3c[0x18];
} FieldRouteActor;

void func_8008C728(void);
void field_reset_leader_position_history(void);
void field_refresh_party_routes(void);
void field_sample_actor_route(FieldRoutePoint* points, s32 remaining, FieldRouteActor* actor, FieldRouteActor* target, s32 heading_offset, s32 unused_limit);
s32 field_get_route_heading_animation(FieldRouteActor* destination, FieldRouteActor* source);
void field_record_actor_position(FieldRouteActor* record);
void field_follow_leader_route(FieldRouteActor* actor, s32 follower_index);

#endif
