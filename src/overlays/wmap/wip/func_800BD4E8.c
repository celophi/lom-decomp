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
extern s32 D_801B31F0;
extern s32 D_801B31F4;
extern void func_800BEDE0(void);

/** @brief World-map step: init hero struct fields, clear tables, advance step.
 *  @note Best match ~94% (gcc280_g0); residual is a whole-function register
 *        renumbering caused by the held constant 1 (a0) plus a store-order tie. */
void func_800BD4E8(void)
{
    s32 i;
    void *base = D_80139280;

    *(s32 *)((u8 *)base + 0x30) = 2;
    *(s32 *)((u8 *)base + 0x34) = 0x80;
    *(s32 *)((u8 *)base + 0x40) = 0x190;
    *(s32 *)((u8 *)base + 0x44) = 0x50;
    *(s32 *)((u8 *)base + 0x48) = 0x2C;
    *(s32 *)((u8 *)base + 0x4C) = 4;
    *(s32 *)((u8 *)base + 0x2C) = 1;
    *(s32 *)((u8 *)base + 0x38) = 0;
    *(s32 *)((u8 *)base + 0x3C) = 1;
    *(s32 *)((u8 *)base + 0x50) = 0x4650;

    for (i = 0; i < 0x28; i++)
    {
        D_801AFBD0[80 + i].field0 = 0;
        D_80139988[80 + i].field4 = D_80121538;
    }

    D_801B31F4 = 0x28;
    D_801B31F0 += 1;
    func_800BEDE0();
}
