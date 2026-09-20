#ifndef FIELD_ACTOR_SEQUENCE_RUNTIME_H
#define FIELD_ACTOR_SEQUENCE_RUNTIME_H

#include "field_effect_types.h"

void field_apply_sequence_displacement(FieldMotionRecord* object, s32 direction_x, s32 vertical_step, s32 direction_z);
void field_update_sequence_actor_binding(FieldMotionRecord* object, s32 release_actor);
s32 field_execute_actor_sequence(FieldMotionRecord* object, s32 script_index);
s32 field_allocate_sequence_actor(s32 index, s32 flags);
void field_restart_sequence_animation(FieldMotionRecord* object);
void field_update_object_tints(void);

#endif
