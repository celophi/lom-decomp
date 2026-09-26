/** @file field_text_quad.c
 * @brief Draw text as a scaled textured quad through a VRAM scratch area.
 */

#include "common.h"
#include "field_calls.h"
#include "display.h"
#include "field_text.h"
#include "gpu_packet.h"
#include "sdk/libgpu.h"
#include "sdk/memory.h"

/** @brief Alignment values in the low bits of the draw flags; 0 left-aligns on x. */
#define FIELD_TEXT_QUAD_ALIGN_RIGHT 1
#define FIELD_TEXT_QUAD_ALIGN_CENTER 2
#define FIELD_TEXT_QUAD_ALIGN_MASK 0x7F

/** @brief Draw flag that adds a black outline around the quad. */
#define FIELD_TEXT_QUAD_OUTLINE 0x80

/** @brief VRAM x of the scratch texture area, right of the display buffers (16-bit units). */
#define FIELD_TEXT_QUAD_SCRATCH_X SCREEN_WIDTH

/** @brief Width of one scratch texture row in 16-bit VRAM units (256 4bpp texels). */
#define FIELD_TEXT_QUAD_SCRATCH_WIDTH 64

/** @brief Scratch texture rows are 16 lines high. */
#define FIELD_TEXT_QUAD_ROW_SHIFT 4
#define FIELD_TEXT_QUAD_ROW_HEIGHT (1 << FIELD_TEXT_QUAD_ROW_SHIFT)

/** @brief VRAM origin of the text cache that field_text_build_sprites typesets into. */
#define FIELD_TEXT_CACHE_VRAM_X 960
#define FIELD_TEXT_CACHE_VRAM_Y 256

/** @brief 4bpp texels per 16-bit VRAM unit. */
#define FIELD_TEXT_TEXELS_PER_VRAM_UNIT 4

/** @brief Number of one-pixel-offset copies that form the outline. */
#define FIELD_TEXT_QUAD_OUTLINE_COPIES 4

/** @brief Scale factors are fixed point with this unit (1.0). */
#define FIELD_TEXT_QUAD_SCALE_ONE 256

static POLY_FT4* field_text_add_quad_outline(u32* ot, POLY_FT4* output);

/**
 * @brief Render text through a scratch texture as a scaled, optionally outlined quad.
 * @param output First free output primitive.
 * @param ot Ordering-table entry.
 * @param text Text to render.
 * @param style Text style passed to field_text_build_sprites.
 * @param x Horizontal position of the aligned edge.
 * @param y Top edge.
 * @param flags Alignment in the low seven bits, plus FIELD_TEXT_QUAD_OUTLINE.
 * @param scratch_row Scratch texture row the text is staged in.
 * @param x_scale Horizontal scale in units of 1/256.
 * @param y_scale Vertical scale in units of 1/256.
 * @param lower_x_offset Horizontal offset of the lower quad edge before scaling (slants the text).
 * @param add_fade_copy Nonzero to also queue a copy of the quad in the fading-primitive pool.
 * @return First free primitive after the text and optional outline.
 * @note Only text that fits in one sprite is drawn; longer text draws nothing.
 */
POLY_FT4* field_text_draw_scaled_quad(POLY_FT4* output, u32* ot, u8* text, s32 style, s32 x, s32 y, s32 flags, s32 scratch_row, s32 x_scale, s32 y_scale,
                                      s32 lower_x_offset, s32 add_fade_copy)
{
    SPRT sprites[10];
    RECT rect;
    s32 sprite_count;
    s32 scratch_y;
    s32 lower_left_x;
    s32 bottom_y;
    s32 texture_v;
    u8 texture_u_end;

    field_text_reset_scratch();
    sprite_count = field_text_build_sprites(sprites, text, style);
    field_text_upload_immediate_cache();
    if (sprite_count < 2)
    {
        scratch_y = scratch_row << FIELD_TEXT_QUAD_ROW_SHIFT;
        setRECT(&rect, FIELD_TEXT_QUAD_SCRATCH_X, scratch_y, FIELD_TEXT_QUAD_SCRATCH_WIDTH, FIELD_TEXT_QUAD_ROW_HEIGHT);
        ClearImage(&rect, 0, 0, 0);
        DrawSync(0);
        rect.x = (sprites[0].u0 >> 2) + FIELD_TEXT_CACHE_VRAM_X;
        rect.y = sprites[0].v0 + FIELD_TEXT_CACHE_VRAM_Y;
        rect.w = sprites[0].w / FIELD_TEXT_TEXELS_PER_VRAM_UNIT;
        rect.h = sprites[0].h;
        MoveImage(&rect, FIELD_TEXT_QUAD_SCRATCH_X, scratch_y);
        switch (flags & FIELD_TEXT_QUAD_ALIGN_MASK)
        {
        case FIELD_TEXT_QUAD_ALIGN_CENTER:
            x -= (sprites[0].w >> 1) * x_scale / FIELD_TEXT_QUAD_SCALE_ONE;
            break;
        case FIELD_TEXT_QUAD_ALIGN_RIGHT:
            x -= sprites[0].w * x_scale / FIELD_TEXT_QUAD_SCALE_ONE;
            break;
        }
        SET_BGR0_PACKED(output, GPU_TINT_NEUTRAL);
        setPolyFT4(output);
        output->x0 = x;
        output->x1 = x + sprites[0].w * x_scale / FIELD_TEXT_QUAD_SCALE_ONE;
        lower_left_x = x + lower_x_offset * x_scale / FIELD_TEXT_QUAD_SCALE_ONE;
        output->x2 = lower_left_x;
        output->x3 = lower_left_x + sprites[0].w * x_scale / FIELD_TEXT_QUAD_SCALE_ONE;
        output->y0 = output->y1 = y;
        bottom_y = y + (sprites[0].h - 1) * y_scale / FIELD_TEXT_QUAD_SCALE_ONE;
        texture_v = scratch_row << FIELD_TEXT_QUAD_ROW_SHIFT;
        output->y2 = output->y3 = bottom_y;
        output->u0 = output->u2 = 0;
        texture_u_end = sprites[0].w + 1;
        output->v0 = output->v1 = texture_v;
        output->u1 = output->u3 = texture_u_end;
        output->v2 = output->v3 = sprites[0].h + texture_v - 1;
        output->clut = sprites[0].clut;
        output->tpage = getTPage(0, 1, FIELD_TEXT_QUAD_SCRATCH_X, 0);
        addPrim(ot, output);
        if (add_fade_copy != 0)
        {
            field_add_fade_prim(output, 0);
        }
        output++;
        if (flags & FIELD_TEXT_QUAD_OUTLINE)
        {
            output = field_text_add_quad_outline(ot, output);
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
static POLY_FT4* field_text_add_quad_outline(u32* ot, POLY_FT4* output)
{
    POLY_FT4* poly;
    s32 i;

    poly = output;
    for (i = 0; i < FIELD_TEXT_QUAD_OUTLINE_COPIES; i++)
    {
        bcopy((u8*)(output - 1), (u8*)poly, sizeof(POLY_FT4));
        SET_BGR0(poly, 0, 0, 0);
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
    }
    return poly;
}
