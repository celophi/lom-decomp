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
extern u8 D_80121538[];
extern void *D_80139280;
extern s32 D_801B2CB0;
extern s32 D_801B2CB4;
extern void func_8009CEF4(void);

/** @brief World-map step: init hero struct fields, clear tables, advance step. */
void func_8009B67C(void)
{
    s32 i;
    void *base = D_80139280;

    *(s32 *)((u8 *)base + 0x34) = 0x80;
    *(s32 *)((u8 *)base + 0x3C) = 2;
    *(s32 *)((u8 *)base + 0x40) = 0x64;
    *(s32 *)((u8 *)base + 0x44) = 0x3C;
    *(s32 *)((u8 *)base + 0x48) = 8;
    *(s32 *)((u8 *)base + 0x4C) = 1;
    *(s32 *)((u8 *)base + 0x2C) = 0;
    *(s32 *)((u8 *)base + 0x30) = 0;
    *(s32 *)((u8 *)base + 0x38) = 0;
    *(s32 *)((u8 *)base + 0x50) = 0x5DC0;

    for (i = 0; i < 0x18; i++)
    {
        D_801AFBD0[60 + i].field0 = 0;
        D_80139988[60 + i].field4 = D_80121538;
    }

    D_801B2CB4 = 0x38;
    D_801B2CB0 += 1;
    func_8009CEF4();
}
