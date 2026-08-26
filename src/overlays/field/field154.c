#include "common.h"

typedef struct
{
    u8 pad0[0x2E8];
    s32 arr2E8[0x40]; /* 0x2E8 */
} StructB74;

extern StructB74 *D_80122B74;

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
