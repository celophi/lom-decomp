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
extern s32 D_8011CF30;
extern s32 D_80139234;
extern WmapVector D_801B2670;
extern s32 D_80182DC0[];
extern s32 D_801B25D8;
extern s32 D_801B2F38;
extern s32 D_801B2F3C;

/** @brief Advance the world-map effect and its sequence state. */
void func_800AE12C(void)
{
    s32 remaining;
    s32 intensity;

    func_8006CFA8(D_80182DC0, &D_801B2670);
    func_800675F0(D_8011CF30, (D_80139234 >> 4) & 3, 7, 0x36, 0x7980, 0x1001, D_801B25D8, 0, -10, -1);
    D_80139234 += 16;
    intensity = D_801B25D8 + 2;
    D_801B25D8 = intensity;
    if (intensity >= 0x82)
    {
        D_801B25D8 = 0x81;
    }
    remaining = D_801B2F3C - 1;
    D_801B2670.field_04 = (u16) (D_801B2670.field_04 + 0x18);
    D_801B2F3C = remaining;
    if (remaining == 0)
    {
        D_801B2F38 += 1;
    }
}
