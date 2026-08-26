#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
} UnkStruct801226A0;

extern UnkStruct801226A0 D_801226A0[];

s32 func_800A6490(void)
{
    s32 i;
    UnkStruct801226A0 *p;

    i = 0;
    p = D_801226A0;
    for (; i < 3; i++, p++)
    {
        if (((u32)p->unk4 >> 17) & 0x3F)
        {
            return 1;
        }
    }
    return 0;
}
