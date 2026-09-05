#include "common.h"

typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} ZukanRect;

extern u8 D_8014471C[];

void func_80142CA0(void)
{
    ZukanRect rect;
    s32 raw;
    s32 dimensions;
    s16 temp;
    u8 *base = D_8014471C;

    func_800141EC(0x5E3, base);

    raw = *(volatile s32 *)base;
    temp = raw;
    dimensions = temp;

    *(volatile s16 *)&rect.x = 0x340;
    rect.y = 0x100;
    rect.w = dimensions;

    dimensions = raw >> 16;
    rect.h = dimensions;

    func_80019A34(&rect, base + 4);
}
