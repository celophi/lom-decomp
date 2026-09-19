#ifndef FIELD_ANIMATION_H
#define FIELD_ANIMATION_H

#include "common.h"

struct FieldAnim;
struct FieldAnimCel;
struct FieldAnimDef;
struct FieldImageReq;
struct FieldPart;
struct FieldTintSrc;

void field_update_scene_animations(void);
void field_update_part_sweep(struct FieldPart* part);
void field_blit_animation_frame(struct FieldAnimDef* def, struct FieldAnim* anim, s32 frame);
void field_apply_animation_tween(struct FieldAnimDef* def, struct FieldAnim* anim, s32 apply_to_target);
void field_update_animation_sfx(struct FieldAnimDef* def, struct FieldAnim* anim);
void field_retarget_cel_cluts(struct FieldAnimDef* def, struct FieldAnimCel* cel, s32 frame);
u_long* field_blend_animation_frames(struct FieldAnimDef* def, struct FieldAnim* anim);
void field_tint_animation_cel(struct FieldAnimDef* def, struct FieldAnimCel* cel, struct FieldTintSrc* src, s32 shade);
void field_tint_animation_cel_list(struct FieldAnimDef* def, struct FieldTintSrc* src, s32 shade);
void field_advance_animation_keyframe(struct FieldAnimDef* def, struct FieldAnim* anim);
void field_retarget_cel_list_cluts(struct FieldAnimDef* def, struct FieldTintSrc* src, s32 frame);
u8* field_find_count_table_span(u8* table, s32 linear_index, volatile s8* range_start_out);
void field_queue_vram_upload(struct FieldImageReq* req);

#endif
