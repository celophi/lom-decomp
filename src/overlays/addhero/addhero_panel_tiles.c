#include "common.h"

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} AddheroRect;

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
} AddheroTile;

#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))

extern s32 D_801609A8;
extern u16 D_80146FB0;
extern u16 D_80146FB2;
extern s32 func_800A88A0(s32, s32 *, void *, s32, s32, s32, s32);

s32 func_801414DC(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    AddheroRect pos;
    AddheroTile *tile;

    if (D_801609A8 != 0)
    {
        tile = (AddheroTile *)prim;
        *(u32 *)&tile->r0 = 0x101010;
        *((u8 *)tile + 3) = 3;
        tile->code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = 0x80;
        tile->h = 0x10;
        tile->tag = (tile->tag & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | ((s32)tile & 0xFFFFFF);
        prim += 0x10;
    }
    return func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB0, 0xC), 4, -arg2 + 0x40, -arg3, 2);
}

s32 func_801415B8(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    AddheroRect pos;
    AddheroTile *tile;

    if (D_801609A8 == 0)
    {
        tile = (AddheroTile *)prim;
        *(u32 *)&tile->r0 = 0x101010;
        *((u8 *)tile + 3) = 3;
        tile->code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = 0x80;
        tile->h = 0x10;
        tile->tag = (tile->tag & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | ((s32)tile & 0xFFFFFF);
        prim += 0x10;
    }
    return func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB2, 0xE), 4, -arg2 + 0x40, -arg3, 2);
}
