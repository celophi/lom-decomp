#include "wmap_frame_render.h"
#include "wmap_map_display.h"
#include "wmap_sprite_render.h"
#include "wmap_resource_support.h"
#include "common.h"
#include "sdk/libgpu.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

#include "sdk/memory.h"
#include "gpu_packet.h"

/** @brief Parts per sprite frame. */
#define WMAP_SPRITE_PARTS_MAX 32
/** @brief Part flags: mirror horizontally, mirror vertically, and the palette index bits. */
#define WMAP_SPRITE_PART_FLIP_U 0x80
#define WMAP_SPRITE_PART_FLIP_V 0x40
#define WMAP_SPRITE_PART_CLUT_MASK 0x3F
/** @brief Shade range: 0x80 draws the texture unmodified, higher values fade it back down. */
#define WMAP_SPRITE_SHADE_NEUTRAL 0x80
#define WMAP_SPRITE_SHADE_MAX 0xFF
/** @brief Variant bits above the scale row select a texture page row (64 texels each). */
#define WMAP_SPRITE_VARIANT_ROW_SHIFT 8
#define WMAP_SPRITE_PAGE_ROW_HEIGHT 64

/** @brief Texture page and palettes of one sprite texture. */
typedef struct
{
    u8 unknown_00[8];
    u16 clut[8];
    u16 tpage;
    u16 unknown_1a;
} WmapSpriteTexture;

/** @brief One quad of a sprite frame; x/y are the low bytes of the offsets from the actor. */
typedef struct
{
    u8 x, y, u, v, width, height;
    s8 flags, texture, angle, blend, x_high, y_high;
} WmapSpritePart;

/** @brief A 16-bit part offset assembled from its low and high bytes. */
typedef union
{
    s16 value;
    u8 byte[2];
} WmapSpriteOffset;

extern WmapSpriteTexture g_wmap_sprite_textures[];

/**
 * @brief Advance sprite-part shading and append transformed textured quads.
 * @param actor Sprite actor; its current frame supplies the parts.
 * @param screen_position Packed horizontal and vertical screen coordinates.
 * @param texture_index Texture-page and palette resource index.
 * @param ot_index Ordering-table index for the emitted packets.
 * @param variant Scale-table selector with an optional vertical texture-page offset.
 */
void wmap_draw_actor_sprite(WmapSpriteActor* actor, s32 screen_position, s32 texture_index, s32 ot_index, s32 variant)
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
    WmapSpriteOffset part_x;
    WmapSpriteOffset part_y;
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
    u8 flipped_u;
    u8 left_u;
    u8 flipped_right_u;
    u8 right_u;
    u8 bottom_v;
    WmapQuadScale* scale;
    POLY_FT4* quad;
    WmapFrame* frame;
    WmapSpritePart* part;

    if (variant >= (1 << WMAP_SPRITE_VARIANT_ROW_SHIFT))
    {
        page_y_offset = ((variant >> WMAP_SPRITE_VARIANT_ROW_SHIFT) - 1) * WMAP_SPRITE_PAGE_ROW_HEIGHT;
        variant &= 0xFF;
    }
    else
    {
        page_y_offset = 0;
    }
    part = (WmapSpritePart*)actor->frame_data;
    current_shade = actor->shade;
    target_shade = actor->target_shade;
    part_count = *(s8*)part;
    /* The frame data is a part count byte followed by the parts. */
    part = (WmapSpritePart*)((u8*)part + 1);
    if (current_shade != target_shade)
    {
        if (target_shade < current_shade)
        {
            actor->shade -= actor->shade_step;
            if (actor->shade < 0)
            {
                actor->shade = 0;
            }
        }
        else
        {
            actor->shade += actor->shade_step;
            if (actor->shade > WMAP_SPRITE_SHADE_MAX)
            {
                actor->shade = WMAP_SPRITE_SHADE_MAX;
            }
            if (actor->shade < 0)
            {
                actor->shade = 0;
            }
        }
    }
    if (part_count < 1 || part_count > WMAP_SPRITE_PARTS_MAX)
    {
        func_80064F14();
        return;
    }
    shade = actor->shade;
    if (shade >= WMAP_SPRITE_SHADE_NEUTRAL)
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
    do
    {
        quad = (POLY_FT4*)g_wmap_current_frame->packet_cursor;
        if (part->angle != 0)
        {
            memset(translation, 0, sizeof(VECTOR));
            memset(rotation, 0, sizeof(SVECTOR));
            /* Signed byte angle in 1/256 turns, scaled to the 12-bit GTE angle. */
            rotation_vector.vz = part->angle << 4;
            TransMatrix(&matrix, translation);
            RotMatrix(rotation, &matrix);
            SetRotMatrix(&matrix);
            SetTransMatrix(&matrix);
            part_x.byte[0] = part->x;
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
            quad->x1 = transformed.vx + origin_x;
            quad->y1 = transformed.vy + origin_y;
            position.vx = 0;
            position.vy = part->height;
            position.vz = 0;
            gte_ldv0(&position);
            gte_rtv0tr();
            gte_stlvnl(&transformed);
            quad->x2 = transformed.vx + origin_x;
            quad->y2 = transformed.vy + origin_y;
            position.vx = part->width;
            position.vy = part->height;
            position.vz = 0;
            gte_ldv0(&position);
            gte_rtv0tr();
            gte_stlvnl(&transformed);
            quad->x3 = transformed.vx + origin_x;
            bottom_y = transformed.vy + origin_y;
        }
        else
        {
            high_x = part->x_high << 8;
            scale = &g_wmap_land_quad_scales[variant][actor->scale_index];
            scaled_x = (s8)part->x | high_x;
            width = part->width;
            high_y = part->y_high << 8;
            height = part->height;
            scaled_y = (s8)part->y | high_y;
            quad->x0 = screen_position + ((scaled_x * scale->x0) >> 8);
            scaled_right = scaled_x + width;
            quad->x1 = screen_position + ((scaled_right * scale->x1) >> 8);
            quad->x2 = screen_position + ((scaled_x * scale->x2) >> 8);
            quad->x3 = screen_position + ((scaled_right * scale->x3) >> 8);
            quad->y0 = screen_y + ((scaled_y * scale->y0) >> 8);
            quad->y1 = screen_y + ((scaled_y * scale->y1) >> 8);
            scaled_bottom = scaled_y + height;
            quad->y2 = screen_y + ((scaled_bottom * scale->y2) >> 8);
            bottom_y = screen_y + ((scaled_bottom * scale->y3) >> 8);
        }
        quad->y3 = bottom_y;
        if (part->flags & WMAP_SPRITE_PART_FLIP_U)
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
        if (part->flags & WMAP_SPRITE_PART_FLIP_V)
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
        SET_BGR0_PACKED(quad, brightness | (brightness << 8) | (brightness << 16));
        quad->tpage = g_wmap_sprite_textures[texture_index + part->texture].tpage;
        quad->clut = g_wmap_sprite_textures[texture_index].clut[part->flags & WMAP_SPRITE_PART_CLUT_MASK];
        setPolyFT4(quad);
        if ((brightness != WMAP_SPRITE_SHADE_NEUTRAL) || (part->blend != 0))
        {
            setSemiTrans(quad, 1);
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
    } while (part_count != 0);
}
