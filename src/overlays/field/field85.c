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

void func_800B0C10(void)
{
    StructB78 *p;
    u32 raw;
    u32 v;

    p = D_80122B78;
    p->unk414 = 0x10;
    p->unk404 = -1;
    raw = p->unk410;
    v = raw;
    v &= 0xFFFFFC00;
    v &= 0xFFF003FF;
    v &= 0xC00FFFFF;
    p->unk410 = v;
}
