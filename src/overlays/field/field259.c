#include "common.h"

typedef struct
{
    char pad0[4];
    s32 unk4;   /* 0x4 */
    char pad4[0x14 - 8];
    s32 unk14;  /* 0x14 */
} SomeStruct;

void func_800B30B8(SomeStruct *arg0, s32 arg1)
{
    s32 v0;

    if (arg1 < 0)
    {
        akao_set_song_params(0x8001, 0x7A, arg0->unk14, arg1);
    }
    else
    {
        v0 = arg0->unk4 - arg1;
        if (v0 >= 0)
        {
            arg0->unk4 = v0;
        }
        else
        {
            arg0->unk4 = 0;
        }
    }
}
