#include "common.h"
#include "sdk/libgpu.h"

typedef struct {
    u8 tp;
    u8 abr;
    u8 semi;
    u8 color;
    u16 tx;
    u16 ty;
    u16 clut_x;
    u16 clut_y;
    u16 u;
    u16 v;
    u16 w;
    u16 h;
    u16 x;
    u16 y;
} WselTexEntry;

extern WselTexEntry D_800C6780;
extern WselTexEntry D_800C6798;
extern WselTexEntry D_800C67B0;
extern WselTexEntry D_800C67E0;
extern WselTexEntry D_800C6828;

POLY_FT4 *func_80051D78(POLY_FT4 *poly, u_long *ot, s32 which)
{
    WselTexEntry *frame;
    WselTexEntry *tex;

    if (which) {
        frame = &D_800C6828;
        tex = &D_800C6798;
    } else {
        frame = &D_800C67E0;
        tex = &D_800C6780;
    }

    *(u_long *)&poly->r0 = 0x00808080;
    setPolyFT4(poly);
    setSemiTrans(poly, tex->semi);

    poly->x2 = poly->x0 = tex->x + 0x20 - ((u8 *)frame)[4] * 8;
    poly->y1 = poly->y0 = tex->y + 0x28 - ((u8 *)frame)[5] * 8;
    poly->x1 = poly->x3 = poly->x0 + frame->semi * 8 - 1;
    poly->y2 = poly->y3 = poly->y0 + frame->color * 8 - 1;

    poly->u0 = poly->u2 = frame->tp * 8;
    poly->v1 = poly->v0 = frame->abr * 8;
    poly->u1 = poly->u3 = poly->u0 + frame->semi * 8 - 1;
    poly->v2 = poly->v3 = poly->v0 + frame->color * 8 - 1;

    setClut(poly, tex->clut_x, tex->clut_y);
    setTPage(poly, tex->tp, tex->abr, tex->tx, tex->ty);
    addPrim(ot, poly);
    poly++;

    poly->x2 = poly->x0 = tex->x;
    poly->y1 = poly->y0 = tex->y + 0x20;
    tex = &D_800C67B0;
    setlen(poly, 9);
    *(u_long *)&poly->r0 = 0x00808080;
    poly->code = 0x2c;
    setSemiTrans(poly, tex->semi);
    poly->u0 = poly->u2 = 0xb8;
    poly->v1 = poly->v0 = 6;
    poly->x1 = poly->x3 = poly->x0 + 0x20;
    poly->y2 = poly->y3 = poly->y0 + 0xa;
    poly->u1 = poly->u3 = poly->u0 + 0x20;
    poly->v2 = poly->v3 = poly->v0 + 0xa;
    setClut(poly, tex->clut_x, tex->clut_y);
    setTPage(poly, tex->tp, tex->abr, tex->tx, tex->ty);
    addPrim(ot, poly);
    return poly + 1;
}
