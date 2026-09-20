#ifndef FIELD_EFFECT_TRANSFORM_H
#define FIELD_EFFECT_TRANSFORM_H

#include "field_types.h"

struct FieldMotionRecord;
struct FieldActorPartDef;
struct FieldActorState;

u8* field_render_effect_ribbon(struct FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table);
s32 field_build_effect_part_matrix(struct FieldMotionRecord* effect, struct FieldActorPartDef* part, FieldMatrix* matrix, struct FieldActorState* actor);
void field_resolve_effect_part_color(struct FieldActorState* actor, struct FieldMotionRecord* effect, struct FieldActorPartDef* part, u8* out);
u8* field_emit_effect_texture_page(struct FieldMotionRecord* effect, struct FieldActorPartDef* part, u8* packet_cursor, s32* ordering_table);
void field_project_effect_sprite_quad(struct FieldMotionRecord* effect, u16* origin, u8* packet, s32 width, s32 height, s32 x, s32 y, u8* item,
                                      FieldMatrix* matrix);
void field_unpack_effect_quad_corners8(s16* out, s32 flip, u8* item);
void field_unpack_effect_quad_corners16(s16* out, s32 mirror, u8* item);
s32 field_resolve_effect_extent(struct FieldActorState* actor, struct FieldActorPartDef* part);

#endif
