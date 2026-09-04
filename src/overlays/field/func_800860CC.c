#include "common.h"
#include "sdk/libgpu.h"

extern s32 D_8010A004;
extern s32 D_8010A010;

POLY_F4 *func_800860CC(POLY_F4 *prim, s32 y, u32 *ot)
{
    if (prim->x1 == prim->x0) {
        prim->x1 = prim->x0 + 1;
    }
    *((u32 *)&prim->r0) = 0xFFFFFF;
    ((P_TAG *)prim)->len = 5, ((P_TAG *)prim)->code = 0x28;
    prim->x3 = prim->x1 - 3;
    prim->y1 = ((u16)D_8010A010) + y;
    prim->x2 = prim->x0 - 3;
    prim->y0 = ((u16)D_8010A010) + y;
    prim->y3 = prim->y1 + ((u16)D_8010A004);
    prim->y2 = prim->y3;
    ((P_TAG *)prim)->addr = (u32)((P_TAG *)ot)->addr,
        ((P_TAG *)ot)->addr = (u32)prim;
    return prim + 1;
}
