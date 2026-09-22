#include "wmap_sequence_runtime.h"
#include "common.h"

/** @brief World-map actor configuration with its original field layout. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

extern void func_80066F9C(void *, s32, s32, s32, s32);
extern WmapConfigA D_800D9420;
extern s32 D_8011CF4C;
extern u8 D_801399D8[];
extern s32 D_80182DF4;
extern s32 D_801B25C8;
extern s32 D_801B25CC;

/** @brief Draw the actor, update two effect fields, and advance when the countdown expires. */
void func_80076A38(void)
{
    s32 remaining_ticks;

    func_8006CC4C(&D_800D9420, &D_801399D8);
    func_80066F9C(&D_800D9420, D_8011CF4C, 0x10, 0xB, 0);
    D_800D9420.field_24 = (u16) D_80182DF4;
    D_800D9420.field_22 = (u16) D_80182DF4;
    if ((s32) D_80182DF4 < 0)
    {
        D_80182DF4 = 0;
    }
    remaining_ticks = D_801B25CC - 1;
    D_801B25CC = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B25C8 += 1;
    }
}
