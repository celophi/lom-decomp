#include "common.h"
#include "sdk/libgpu.h"

extern void bcopy(void *, void *, s32);

s32 field_text_build_sprites(SPRT *, u8 *, s32);
void field_text_reset_scratch(void);
void func_80063194(void);
void func_80086F48(POLY_FT4 *, s32);
POLY_FT4 *func_800AFC50(u32 *, POLY_FT4 *);

/**
 * @brief Render text through a scratch texture as a scaled, optionally outlined quad.
 * @param output First free output primitive.
 * @param ot Ordering-table entry.
 * @param text Text to render.
 * @param style Text style.
 * @param x Horizontal position.
 * @param y Vertical position.
 * @param flags Alignment and outline flags.
 * @param scratch_row Scratch texture row.
 * @param x_scale Horizontal scale in units of 1/256.
 * @param y_scale Vertical scale in units of 1/256.
 * @param lower_x_offset Horizontal offset of the lower quad edge before scaling.
 * @param adjust_primitive Enable primitive adjustment through func_80086F48.
 * @return First free primitive after the text and optional outline.
 */
POLY_FT4 *func_800AF950(POLY_FT4 *output, u32 *ot, u8 *text, s32 style, s32 x, s32 y, s32 flags, s32 scratch_row, s32 x_scale, s32 y_scale, s32 lower_x_offset, s32 adjust_primitive)
{
    SPRT sprites[10];
    RECT rect;
    s32 glyph_count;
    s32 scratch_y;
    s32 sprite_width;
    s32 align_width;
    s32 scaled_align_width;
    s32 scaled_width;
    s32 scaled_lower_offset;
    s32 lower_left_x;
    s32 scaled_lower_width;
    s32 scaled_height;
    s32 bottom_y;
    s32 texture_v;
    u8 texture_u_max;
    u8 texture_v_max;
    s32 alignment;

    field_text_reset_scratch();
    glyph_count = field_text_build_sprites(sprites, text, style);
    func_80063194();
    if (glyph_count < 2)
    {
        rect.x = 0x140;
        scratch_y = scratch_row << 4;
        rect.y = scratch_y;
        rect.w = 0x40;
        rect.h = 0x10;
        ClearImage(&rect, 0, 0, 0);
        DrawSync(0);
        sprite_width = sprites[0].w;
        rect.x = (sprites[0].u0 >> 2) + 0x3C0;
        rect.y = sprites[0].v0 + 0x100;
        if (sprite_width < 0)
        {
            sprite_width += 3;
        }
        rect.w = (s16) (sprite_width >> 2);
        rect.h = (u16) sprites[0].h;
        MoveImage(&rect, 0x140, scratch_y);
        alignment = flags & 0x7F;
        if (alignment != 1)
        {
            if (alignment == 2)
            {
                align_width = (s32) ((u16) sprites[0].w << 0x10) >> 0x11;
                goto apply_alignment;
            }
        }
        else
        {
            align_width = (s32) sprites[0].w;
apply_alignment:
            scaled_align_width = align_width * x_scale;
            if (scaled_align_width < 0)
            {
                scaled_align_width += 0xFF;
            }
            x -= scaled_align_width >> 8;
        }
        *(s32 *) &output->r0 = 0x808080;
        setPolyFT4(output);
        output->x0 = (s16) x;
        scaled_width = sprites[0].w * x_scale;
        if (scaled_width < 0)
        {
            scaled_width += 0xFF;
        }
        scaled_lower_offset = lower_x_offset * x_scale;
        output->x1 = (s16) (x + (scaled_width >> 8));
        if (scaled_lower_offset < 0)
        {
            scaled_lower_offset += 0xFF;
        }
        lower_left_x = x + (scaled_lower_offset >> 8);
        output->x2 = lower_left_x;
        scaled_lower_width = sprites[0].w * x_scale;
        if (scaled_lower_width < 0)
        {
            scaled_lower_width += 0xFF;
        }
        output->x3 = (s16) (lower_left_x + (scaled_lower_width >> 8));
        output->y1 = (s16) y;
        output->y0 = (s16) y;
        scaled_height = ((s16) sprites[0].h - 1) * y_scale;

        if (scaled_height < 0)
        {
            scaled_height += 0xFF;
        }
        bottom_y = y + (scaled_height >> 8);
        texture_v = scratch_row * 0x10;
        output->y3 = bottom_y;
        output->y2 = bottom_y;
        output->u2 = 0;
        output->u0 = 0;
        texture_u_max = (u8) sprites[0].w;
        output->v1 = texture_v;
        output->v0 = texture_v;
        texture_u_max += 1;
        output->u3 = texture_u_max;
        output->u1 = texture_u_max;
        texture_v_max = ((u8) sprites[0].h + texture_v) - 1;
        output->v3 = texture_v_max;
        output->v2 = texture_v_max;
        output->clut = sprites[0].clut;
        output->tpage = 0x25;
        addPrim(ot, output);
        if (adjust_primitive != 0)
        {
            func_80086F48(output, 0);
        }
        output += 1;
        if (flags & 0x80)
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
 * @note 100% match with GCC 2.7.2 CDK: 113 instructions, 452 bytes.
 */
POLY_FT4 *func_800AFC50(u32 *ot, POLY_FT4 *output)
{
    POLY_FT4 *source = output;
    POLY_FT4 *poly = output;
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
