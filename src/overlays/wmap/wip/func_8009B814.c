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
extern u8 D_8011F538[];
extern void *D_80139280;
extern s32 D_801B2CC0;
extern s32 D_801B2CC4;
extern void func_8009D304(void);

/** @brief World-map step: init hero struct fields, clear tables, advance step.
 *  @note Best match ~94% (gcc280_g0); residual is a whole-function register
 *        renumbering caused by the held constant 1 (a0) plus a store-order tie. */
void func_8009B814(void)
{
    s32 i;
    void *base = D_80139280;

    *(s32 *)((u8 *)base + 0xA8) = 4;
    *(s32 *)((u8 *)base + 0xAC) = 0x20;
    *(s32 *)((u8 *)base + 0xB8) = 0x3E8;
    *(s32 *)((u8 *)base + 0xBC) = 0xB4;
    *(s32 *)((u8 *)base + 0xC0) = 0x15;
    *(s32 *)((u8 *)base + 0xC4) = 2;
    *(s32 *)((u8 *)base + 0xA4) = 1;
    *(s32 *)((u8 *)base + 0xB0) = 0;
    *(s32 *)((u8 *)base + 0xB4) = 1;
    *(s32 *)((u8 *)base + 0xC8) = 0x1F40;

    for (i = 0; i < 0x14; i++)
    {
        D_801AFBD0[180 + i].field0 = 0;
        D_80139988[180 + i].field4 = D_8011F538;
    }

    D_801B2CC4 = 0x14;
    D_801B2CC0 += 1;
    func_8009D304();
}
