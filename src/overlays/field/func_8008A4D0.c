#include "common.h"

typedef struct {
    u8 pad0[0xC];
    s32 unkC;
} FieldA4D0State;

typedef struct {
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} FieldA4D0Rect;

void func_8008A4D0(FieldA4D0State *arg0, FieldA4D0Rect *rect, s32 arg2, s32 arg3)
{
    s32 xoff;
    s32 yoff;
    s32 mode;

    mode = arg0->unkC;
    if (mode >= 2)
    {
        yoff = 0;
        if (mode >= 9)
        {
            yoff = 0x100;
            xoff = 0x3C0 - ((mode - 9) << 6);
        }
        else
        {
            xoff = 0x340 - (mode << 6);
        }
    }
    else
    {
        yoff = 0;
        xoff = 0x380 - (mode << 7);
    }

    rect->x = (s16)rect->x >> 2;
    rect->x += xoff;
    rect->w = (s16)rect->w >> 2;
    rect->y += yoff;
    MoveImage2(rect, (arg2 >> 2) + xoff, arg3 + yoff);
}
