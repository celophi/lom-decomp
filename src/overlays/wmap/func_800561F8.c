#include "wmap_resource_support.h"
#include "common.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

typedef struct
{
    u8 _pad00[0x70];
    u_long ordering_table[0xB3];
    u8* packet_cursor;
} WmapRenderContext;

typedef struct
{
    s32 x;
    s32 y;
    s32 projection_scale;
    s32 animation_offset;
    s32 texture_offset;
} WmapProjection;

typedef struct
{
    u8 _pad00[0x10];
    u8* animation_data;
} WmapSpriteResource;

typedef struct
{
    u8 _pad00[5];
    u8 phase_mode;
    u8 phase;
    u8 _pad07[7];
    s16 frame_index;
    s16 previous_frame_index;
    u8* animation_cursor;
    u8* animation_start;
    s8* quad_data;
    s16 frame_timer;
} WmapSpriteState;

typedef struct
{
    u8 sequence_id;
    u8 duration;
    u8 _pad02[2];
} WmapAnimationEntry;

typedef struct
{
    s8 x;
    s8 y;
    u8 u;
    u8 v;
    u8 width;
    u8 height;
    s8 texture_index;
    u8 _pad07[2];
    u8 blend_mode;
    u8 _pad0A[2];
} WmapQuadData;

typedef struct
{
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    s16 x2;
    s16 y2;
    s16 x3;
    s16 y3;
} WmapQuadScale;

typedef struct
{
    u8 _pad00[2];
    u8 v_offset;
    u8 _pad03[5];
    u16 clut[8];
    u16 tpage_x;
    u16 tpage_y;
} WmapTextureInfo;

extern s32 D_800500B4[];
extern WmapTextureInfo D_800CBBE8[];
extern WmapQuadScale D_800CBDC4[];
extern s32 D_800D921C;
extern WmapSpriteResource D_8011CF88[];
extern WmapRenderContext* D_801398EC;
extern WmapProjection D_80139950;


typedef struct
{
    SVECTOR position;
    volatile s32 blend_mode;
    DVECTOR screen;
    s32 depth;
} WmapSpriteLocals;

/**
 * @brief Update and render an animated world-map sprite.
 * @param x World-map X cell coordinate.
 * @param y World-map Y cell coordinate.
 * @param state Animation and rendering state for the sprite.
 * @param resource_index Index of the sprite resource and texture set.
 */
void func_800561F8(s32 x, s32 y, WmapSpriteState* state, s32 resource_index)
{
    WmapSpriteResource* resource;
    WmapQuadScale* scale;
    WmapTextureInfo* texture;
    WmapTextureInfo* texture_base;
    WmapAnimationEntry* animation;
    WmapQuadData* quad;
    POLY_FT4* packet;
    WmapSpriteLocals locals;
    DVECTOR* screen_ptr;
    s32 depth_index;
    s32 ot_depth;
    s32 count;
    s32 sequence_id;
    s32 sequence_end;
    s32 phase_delta;
    s32 projection_scale;
    s32 animation_offset;
    s32 texture_offset;
    u8* animation_data;
    u32 address_mask;
    u32 tag_mask;

    phase_delta = D_800500B4[state->phase_mode];
    state->phase += phase_delta;
    resource = &D_8011CF88[resource_index];

    if ((s8)state->phase >= 15)
    {
        state->phase = 15;
        state->phase_mode = 2;
    }

    if ((s8)state->phase <= 0)
    {
        state->phase_mode = 1;
        return;
    }

    projection_scale = D_80139950.projection_scale;
    locals.position.vx = (((x - 1) * 0xA0 - (D_80139950.x * 0x14000) / projection_scale) * 0x6000) / projection_scale;
    locals.position.vy = (((y - 1) * 0xA0 - (D_80139950.y * 0x14000) / projection_scale) * 0x6000) / projection_scale;
    locals.position.vz = 0;

    gte_ldv0(&locals.position);
    gte_rtps();
    {
        DVECTOR* projected_screen;

        projected_screen = &locals.screen;
        gte_stsxy(projected_screen);
        gte_stszotz(&locals.depth);

        if (((u16)projected_screen->vy << 16) < 0)
        {
            return;
        }
    }

    animation_data = resource->animation_data;
    if (state->previous_frame_index != state->frame_index)
    {
        state->previous_frame_index = state->frame_index;
        animation_offset = *(s16*)(animation_data + state->frame_index * 2);
        state->frame_timer = 1;
        state->animation_start = animation_data + animation_offset;
        state->animation_cursor = state->animation_start;
    }

    sequence_end = 0xFF;
    if (state->frame_timer != sequence_end)
    {
        state->frame_timer--;
    }

    if (state->frame_timer == 0)
    {
        animation = (WmapAnimationEntry*)state->animation_cursor;
        sequence_id = animation->sequence_id;
        state->frame_timer = animation->duration;
        if (sequence_id == sequence_end)
        {
            animation = (WmapAnimationEntry*)state->animation_start;
            state->animation_cursor = (u8*)animation;
            sequence_id = animation->sequence_id;
            state->frame_timer = animation->duration;
        }
        state->animation_cursor += sizeof(WmapAnimationEntry);
        state->quad_data = (s8*)(animation_data + *(s16*)(animation_data + 0x40 + sequence_id * 2));
    }

    quad = (WmapQuadData*)state->quad_data;
    count = *(s8*)quad;
    quad = (WmapQuadData*)((s8*)quad + 1);
    if ((u32)(count - 1) >= 32)
    {
        func_80064F14(animation_data, sequence_end, projection_scale, resource);
        return;
    }

    depth_index = (0x1B91 - locals.depth) / 4;
    ot_depth = depth_index + 0x2E;
    if ((u32)depth_index >= 0x81)
    {
        ot_depth = 0x2E;
    }

    do
    {
        packet = (POLY_FT4*)D_801398EC->packet_cursor;
        screen_ptr = &locals.screen;
        texture_base = D_800CBBE8;
        address_mask = 0x00FFFFFF;
        tag_mask = 0xFF000000;
        scale = &D_800CBDC4[(s8)state->phase];

        packet->x0 = locals.screen.vx + ((quad->x * scale->x0) >> 8);
        packet->x1 = locals.screen.vx + (((quad->x + (s8)quad->width) * scale->x1) >> 8);
        packet->x2 = locals.screen.vx + ((quad->x * scale->x2) >> 8);
        packet->x3 = locals.screen.vx + (((quad->x + (s8)quad->width) * scale->x3) >> 8);
        packet->y0 = screen_ptr->vy + ((quad->y * scale->y0) >> 8);
        packet->y1 = screen_ptr->vy + ((quad->y * scale->y1) >> 8);
        packet->y2 = screen_ptr->vy + (((quad->y + (s8)quad->height) * scale->y2) >> 8);
        packet->y3 = screen_ptr->vy + (((quad->y + (s8)quad->height) * scale->y3) >> 8);

        texture_offset = resource_index * sizeof(WmapTextureInfo);
        texture = (WmapTextureInfo*)((u8*)texture_base + texture_offset);
        packet->u0 = quad->u;
        packet->u1 = quad->u + quad->width;
        packet->u2 = quad->u;
        packet->u3 = quad->u + quad->width;
        packet->v0 = quad->v + texture->v_offset;
        packet->v1 = quad->v + texture->v_offset;
        packet->v2 = quad->v + quad->height + texture->v_offset;
        packet->v3 = quad->v + quad->height + texture->v_offset;
        *(u32*)&packet->r0 = 0x80808080;
        locals.blend_mode = (s8)quad->blend_mode;
        packet->tpage = getTPage(0, quad->blend_mode & 3, texture->tpage_x, texture->tpage_y);
        packet->clut = *(u16*)((u8*)texture_base + (quad->texture_index * 2 + texture_offset) + 8);
        setPolyFT4(packet);
        setSemiTrans(packet, 1);
        packet->tag = (packet->tag & tag_mask) | (ot_depth[D_801398EC->ordering_table] & address_mask);
        ot_depth[D_801398EC->ordering_table] = (ot_depth[D_801398EC->ordering_table] & tag_mask) | ((u32)packet & address_mask);

        if (D_800D921C < 0x7D00)
        {
            D_800D921C += sizeof(POLY_FT4);
            D_801398EC->packet_cursor += sizeof(POLY_FT4);
        }

        quad++;
        count--;
    } while (count != 0);
}
