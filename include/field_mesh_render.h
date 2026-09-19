#ifndef FIELD_MESH_RENDER_H
#define FIELD_MESH_RENDER_H

#include "field_effect_types.h"

s32 field_update_actor_palette_animation(FieldActorState *actor);
s32 *field_render_effect_mesh(FieldMotionRecord *effect, s32 mesh_index, s32 *packet_cursor, s32 *ordering_table);
s32 *field_render_lit_effect_mesh(FieldMotionRecord *effect, s32 mesh_index, s32 *packet_cursor, s32 *ordering_table);

#endif
