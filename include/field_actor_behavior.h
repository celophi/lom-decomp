#ifndef FIELD_ACTOR_BEHAVIOR_H
#define FIELD_ACTOR_BEHAVIOR_H

#include "common.h"

/* Defined in src/overlays/field/field_actor_tables.h. */
struct FieldActor;

s32 field_update_actor_input(struct FieldActor* actor, s32 pad_index);
void field_prepare_actor_action(struct FieldActor* actor);
void field_update_actor_movement_animation(struct FieldActor* record, s32 delta_x, s32 delta_z);
s32 field_update_actor_command(struct FieldActor* actor);
s32 field_start_bound_action_animation(s32 object_index, s32 target_count, u8* targets, s32 request);

#endif
