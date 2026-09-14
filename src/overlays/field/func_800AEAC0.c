#include "common.h"
#include "sdk/libgpu.h"


#define setXYWHX(p,_x0,_y0,_w,_h) \
    ((p)->x0 = (_x0), (p)->y0 = (_y0), \
     (p)->x1 = (_x0)+(_w), (p)->y1 = (_y0), \
     (p)->x2 = (_x0), (p)->y2 = (_y0)+(_h), \
     (p)->x3 = (_x0)+(_w), (p)->y3 = (_y0)+(_h))

#define setUVWHX(p,_u0,_v0,_w,_h) \
    ((p)->u0 = (_u0), (p)->v0 = (_v0), \
     (p)->u1 = (_u0)+(_w), (p)->v1 = (_v0), \
     (p)->u2 = (_u0), (p)->v2 = (_v0)+(_h), \
     (p)->u3 = (_u0)+(_w), (p)->v3 = (_v0)+(_h))


s32 func_800AEAC0(POLY_FT4 *handle, u_long *ordering_table, s32 selector, s32 x, s32 y, s32 flip)
{
    POLY_FT4 *initial;
    POLY_FT4 *prim;
    POLY_FT4 *source;
    DR_TPAGE *draw_mode;
    s32 left_x;
    s32 top_y;
    s32 right_x;
    s32 bottom_y;
    u8 top_v;
    u8 bottom_v;
    u8 swap_u;
    u8 right_u;
    u8 u_value;
    u8 v_value;

    initial = handle;
    prim = handle;
    *(u32 *)&initial->r0 = 0x808080;
    initial->x0 = x;
    setPolyFT4(initial);
    initial->x3 = initial->x0 + 0x2F;
    initial->x1 = initial->x3;
    initial->x2 = initial->x0;
    initial->y1 = y;
    initial->y0 = y;
    initial->y3 = initial->y1 + 0x2F;
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
        prim->u1 = prim->u0;
        right_u = prim->u1;
        swap_u = prim->u2;
        prim->u0 = right_u;
        prim->u2 = right_u;
        prim->u3 = swap_u;
    }

    prim->clut = ((selector + 0x1D8) << 6) | 0x11;
    prim->tpage = 0x1F;
    addPrim(ordering_table, prim);

    source = prim;
    handle++;
    prim = handle;
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

    handle++;
    prim = handle;
    draw_mode = (DR_TPAGE *)prim;
    setDrawTPage(draw_mode, 0, 0, 0x1F);
    addPrim(ordering_table, draw_mode);
    return (s32)(draw_mode + 1);
}
