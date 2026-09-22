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

extern u8 D_800D94D0;
extern s32 D_801B2760;
extern s32 D_801B2764;
extern void func_8007E6A0(void);

/** @brief World-map step handler: clear a small entry table, set the timer, advance the step. */
void func_8007E640(void)
{
    WmapEntry *p;
    s32 i;

    i = 0;
    p = (WmapEntry *)&D_800D94D0;
    do
    {
        p->f22 = 0;
        p->f26 = 8;
        i += 1;
        p += 1;
    } while (i < 4);

    D_801B2764 = 0x10;
    D_801B2760 += 1;
    func_8007E6A0();
}
