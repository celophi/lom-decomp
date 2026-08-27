#include "common.h"

typedef struct
{
    u8 pad0[0x404];
    s32 unk404; /* 0x404 */
    u8 pad408[0x410 - 0x408];
    u32 unk410; /* 0x410 */
    u32 unk414; /* 0x414 */
} StructB78;

extern u8 g_menuLayoutBuffer[];
extern s32 D_80122C00;
extern u8* D_80122B74;
extern s32* D_80122B78;
extern void* D_80122B70;

void func_800B0BDC(void)
{
    D_80122B74 = g_menuLayoutBuffer;
    D_80122B78 = &D_80122C00;
    D_80122B70 = (void*)0x801ED480;
}

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
