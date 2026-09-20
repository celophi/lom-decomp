#ifndef FIELD_SCENE_TRANSITION_H
#define FIELD_SCENE_TRANSITION_H

#include "common.h"

void field_seek_scene_resource(s32 scene_selector);
void field_set_scene_parameters(s32 scene_id, s32 context_id, u32 spawn_id, s32 primary_layout, s32 layout_option, s32 secondary_layout);
void field_update_scene(void);
void field_move_actor_position(void* actor, void* motion);
void field_set_party_palettes(void);

#endif
