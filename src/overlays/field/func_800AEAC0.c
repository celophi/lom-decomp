#include "common.h"
#include "sdk/libgpu.h"


/**
 * @brief Build the textured coordinate icon primitives and append them to the ordering table.
 * @param handle Primitive buffer cursor.
 * @param ordering_table Ordering table entry that receives the generated primitives.
 * @param selector Selects the texture coordinates and CLUT.
 * @param x Horizontal screen coordinate.
 * @param y Vertical screen coordinate.
 * @param flip Nonzero to mirror the texture horizontally.
 * @return Primitive buffer cursor immediately after the generated draw-mode packet.
 */
s32 func_800AEAC0(POLY_FT4 *handle, u_long *ordering_table, s32 selector, s32 x, s32 y, s32 flip)
{
    POLY_FT4 *initial;
    POLY_FT4 *prim;
    POLY_FT4 *source;
    DR_TPAGE *draw_mode;
    u8 right_u;
    u8 u_value;

    initial = handle;
    prim = handle;
    prim++;
    prim--;
    *(u32 *)&initial->r0 = 0x808080;
    initial->x0 = x;
    setPolyFT4(initial);
    initial->x2 = initial->x0;
    initial->x3 = initial->x0 + 0x2F;
    initial->x1 = initial->x3;
    initial->y0 = y;
    initial->y1 = y;
    initial->y3 = initial->y0 + 0x2F;
    initial->y2 = initial->y3;

    if (selector == 2)
    {
        initial->u0 = 0xA0;
    }
    else
    {
        initial->u0 = 0xD0;
    }

    if (selector == 0)
    {
        prim->v0 = 0x20;
    }
    else
    {
        prim->v0 = 0x50;
    }

    prim->u2 = prim->u0;
    prim->u3 = prim->u0 + 0x2F;
    prim->u1 = prim->u3;
    prim->v1 = prim->v0;
    prim->v3 = prim->v0 + 0x2F;
    prim->v2 = prim->v3;
    if (flip != 0)
    {
        u_value = prim->u0;
        do
        {
            right_u = prim->u3;
        } while (0);
        prim->u1 = u_value;
        u_value = prim->u2;
        prim->u0 = right_u;
        prim->u2 = prim->u3;
        prim->u3 = u_value;
    }

    prim->clut = ((selector + 0x1D8) << 6) | 0x11;
    prim->tpage = 0x1F;
    addPrim(ordering_table, prim);

    source = prim;
    prim++;
    bcopy(source, prim, sizeof(POLY_FT4));

    setlen(prim, 9);
    *(u32 *)&prim->r0 = 0;
    setcode(prim, 0x2E);
    prim->x0 = (u16)(prim->x0 + 2);
    prim->x1 = (u16)(prim->x1 + 2);
    prim->x2 = (u16)(prim->x2 + 2);
    prim->x3 = (u16)(prim->x3 + 2);
    prim->y0 = (u16)(prim->y0 + 2);
    prim->y1 = (u16)(prim->y1 + 2);
    prim->y2 = (u16)(prim->y2 + 2);
    prim->y3 = (u16)(prim->y3 + 2);
    addPrim(ordering_table, prim);

    prim++;
    draw_mode = (DR_TPAGE *)prim;
    setDrawTPage(draw_mode, 0, 0, 0x1F);
    addPrim(ordering_table, draw_mode);
    return (s32)(draw_mode + 1);
}
