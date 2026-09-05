#include "common.h"

extern u8 *g_field_script;
extern u8 *D_80122B78;

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Struct80087F44;

void func_800BC9F8(s32 arg0, s32 arg1)
{
    Struct80087F44 sp18;
    s32 idx;

    if (arg0 == 0xFF)
    {
        idx = *g_field_script;
    }
    else
    {
        idx = arg0;
    }
    func_80087F44(idx, &sp18);
    {
        s32 x = sp18.unk0 >> 8;
        s32 y = sp18.unk4 >> 8;
        s32 z = sp18.unk8 >> 8;
        func_80087680(idx, arg1, D_80122B78[0x403], x, y, z);
    }
}
