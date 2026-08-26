#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
} UnkStruct800B62D8;

extern s32 func_800BD414(s32 arg0, s32 arg1);

s32 func_800B62D8(UnkStruct800B62D8 *arg0)
{
    if (arg0->unk4 & 0x200)
    {
        if (func_800BD414(0, 0x4280) == 0)
        {
            return 1;
        }
    }
    else
    {
        if (func_800BD414(0, 0x4284) == 0)
        {
            return 2;
        }
    }

    return 0;
}
