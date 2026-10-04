#ifndef FIELD_ANIMATION_H
#define FIELD_ANIMATION_H

#include "common.h"
#include "sdk/libgpu.h"

struct FieldAnim;
struct FieldAnimDef;
/** @brief Pending image upload and its inline VRAM destination rectangle. */
typedef struct FieldImageReq FieldImageReq;
struct FieldImageReq
{
    FieldImageReq* next;
    RECT rect;
    u_long* data;
};
struct FieldPart;
struct FieldTintSrc;
struct FieldTweenSpan;

void field_update_scene_animations(void);
void field_blit_animation_frame(struct FieldAnimDef* def, struct FieldAnim* anim, s32 frame);
void field_apply_animation_tween(struct FieldAnimDef* def, struct FieldAnim* anim, s32 apply_to_target);
void field_tint_animation_cel(struct FieldAnimDef* def, struct FieldPart* cel, struct FieldTintSrc* src, s32 shade);
struct FieldTweenSpan* field_find_count_table_span(struct FieldAnimDef* def, s32 keyframe, u8* range_start_out);
void field_queue_vram_upload(struct FieldImageReq* req);

#endif
