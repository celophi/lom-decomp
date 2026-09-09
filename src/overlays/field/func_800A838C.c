#include "common.h"
#include "sdk/libgpu.h"

extern s32 g_frame_counter;

/**
 * @brief Build and link an animated textured quad primitive.
 * @param ordering_table Ordering-table tag to link the primitive into.
 * @param prim Primitive buffer slot to populate.
 * @param x Left screen coordinate.
 * @param y Top screen coordinate.
 * @param wide Selects the larger geometry and texture region when nonzero.
 * @return Pointer just past the emitted primitive.
 */
POLY_FT4* func_800A838C(u32* ordering_table, POLY_FT4* prim, s16 x, s16 y, s32 wide)
{
    s32 phase_table[8] = {0, 1, 2, 3, 2, 1, 0, 0};
    s32 phase;

    phase = phase_table[((g_frame_counter >> 2) + 1) & 7];

    *(u32*)&prim->r0 = 0x808080;
    setlen(prim, 9);
    setcode(prim, 0x2C);
    prim->x2 = x;
    prim->x0 = x;
    prim->y1 = y;
    prim->y0 = y;

    if (wide != 0)
    {
        prim->x3 = x + 12;
        prim->x1 = x + 12;
        prim->y3 = y + 24;
        prim->y2 = y + 24;
        prim->u2 = phase * 12 + 0x20;
        prim->u0 = phase * 12 + 0x20;
        prim->u3 = phase * 12 + 0x2C;
        prim->u1 = phase * 12 + 0x2C;
        prim->v1 = 0x60;
        prim->v0 = 0x60;
        prim->v3 = 0x78;
        prim->v2 = 0x78;
    }
    else
    {
        prim->x3 = x + 8;
        prim->x1 = x + 8;
        prim->y3 = y + 16;
        prim->y2 = y + 16;
        prim->u2 = phase * 8 - 0x48;
        prim->u0 = phase * 8 - 0x48;
        prim->u3 = phase * 8 - 0x40;
        prim->u1 = phase * 8 - 0x40;
        prim->v1 = 0x40;
        prim->v0 = 0x40;
        prim->v3 = 0x50;
        prim->v2 = 0x50;
    }

    prim->clut = 0x7A87;
    prim->tpage = 0x26;
    addPrim(ordering_table, prim);
    return prim + 1;
}
