#ifndef FIELD_ACTOR_BEHAVIOR_H
#define FIELD_ACTOR_BEHAVIOR_H

#include "common.h"

struct FieldBehaviorActor;
struct FieldActionActor;
struct FieldMotionRecord;

s32 field_update_actor_input(struct FieldBehaviorActor* actor, s32 pad_index);
void field_prepare_actor_action(struct FieldActionActor* actor);
void field_update_actor_movement_animation(struct FieldMotionRecord* actor, s32 delta_x, s32 delta_z);
s32 field_update_actor_command(struct FieldBehaviorActor* actor);
s32 field_start_bound_action_animation(s32 object_index, s32 target_count, u8* targets, s32 flags);

#endif
