#ifndef FIELD_EFFECT_GEOMETRY_H
#define FIELD_EFFECT_GEOMETRY_H

#include "field_types.h"
#include "sdk/libgte.h"

struct FieldActorPartDef;
struct FieldActorState;
struct FieldMotionRecord;
struct FieldObjectPlacement;

void field_resolve_actor_part_anchor(struct FieldActorState *actor, struct FieldActorPartDef *part, Vec3i *out, s32 attachment_index);
void field_extract_effect_quad_corners8(struct FieldMotionRecord *effect, u8 *frame_data, s16 *corners);
void field_extract_effect_quad_corners16(struct FieldMotionRecord *effect, u8 *frame_data, s16 *corners);
void field_apply_effect_quad_center_offset(struct FieldMotionRecord *effect, DVECTOR *screen_origin, s16 *quad_bounds, s32 fallback_depth, s32 mode);
void field_transform_effect_quad_vertices8(struct FieldMotionRecord *effect, struct FieldObjectPlacement *object, u8 *quad_data, s32 vertex_index,
                                           Vec2s *screen_origin, SVECTOR *direction, FieldVector *gte_out);
void field_transform_effect_quad_vertices16(struct FieldMotionRecord *effect, struct FieldObjectPlacement *object, u8 *quad_data, s32 vertex_index,
                                            Vec2s *screen_origin, SVECTOR *direction, FieldVector *gte_out);

#endif
