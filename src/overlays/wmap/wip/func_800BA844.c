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
extern s32 D_801B3180;
extern s32 D_801B3184;
extern void func_800BC7F4(void);

/** @brief World-map step: init hero struct fields, clear tables, advance step.
 *  @note Best match ~96% (gcc280_g0); residual is a base-load vs IV-init schedule swap. */
void func_800BA844(void)
{
    s32 i;
    void *base = D_80139280;

    *(s32 *)((u8 *)base + 0xC) = 0x80;
    *(s32 *)((u8 *)base + 0x14) = 3;
    *(s32 *)((u8 *)base + 0x18) = 0x384;
    *(s32 *)((u8 *)base + 0x1C) = 0x14;
    *(s32 *)((u8 *)base + 0x20) = 8;
    *(s32 *)((u8 *)base + 0x24) = 1;
    *(s32 *)((u8 *)base + 0x4) = 0;
    *(s32 *)((u8 *)base + 0x8) = 0;
    *(s32 *)((u8 *)base + 0x10) = 0;
    *(s32 *)((u8 *)base + 0x28) = 0x1F40;

    for (i = 0; i < 0x3C; i++)
    {
        D_801AFBD0[20 + i].field0 = 0;
        D_80139988[20 + i].field4 = D_80121538;
    }

    D_801B3184 = 0xB4;
    D_801B3180 += 1;
    func_800BC7F4();
}
