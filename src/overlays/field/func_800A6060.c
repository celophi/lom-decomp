#include "common.h"
#include "sdk/libgpu.h"

extern s16 D_8011F3D0;
extern u16 D_80122720;
extern u16 D_80122722;
void func_80086F48(const void* src, s16 value);

POLY_FT4* func_800A6060(POLY_FT4* prim, u_long* ordering_table)
{
    s32 value0;
    s32 value1;
    s32 value2;
    s32 value3;
    s32 value4;
    s32 value5;
    u32 condition;

    value0 = 9;
    setlen(prim, value0);
    value0 = 0x2C;
    setcode(prim, value0);
    value0 = (u8)D_8011F3D0;
    prim->b0 = value0;
    prim->g0 = value0;
    prim->r0 = value0;
    value1 = getcode(prim);
    value1 |= 2;
    setcode(prim, value1);
    value1 = 0x80;
    value0 = (u16)D_8011F3D0;
    value4 = D_80122720;
    value5 = value1 - value0;
    value3 = value4 * value5;
    value2 = value3 / 32;
    value0 = 0xA0;
    value0 -= value2;
    value1 = value4 * 2;
    value0 -= value1;
    prim->x2 = value0;
    prim->x0 = value0;
    value0 = value4 * 4;
    value0 += 0xA0;
    value0 -= value1;
    value3 = D_80122722;
    value1 = value3 * value5;
    value0 += value2;
    value0--;
    prim->x3 = value0;
    prim->x1 = value0;
    value0 = 0x70;
    value2 = (u32)value3 >> 1;
    value0 -= value2;
    value4 = value1;
    if (value1 < 0)
    {
        value4 = value1 + 0x7F;
    }
    value5 = value4 >> 7;
    value0 -= value5;
    prim->y1 = value0;
    prim->y0 = value0;
    value0 = value2 - 0x70;
    value0 = value3 - value0;
    value0 += value5;
    value0--;
    prim->y3 = value0;
    prim->y2 = value0;
    value0 = 0x7C80;
    prim->u2 = 0;
    prim->u0 = 0;
    value1 = D_80122720;
    prim->v1 = 0;
    prim->v0 = 0;
    prim->clut = value0;
    value1 <<= 2;
    value1--;
    prim->u3 = value1;
    prim->u1 = value1;
    value0 = (u8)D_80122722;
    value1 = 0x25;
    prim->tpage = value1;
    value0--;
    prim->v3 = value0;
    prim->v2 = value0;
    addPrim(ordering_table, prim);
    condition = (u16)D_8011F3D0;
    condition = condition < 0x7C;
    if (condition != 0)
    {
        func_80086F48(prim, 0);
    }
    return prim + 1;
}
