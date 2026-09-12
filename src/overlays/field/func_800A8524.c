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
 */
void *func_800A8524(s32 *ordering_table, POLY_FT4 *prim, s32 x, s32 y)
{
    FieldQuadAnimationTable table;
    s32 phase_index;
    s32 frame;
    u32 color;
    s32 phase;
    u16 left_u;
    u16 right_u;

    table = D_800513E8;
    color = 0x808080;
    frame = g_frame_counter;
    phase_index = ((frame >> 2) + 2) & 7;
    phase = table.words[phase_index];

    *(u32 *)&prim->r0 = color;
    setlen(prim, 9);
    setcode(prim, 0x2C);
    prim->x2 = x;
    prim->x0 = x;
    prim->x3 = x + 8;
    prim->x1 = x + 8;
    prim->y1 = y;
    prim->y0 = y;
    prim->y3 = y + 8;
    prim->y2 = y + 8;
    prim->v1 = 0x78;
    prim->v0 = 0x78;
    prim->v3 = 0x80;
    prim->v2 = 0x80;
    prim->clut = 0x7A87;
    prim->tpage = 0x26;

    phase *= 8;
    left_u = phase + 0x20;
    right_u = phase + 0x28;
    prim->u3 = right_u;
    prim->u1 = right_u;
    prim->u2 = left_u;
    prim->u0 = left_u;

    prim->tag = (prim->tag & 0xFF000000) | (*ordering_table & 0xFFFFFF);
    *ordering_table = (*ordering_table & 0xFF000000) | ((s32)prim & 0xFFFFFF);
    return (u8 *)prim + 0x28;
}
