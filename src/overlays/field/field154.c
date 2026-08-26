#include "common.h"

typedef struct
{
    u8 pad0[0x2E8];
    s32 arr2E8[0x40]; /* 0x2E8 */
} StructB74;

extern StructB74 *D_80122B74;

extern void func_800C2228(s32 idx);

s32 func_800C2094(s32 arg0)
{
    s32 word;
    s32 bit;
    s32 q;

    q = arg0 / 32;
    bit = arg0 % 32;
    word = q;
    D_80122B74->arr2E8[word] |= 1 << bit;
    return -1;
}

u8 func_800C20D8(s32 arg0)
{
    if (arg0 < 0xFF)
    {
        u8 *p;

        func_800C2228(arg0);
        p = (u8 *) D_80122B74 + arg0;
        return p[0x25E0];
    }
    akao_set_song_params(0x8001, 0x70, arg0, 0);
    return 0;
}
