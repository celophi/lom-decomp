#ifndef FIELD_EFFECT_TRANSFORM_H
#define FIELD_EFFECT_TRANSFORM_H

#include "field_types.h"
#include "sdk/libgpu.h"

/** @brief Primitive RGB and command byte, also copied as a single packed word. */
typedef union
{
    u32 word;
    s32 signed_word;
    u8 bytes[4];
    CVECTOR channels;
} FieldPrimitiveColor;

/** @brief Sprite resource prefix containing signed offsets, byte-sized geometry, UVs, and tilt. */
typedef struct
{
    s8 x;
    s8 y;
    u8 u;
    u8 v;
    u8 width;
    u8 height;
    u8 unknown_0x6;
    u8 flags;
    u8 tilt_16;
} FieldSpriteFrame;

struct FieldMotionRecord;
struct FieldActorPartDef;
struct FieldActorState;

u8* field_render_effect_ribbon(struct FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table);
s32 field_build_effect_part_matrix(struct FieldMotionRecord* effect, struct FieldActorPartDef* part, MATRIX* matrix, struct FieldActorState* actor);
void field_resolve_effect_part_color(struct FieldActorState* actor, struct FieldMotionRecord* effect, struct FieldActorPartDef* part, FieldPrimitiveColor* out);
u8* field_emit_effect_texture_page(struct FieldMotionRecord* effect, struct FieldActorPartDef* part, u8* packet_cursor, s32* ordering_table);
void field_project_effect_sprite_quad(struct FieldMotionRecord* effect, Vec2s* origin, POLY_FT4* quad, s32 width, s32 height, s32 x, s32 y,
                                      FieldSpriteFrame* frame, MATRIX* matrix);
void field_unpack_effect_quad_corners8(s16* out, s32 flip, s8* item);
void field_unpack_effect_quad_corners16(s16* out, s32 mirror, u8* item);
s32 field_resolve_effect_extent(struct FieldActorState* actor, struct FieldActorPartDef* part);

#endif
