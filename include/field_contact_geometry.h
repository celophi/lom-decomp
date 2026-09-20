#ifndef FIELD_CONTACT_GEOMETRY_H
#define FIELD_CONTACT_GEOMETRY_H

#include "field_effect_types.h"
#include "vector.h"

/** @brief Packed screen point with signed 16-bit coordinates. */
typedef union
{
    s32 packed;
    Vec2s coord;
} FieldContactPoint;

/** @brief Actor index and packed point returned by a contact scan. */
typedef struct
{
    s32 actor;
    s32 point;
} FieldContactResult;

void field_load_actor_sequence_data(void);
s32 field_test_quad_actor_contacts(FieldContactPoint* quad, FieldMotionRecord* record, FieldContactResult* contact);
s32 field_intersect_screen_segments(Vec2s* first_start, Vec2s* first_end, Vec2s* second_start, Vec2s* second_end);
s32 field_resolve_actor_movement(FieldMotionRecord* actor, s32* position, s32 mode);
s32 field_find_actor_overlap(FieldMotionRecord* record, s32* position, s32 filter_group);
void field_start_actor_contact_interaction(FieldMotionRecord* record, s32 actor_index);
void field_probe_actor_interaction(FieldMotionRecord* entry);
s32 field_find_actor_in_range(s32* reference_position, s32 distance_limit, FieldActorState* source_actor, s32 opposing_group);
void field_collect_attack_sphere_hits(FieldActorState* actor, FieldActorPartDef* part);
s32 field_get_position_distance(VECTOR* a, VECTOR* b);

#endif
