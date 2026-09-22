/* Partial WMAP decompilation: 96.078430% (gcc280_g0). */
#include "common.h"

typedef struct
{
    s16 field0;
    s16 field2;
    void *field4;
} WmapB;

typedef struct
{
    s16 field0;
    s16 pad[9];
} WmapA;

extern WmapA D_801AFBD0[];
extern WmapB D_80139988[];
extern u8 D_80123538[];
extern void *D_80139280;
extern s32 D_801B2E18;
extern s32 D_801B2E1C;
extern void func_800A5118(void);

/** @brief World-map step: init hero struct fields, clear tables, advance step. */
void func_800A3960(void)
{
    s32 i;
    void *base = D_80139280;

    *(s32 *)((u8 *)base + 0x0C) = 0x20;
    *(s32 *)((u8 *)base + 0x14) = 2;
    *(s32 *)((u8 *)base + 0x18) = 0x384;
    *(s32 *)((u8 *)base + 0x1C) = 0x14;
    *(s32 *)((u8 *)base + 0x20) = 8;
    *(s32 *)((u8 *)base + 0x24) = 1;
    *(s32 *)((u8 *)base + 0x04) = 0;
    *(s32 *)((u8 *)base + 0x08) = 0;
    *(s32 *)((u8 *)base + 0x10) = 0;
    *(s32 *)((u8 *)base + 0x28) = 0x32C8;

    for (i = 0; i < 0xC; i++)
    {
        D_801AFBD0[20 + i].field0 = 0;
        D_80139988[20 + i].field4 = D_80123538;
    }

    D_801B2E1C = 0x18;
    D_801B2E18 += 1;
    func_800A5118();
}
