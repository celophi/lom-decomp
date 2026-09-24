/** @file field_draw_state.c
 * @brief Reset field drawing state and emit its debug load bar.
 */

#include "common.h"
#include "field_runtime.h"
#include "sdk/libgpu.h"

void field_reset_effect_pool(void);
void field_reset_global_color_scale(void);

extern s32 D_800F2278;
extern s32 D_800F227C;
extern s32 D_800F2280;

/** @brief LINE_F2 with its color word addressable as one value. */
typedef struct
{
    u_long tag;
    u32 rgbc;
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
} FieldBarLine;

/**
 * @brief Clear the three field draw-state globals and reset the color scale.
 * @see decomp.me (100%) TODO
 */
void func_80067AA4(void)
{
    D_800F2280 = 0;
    D_800F227C = 0;
    D_800F2278 = 0;
    field_reset_effect_pool();
    field_reset_global_color_scale();
}

/**
 * @brief Draw a horizontal bar whose length and color show @p load.
 *
 * The bar is grey below 0x200, yellow from 0x200 and red from 0x300, and is
 * @p load / 4 pixels long at y = 16.
 *
 * @param ctx Render half whose first ordering-table entry receives the line.
 * @param load Value the bar represents.
 * @see decomp.me (100%) TODO
 */
void func_80067AE0(FieldRenderHalf* ctx, s32 load)
{
    FieldBarLine* line = (FieldBarLine*)ctx->primitive_cursor;

    line->rgbc = 0x808080;
    if (load >= 0x200)
    {
        line->rgbc = 0x8080;
    }
    if (load >= 0x300)
    {
        line->rgbc = line->rgbc >> 8;
    }
    setLineF2(line);
    line->y1 = 16;
    line->y0 = 16;
    line->x0 = 0;
    line->x1 = load >> 2;
    addPrim(&ctx->ordering_table[0], line);
    line += 1;
    ctx->primitive_cursor = (u8*)line;
}
