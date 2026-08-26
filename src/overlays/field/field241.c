#include "common.h"

typedef struct
{
    u8 pad0[0x10];
    s16 unk10;
    u8 pad12[0x2A - 0x12];
    s16 unk2A;
} Rec875C4;

s32 func_800875C4(void)
{
    Rec875C4 *rec = func_80087C9C();
    s32 result;

    if (rec == (Rec875C4 *)-1)
    {
        return -1;
    }

    result = 0;
    if (rec->unk10 == 0)
    {
        result = rec->unk2A == 0;
    }

    return result;
}
