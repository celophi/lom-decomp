#include "wmap_sequence_runtime.h"
#include "common.h"
#include "sdk/libgte.h"

/** @brief World-map vector; only the third halfword is changed here. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    s16 field_04;
    s16 field_06;
} WmapVector;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 D_8011CF34;
extern s32 D_8013923C;
extern WmapVector D_801B2678;
extern s32 D_80182DC0[];
extern s32 D_801B25DC;
extern s32 D_801B2F40;
extern s32 D_801B2F44;

/** @brief Advance the world-map effect and its sequence state. */
void func_800AE30C(void)
{
    s32 remaining;
    s32 intensity;

    func_8006CFA8(D_80182DC0, &D_801B2678);
    func_800675F0(D_8011CF34, (D_8013923C >> 4) & 7, 10, 0x35, 0x7800, 0x1001, D_801B25DC, -1, 7, -1);
    D_8013923C += 16;
    intensity = D_801B25DC + 2;
    D_801B25DC = intensity;
    if (intensity >= 0x82)
    {
        D_801B25DC = 0x81;
    }
    remaining = D_801B2F44 - 1;
    D_801B2678.field_04 = (u16) (D_801B2678.field_04 + 0xC);
    D_801B2F44 = remaining;
    if (remaining == 0)
    {
        D_801B2F40 += 1;
    }
}
