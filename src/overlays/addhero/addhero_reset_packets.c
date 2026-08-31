#include "common.h"

typedef struct
{
    s32 attr;
    s32 flags;
    s32 draw;
} AddheroPacket;

extern s32 D_8012298C;
extern AddheroPacket D_80160940;

/** @see decomp.me (100%) */
void func_80141E54(void)
{
    AddheroPacket *p;
    s32 i;

    D_8012298C = 0x20;
    p = &D_80160940;
    for (i = 0; i < 8; i++)
    {
        p->flags &= ~0x200;
        p->attr &= ~7;
        p++;
    }
}
