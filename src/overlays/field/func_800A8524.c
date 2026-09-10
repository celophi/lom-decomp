#include "common.h"
#include "sdk/libgpu.h"

/** @brief Eight-word animation sequence copied to local storage. */
typedef struct
{
    s32 words[8];
} FieldQuadAnimationTable;

extern FieldQuadAnimationTable D_800513E8;
extern s32 g_frame_counter;

/**
 * @brief Build and link an animated eight-pixel textured quad.
 * @param ordering_table Ordering-table tag receiving the primitive.
 * @param prim Primitive buffer to populate.
 * @param x Left screen coordinate.
 * @param y Top screen coordinate.
 * @return Buffer address immediately after the emitted primitive.
 * @note GCC 2.7.2 CDK currently matches 90.886080 percent of the target.
 */
void *func_800A8524(s32 *ordering_table, POLY_FT4 *prim, s32 x, s32 y)
{
    FieldQuadAnimationTable table;
    s32 phase_index;
    s32 phase;
    u16 left_u;
    u16 right_u;

    table = D_800513E8;
    phase_index = ((g_frame_counter >> 2) + 2) & 7;
    phase = table.words[phase_index];

    setlen(prim, 9);
    prim->x2 = x;
    prim->x0 = x;
    x += 8;
    prim->y1 = y;
    prim->y0 = y;
    y += 8;
    *(u32 *)&prim->r0 = 0x808080;
    setcode(prim, 0x2C);
    prim->v1 = 0x78;
    prim->v0 = 0x78;
    prim->v3 = 0x80;
    prim->v2 = 0x80;
    prim->clut = 0x7A87;
    prim->x3 = x;
    prim->x1 = x;
    prim->tpage = 0x26;

    phase *= 8;
    left_u = phase + 0x20;
    right_u = phase + 0x28;
    prim->u3 = right_u;
    prim->u1 = right_u;
    prim->y3 = y;
    prim->y2 = y;
    prim->u2 = left_u;
    prim->u0 = left_u;

    prim->tag = (prim->tag & 0xFF000000) | (*ordering_table & 0xFFFFFF);
    *ordering_table = (*ordering_table & 0xFF000000) | ((s32) prim & 0xFFFFFF);
    return (u8 *) prim + 0x28;
}
