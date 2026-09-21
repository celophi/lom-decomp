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

extern void func_8006CFA8(s32 *, WmapVector *);
extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 D_8011CF28;
extern s32 D_80139260;
extern WmapVector D_8013B240;
extern s32 D_80182DC0[];
extern s32 D_80182DF4;
extern s32 D_801B2DB0;
extern s32 D_801B2DB4;

/** @brief Advance the world-map effect and its sequence state. */
void func_800A11C4(void)
{
    s32 remaining;
    s32 intensity;

    func_8006CFA8(D_80182DC0, &D_8013B240);
    func_800675F0(D_8011CF28, D_80139260 & 3, 4, 0x36, 0x7900, 0x1001, D_80182DF4, 0, 0, -1);
    D_80139260 += 1;
    intensity = D_80182DF4 + 2;
    D_80182DF4 = intensity;
    if (intensity >= 0x62)
    {
        D_80182DF4 = 0x61;
    }
    remaining = D_801B2DB4 - 1;
    D_8013B240.field_04 = (u16) (D_8013B240.field_04 + 0x14);
    D_801B2DB4 = remaining;
    if (remaining == 0)
    {
        D_801B2DB0 += 1;
    }
}
