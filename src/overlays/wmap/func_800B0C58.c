/* Partial WMAP decompilation: 87.208336% (gcc280_g0). */
#include "common.h"

typedef struct
{
    u8 pad0[0x22];
    s16 f22;
    u8 pad1[0x2];
    s16 f26;
    u8 pad2[0x4];
} WmapEntry;

extern u8 D_800DB4C8;
extern s32 D_801B2F68;
extern s32 D_801B2F6C;
extern void func_800B0CB8(void);

/** @brief World-map step handler: clear a small entry table, set the timer, advance the step. */
void func_800B0C58(void)
{
    WmapEntry *p;
    s32 i;

    i = 0;
    p = (WmapEntry *)&D_800DB4C8;
    do
    {
        p->f22 = 0;
        p->f26 = 2;
        i += 1;
        p += 1;
    } while (i < 0x28);

    D_801B2F6C = 0x40;
    D_801B2F68 += 1;
    func_800B0CB8();
}
