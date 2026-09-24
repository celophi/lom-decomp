/** @file field_text_quad.c
 * @brief Draw text as a scaled textured quad through a VRAM scratch area.
 */

#include "common.h"
#include "display.h"
#include "field_text.h"
#include "gpu_packet.h"
#include "sdk/libgpu.h"

/** @brief Alignment value that right-aligns the quad on @p x. */
#define FIELD_TEXT_QUAD_ALIGN_RIGHT 1

/** @brief Alignment value that centers the quad on @p x. */
#define FIELD_TEXT_QUAD_ALIGN_CENTER 2

/** @brief Flag bit that adds a black outline around the quad. */
#define FIELD_TEXT_QUAD_OUTLINE 0x80

extern void bcopy(void*, void*, s32);

void func_80086F48(POLY_FT4*, s32);
POLY_FT4* func_800AFC50(u32*, POLY_FT4*);

/**
 * @brief Render text through a scratch texture as a scaled, optionally outlined quad.
 * @param output First free output primitive.
 * @param ot Ordering-table entry.
 * @param text Text to render.
 * @param style Text style.
 * @param x Horizontal position.
 * @param y Vertical position.
 * @param flags Alignment in the low seven bits, plus FIELD_TEXT_QUAD_OUTLINE.
 * @param scratch_row Scratch texture row.
 * @param x_scale Horizontal scale in units of 1/256.
 * @param y_scale Vertical scale in units of 1/256.
 * @param lower_x_offset Horizontal offset of the lower quad edge before scaling.
 * @param adjust_primitive Enable primitive adjustment through func_80086F48.
 * @return First free primitive after the text and optional outline.
 */
POLY_FT4* func_800AF950(POLY_FT4* output, u32* ot, u8* text, s32 style, s32 x, s32 y, s32 flags, s32 scratch_row, s32 x_scale, s32 y_scale, s32 lower_x_offset,
                        s32 adjust_primitive)
{
    SPRT sprites[10];
    RECT rect;
    s32 glyph_count;
    s32 scratch_y;
    s32 lower_left_x;
    s32 bottom_y;
    s32 texture_v;
    u8 texture_u_max;
    u8 texture_v_max;
    s32 alignment;

    field_text_reset_scratch();
    glyph_count = field_text_build_sprites(sprites, text, style);
    field_text_upload_immediate_cache();
    if (glyph_count < 2)
    {
        rect.x = 0x140;
        scratch_y = scratch_row << 4;
        rect.y = scratch_y;
        rect.w = 0x40;
        rect.h = 0x10;
        ClearImage(&rect, 0, 0, 0);
        DrawSync(0);
        rect.x = (sprites[0].u0 >> 2) + 0x3C0;
        rect.y = sprites[0].v0 + 0x100;
        rect.w = sprites[0].w / 4;
        rect.h = (u16)sprites[0].h;
        MoveImage(&rect, 0x140, scratch_y);
        alignment = flags & 0x7F;
        if (alignment != FIELD_TEXT_QUAD_ALIGN_RIGHT)
        {
            if (alignment == FIELD_TEXT_QUAD_ALIGN_CENTER)
            {
                x -= (sprites[0].w >> 1) * x_scale / 256;
            }
        }
        else
        {
            x -= sprites[0].w * x_scale / 256;
        }
        SET_BGR0_PACKED(output, GPU_TINT_NEUTRAL);
        setPolyFT4(output);
        output->x0 = (s16)x;
        output->x1 = x + sprites[0].w * x_scale / 256;
        lower_left_x = x + lower_x_offset * x_scale / 256;
        output->x2 = lower_left_x;
        output->x3 = lower_left_x + sprites[0].w * x_scale / 256;
        output->y1 = (s16)y;
        output->y0 = (s16)y;
        bottom_y = y + (sprites[0].h - 1) * y_scale / 256;
        texture_v = scratch_row * 0x10;
        output->y3 = bottom_y;
        output->y2 = bottom_y;
        output->u2 = 0;
        output->u0 = 0;
        texture_u_max = (u8)sprites[0].w;
        output->v1 = texture_v;
        output->v0 = texture_v;
        texture_u_max += 1;
        output->u3 = texture_u_max;
        output->u1 = texture_u_max;
        texture_v_max = ((u8)sprites[0].h + texture_v) - 1;
        output->v3 = texture_v_max;
        output->v2 = texture_v_max;
        output->clut = sprites[0].clut;
        output->tpage = getTPage(0, 1, SCREEN_WIDTH, 0);
        addPrim(ot, output);
        if (adjust_primitive != 0)
        {
            func_80086F48(output, 0);
        }
        output += 1;
        if (flags & FIELD_TEXT_QUAD_OUTLINE)
        {
            output = func_800AFC50(ot, output);
        }
    }
    return output;
}

/**
 * @brief Add four black, one-pixel-offset copies of the preceding textured quad.
 * @param ot Ordering-table entry receiving the outline primitives.
 * @param output First free primitive slot, immediately after the quad to copy.
 * @return First free primitive slot after the four outline copies.
 */
POLY_FT4* func_800AFC50(u32* ot, POLY_FT4* output)
{
    POLY_FT4* source = output;
    POLY_FT4* poly = output;
    s32 i = 0;
    do
    {
        bcopy(source - 1, poly, 0x28);
        poly->b0 = 0;
        poly->g0 = 0;
        poly->r0 = 0;
        switch (i)
        {
        case 0:
            poly->x0++;
            poly->x1++;
            poly->x2++;
            poly->x3++;
            break;
        case 1:
            poly->x0--;
            poly->x1--;
            poly->x2--;
            poly->x3--;
            break;
        case 2:
            poly->y0++;
            poly->y1++;
            poly->y2++;
            poly->y3++;
            break;
        default:
            poly->y0--;
            poly->y1--;
            poly->y2--;
            poly->y3--;
            break;
        }
        addPrim(ot, poly);
        poly++;
        i++;
    } while (i < 4);
    return poly;
}
