#include "common.h"
#include "sdk/libgpu.h"

/** @brief Eight-word animation lookup table copied to local storage. */
typedef struct
{
    s32 words[8];
} FieldQuadAnimationTable;

extern FieldQuadAnimationTable D_800513E8, D_80051408, D_80051428, D_80051448;
extern s32 g_frame_counter;

/**
 * @brief Append an animated textured quad to an ordering table.
 * @param ordering_table Ordering-table entry receiving the primitive.
 * @param prim Writable primitive buffer.
 * @param x Horizontal origin.
 * @param y Vertical origin.
 * @return Buffer address immediately after the emitted primitive.
 */
void *func_800A8660(s32 *ordering_table, POLY_FT4 *prim, s32 x, s32 y)
{
    s32 frame;
    u32 color;
    FieldQuadAnimationTable tables[4];
    u16 left_x;
    u16 right_x;
    u16 width;
    u8 texture_u;
    s32 phase_index;

    tables[0] = D_800513E8;
    tables[1] = D_80051408;
    tables[2] = D_80051428;
    tables[3] = D_80051448;
    color = 0x808080;
    frame = g_frame_counter;
    *(u32 *)&prim->r0 = color;
    setlen(prim, 9);
    setcode(prim, 0x2C);
    phase_index = ((frame >> 2) + 3) & 7;
    left_x = *(u16 *)&tables[1].words[phase_index] + x;
    prim->x2 = left_x;
    prim->x0 = left_x;
    width = *(u16 *)&tables[3].words[phase_index];
    prim->y1 = y;
    prim->y0 = y;
    y += 0x10;
    prim->y3 = y;
    prim->y2 = y;
    right_x = left_x + width;
    prim->x3 = right_x;
    prim->x1 = right_x;
    texture_u = *(u8 *)&tables[2].words[phase_index];
    prim->u2 = texture_u;
    prim->u0 = texture_u;
    texture_u += *(u8 *)&tables[3].words[phase_index];
    prim->u3 = texture_u;
    prim->u1 = texture_u;
    prim->v1 = 0x80;
    prim->v0 = 0x80;
    prim->v3 = 0x90;
    prim->v2 = 0x90;
    prim->clut = 0x7A87;
    prim->tpage = 0x26;
    addPrim(ordering_table, prim);
    return prim + 1;
}
