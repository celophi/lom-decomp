#include "common.h"
#include "sdk/libgpu.h"

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} CardaRect;

typedef struct
{
    u32 tag;
    u8 r0;
    u8 g0;
    u8 b0;
    u8 code;
    s16 x0;
    s16 y0;
    s16 w;
    s16 h;
} CardaTile;

#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))

extern s32 D_801660A0;
extern u16 D_8014B044;
extern u16 D_8014B046;
extern s32 func_800A88A0(s32, s32 *, void *, s32, s32, s32, s32);

s32 func_80141C3C(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    CardaRect pos;
    CardaTile *tile;

    if (D_801660A0 != 0)
    {
        tile = (CardaTile *)prim;
        *(u32 *)&tile->r0 = 0x101010;
        setlen(tile, 3);
        setcode(tile, 0x62);
        setXY0(tile, 0, 0);
        setWH(tile, 0x80, 0x10);
        addPrim(ot, tile);
        prim += 0x10;
    }
    return func_800A88A0(prim, ot, GLYPH_SYM(D_8014B044, 0xC), 4, -arg2 + 0x40, -arg3, 2);
}

s32 func_80141D18(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    CardaRect pos;
    CardaTile *tile;

    if (D_801660A0 == 0)
    {
        tile = (CardaTile *)prim;
        *(u32 *)&tile->r0 = 0x101010;
        setlen(tile, 3);
        setcode(tile, 0x62);
        setXY0(tile, 0, 0);
        setWH(tile, 0x80, 0x10);
        addPrim(ot, tile);
        prim += 0x10;
    }
    return func_800A88A0(prim, ot, GLYPH_SYM(D_8014B046, 0xE), 4, -arg2 + 0x40, -arg3, 2);
}
