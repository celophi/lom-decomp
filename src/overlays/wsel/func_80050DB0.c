#include "common.h"
#include "sdk/libgpu.h"

typedef struct {
    u8 _pad0[4];
    u16 x1;
    u16 y1;
    u16 x2;
    u16 y2;
    u8 _pad1[0x18 - 0xC];
} WselTexEntry;

typedef struct {
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
    s16 x3, y3;
} WselQuadCoords;

extern WselTexEntry D_800C6720[];

void *func_80050DB0(POLY_FT4 *poly, u_long *ot, WselQuadCoords *coords, s32 semi, s32 color)
{
    setPolyFT4(poly);
    poly->b0 = color;
    poly->g0 = color;
    poly->r0 = color;
    setSemiTrans(poly, semi);

    poly->x0 = coords->x0;
    poly->x1 = coords->x1;
    poly->x2 = coords->x2;
    poly->x3 = coords->x3;
    poly->y0 = coords->y0;
    poly->y1 = coords->y1;
    poly->y2 = coords->y2;
    poly->y3 = coords->y3;

    poly->u2 = 0x60;
    poly->u0 = 0x60;
    poly->u3 = 0xE0;
    poly->u1 = 0xE0;
    poly->v1 = 0x30;
    poly->v0 = 0x30;
    poly->v3 = 0xA8;
    poly->v2 = 0xA8;

    setClut(poly, D_800C6720[1].x2, D_800C6720[1].y2);
    setTPage(poly, 1, 1, D_800C6720[1].x1, D_800C6720[1].y1);
    addPrim(ot, poly);
    return poly + 1;
}
