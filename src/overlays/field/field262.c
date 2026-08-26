#include "common.h"

typedef struct
{
    u8 pad0[0x404];
    s32 unk404; /* 0x404 */
    u8 pad408[0x410 - 0x408];
    u32 unk410; /* 0x410 */
    u32 unk414; /* 0x414 */
} StructB78;

extern StructB78 *D_80122B78;

void func_800BD04C(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    StructB78 *p;
    u32 raw;
    u32 v;

    p = D_80122B78;
    raw = p->unk410;
    p->unk414 = arg3;
    v = raw;
    v &= ~0x3FF;
    v |= arg0 & 0x3FF;
    v &= 0xFFF003FF;
    v |= (arg1 & 0x3FF) << 10;
    v &= 0xC00FFFFF;
    v |= (arg2 & 0x3FF) << 20;
    p->unk410 = v;
}
