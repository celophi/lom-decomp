#include "wmap_frame_render.h"
#include "wmap_map_display.h"
#include "wmap_sprite_render.h"
#include "wmap_resource_support.h"
#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

void* memset(void*, int, unsigned int);
typedef struct
{
    u8 pad00[6];
    s8 scale_index;
    u8 pad07[0x15];
    u8* parts;
    u8 pad20[2];
    s16 target_shade;
    s16 shade;
    u16 shade_step;
} SpriteActor;

typedef struct
{
    u8 pad00[8];
    u16 clut[8];
    u16 tpage;
    u16 pad1A;
} SpriteTexture;

typedef struct
{
    u8 x, y, u, v, width, height;
    s8 flags, texture, angle, blend, x_high, y_high;
} SpritePart;

typedef union
{
    s16 value;
    u8 byte[2];
} SpriteCoordinate;

extern SpriteTexture D_800D0A6C[];

/**
 * @brief Advance sprite-part shading and append transformed textured quads.
 * @param actor Actor configuration and sprite-part list.
 * @param screen_position Packed horizontal and vertical screen coordinates.
 * @param texture_index Texture-page and palette resource index.
 * @param ot_index Ordering-table index for the emitted packets.
 * @param variant Scale-table selector with an optional vertical texture-page offset.
 */
void wmap_draw_actor_sprite(void* actor, s32 screen_position, s32 texture_index, s32 ot_index, s32 variant)
{
    SVECTOR position;
    VECTOR transformed;
    VECTOR translation_vector;
    SVECTOR rotation_vector;
    MATRIX matrix;
    s32 page_y_offset;
    VECTOR* translation;
    SVECTOR* rotation;
    s16 current_shade;
    SpriteCoordinate part_x;
    SpriteCoordinate part_y;
    s16 origin_y;
    s16 origin_x;
    s16 target_shade;
    s16 shade;
    s32 brightness;
    s16 bottom_y;
    s32 high_x;
    s32 high_y;
    s32 width;
    s32 height;
    s32 scaled_y;
    s32 scaled_bottom;
    s32 scaled_right;
    s32 screen_y;
    s32 scaled_x;
    s8 flipped_v;
    s8 top_v;
    s32 part_count;
    u32 count_index;
    u8* part_bytes;
    u8 flipped_u;
    u8 left_u;
    u8 flipped_right_u;
    u8 right_u;
    u8 bottom_v;
    WmapQuadScale* scale;
    POLY_FT4* quad;
    WmapFrame* frame;
    SpritePart* part;

    if (variant >= 0x100)
    {
        page_y_offset = ((variant >> 8) - 1) << 6;
        variant &= 0xFF;
    }
    else
    {
        page_y_offset = 0;
    }
    part_bytes = (u8*)((SpriteActor*)actor)->parts;
    current_shade = ((SpriteActor*)actor)->shade;
    target_shade = ((SpriteActor*)actor)->target_shade;
    part_count = *(s8*)part_bytes++;
    if (current_shade != target_shade)
    {
        if (target_shade < current_shade)
        {
            ((SpriteActor*)actor)->shade -= ((SpriteActor*)actor)->shade_step;
            if (((SpriteActor*)actor)->shade < 0)
            {
                ((SpriteActor*)actor)->shade = 0;
            }
        }
        else
        {
            ((SpriteActor*)actor)->shade += ((SpriteActor*)actor)->shade_step;
            if (((SpriteActor*)actor)->shade >= 0x100)
            {
                ((SpriteActor*)actor)->shade = 0xFF;
            }
            if (((SpriteActor*)actor)->shade < 0)
            {
                ((SpriteActor*)actor)->shade = 0;
            }
        }
    }
    count_index = part_count - 1;
    if (count_index >= 0x20U)
    {
        func_80064F14();
        return;
    }
    shade = ((SpriteActor*)actor)->shade;
    if (shade >= 0x80)
    {
        brightness = 0x100 - shade;
    }
    else
    {
        brightness = shade;
    }
    translation = &translation_vector;
    rotation = &rotation_vector;
    screen_y = screen_position >> 0x10;
    part = (SpritePart*)part_bytes;
    do
    {
        quad = (POLY_FT4*)g_wmap_current_frame->packet_cursor;
        if (part->angle != 0)
        {
            memset(translation, 0, 0x10);
            memset(rotation, 0, 8);
            rotation_vector.vz = (s16)((s32)((u8)part->angle << 0x18) >> 0x14);
            TransMatrix(&matrix, translation);
            RotMatrix(rotation, &matrix);
            SetRotMatrix(&matrix);
            SetTransMatrix(&matrix);
            part_x.byte[0] = *part_bytes;
            part_x.byte[1] = part->x_high;
            part_y.byte[0] = part->y;
            part_y.byte[1] = part->y_high;
            origin_x = part_x.value + screen_position;
            quad->x0 = origin_x;
            origin_y = part_y.value + screen_y;
            quad->y0 = origin_y;
            position.vx = part->width;
            position.vy = 0;
            position.vz = 0;
            gte_ldv0(&position);
            gte_rtv0tr();
            gte_stlvnl(&transformed);
            quad->x1 = (s16)((u16)transformed.vx + origin_x);
            quad->y1 = (s16)((u16)transformed.vy + origin_y);
            position.vx = 0;
            position.vy = part->height;
            position.vz = 0;
            gte_ldv0(&position);
            gte_rtv0tr();
            gte_stlvnl(&transformed);
            quad->x2 = (s16)((u16)transformed.vx + origin_x);
            quad->y2 = (s16)((u16)transformed.vy + origin_y);
            position.vx = part->width;
            position.vy = part->height;
            position.vz = 0;
            gte_ldv0(&position);
            gte_rtv0tr();
            gte_stlvnl(&transformed);
            quad->x3 = (s16)((u16)transformed.vx + origin_x);
            bottom_y = (u16)transformed.vy + origin_y;
        }
        else
        {
            high_x = part->x_high << 8;
            scale = &((WmapQuadScale(*)[16])g_wmap_land_quad_scales)[variant][((SpriteActor*)actor)->scale_index];
            scaled_x = (s8)*part_bytes | high_x;
            width = part->width;
            high_y = part->y_high << 8;
            height = part->height;
            scaled_y = (s8)part->y | high_y;
            quad->x0 = (s16)(screen_position + ((s32)(scaled_x * scale->x0) >> 8));
            scaled_right = scaled_x + width;
            quad->x1 = (s16)(screen_position + ((s32)(scaled_right * scale->x1) >> 8));
            quad->x2 = (s16)(screen_position + ((s32)(scaled_x * scale->x2) >> 8));
            quad->x3 = (s16)(screen_position + ((s32)(scaled_right * scale->x3) >> 8));
            quad->y0 = (s16)(screen_y + ((s32)(scaled_y * scale->y0) >> 8));
            quad->y1 = (s16)(screen_y + ((s32)(scaled_y * scale->y1) >> 8));
            scaled_bottom = scaled_y + height;
            quad->y2 = (s16)(screen_y + ((s32)(scaled_bottom * scale->y2) >> 8));
            bottom_y = screen_y + ((s32)(scaled_bottom * scale->y3) >> 8);
        }
        quad->y3 = bottom_y;
        if (part->flags & 0x80)
        {
            flipped_u = part->u;
            quad->u3 = flipped_u;
            quad->u1 = flipped_u;
            flipped_right_u = part->u + part->width;
            quad->u2 = flipped_right_u;
            quad->u0 = flipped_right_u;
        }
        else
        {
            left_u = part->u;
            quad->u2 = left_u;
            quad->u0 = left_u;
            right_u = part->u + part->width;
            quad->u3 = right_u;
            quad->u1 = right_u;
        }
        if ((u8)part->flags & 0x40)
        {
            flipped_v = part->v + part->height;
            quad->v1 = flipped_v;
            quad->v0 = flipped_v;
            bottom_v = part->v;
            quad->v3 = bottom_v;
            quad->v2 = bottom_v;
        }
        else
        {
            top_v = part->v + page_y_offset;
            quad->v1 = top_v;
            quad->v0 = top_v;
            bottom_v = part->v + part->height + page_y_offset;
            quad->v3 = bottom_v;
            quad->v2 = bottom_v;
        }
        *(u32*)&quad->r0 = (s32)(brightness | (brightness << 8) | (brightness << 0x10));
        quad->tpage = D_800D0A6C[texture_index + part->texture].tpage;
        quad->clut = D_800D0A6C[texture_index].clut[(u8)part->flags & 0x3F];
        setlen(quad, 9);
        quad->code = 0x2C;
        if ((brightness != 0x80) || (part->blend != 0))
        {
            quad->code = 0x2E;
        }
        frame = g_wmap_current_frame;
        addPrim(&frame->ordering_table[ot_index], quad);
        if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
        {
            g_wmap_packet_bytes += sizeof(POLY_FT4);
            frame->packet_cursor += sizeof(POLY_FT4);
        }
        part++;
        part_count -= 1;
        part_bytes += sizeof(SpritePart);
    } while (part_count != 0);
}
