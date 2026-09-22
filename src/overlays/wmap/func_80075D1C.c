#include "wmap_sequence_runtime.h"
#include "common.h"

/** @brief World-map actor configuration. */
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

extern WmapConfigA D_800D9370;
extern u8 *D_801399B8;
extern s32 D_8011CF4C;
extern s32 D_80182DE4;
extern s32 D_801B2590;
extern s32 D_801B2594;
extern void func_80066F9C(WmapConfigA *, s32, s32, s32, s32);

/** @brief Draw the actor, reduce its scale, and advance when its timer expires. */
void func_80075D1C(void)
{
    WmapConfigA *actors;

    func_8006CC4C(&D_800D9370, &D_801399B8);
    func_80066F9C(&D_800D9370, D_8011CF4C, 15, 11, 0);
    actors = &D_800D9370 - 6;
    actors[6].field_24 = *(u16 *)&D_80182DE4;
    actors[6].field_22 = *(u16 *)&D_80182DE4;
    D_80182DE4 -= 8;
    if (D_80182DE4 < 0)
    {
        D_80182DE4 = 0;
    }
    if (--D_801B2594 == 0)
    {
        D_801B2590++;
    }
}
