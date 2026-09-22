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

extern s32 D_8011CF24;
extern s32 D_8013923C;
extern WmapVector D_8013B238;
extern s32 D_80182DC0[];
extern s32 D_80182DF0;
extern s32 D_801B2C28;
extern s32 D_801B2C2C;

/** @brief Advance the world-map effect and its sequence state. */
void func_80097A94(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    func_8006CFA8(D_80182DC0, &D_8013B238);
    func_8006CD98(D_8011CF24, D_8013923C, 4, 0x35, 0x7800, 0x1001, D_80182DF0);
    intensity = D_80182DF0 - 2;
    D_8013B238.field_04 = (u16) (D_8013B238.field_04 + 0x10);
    D_80182DF0 = intensity;
    if (intensity < 0)
    {
        D_80182DF0 = 0;
    }
    D_8013923C = (D_8013923C + 1) & 7;
    PopMatrix();
    remaining = D_801B2C2C - 1;
    D_801B2C2C = remaining;
    if (remaining == 0)
    {
        D_801B2C28 += 1;
    }
}
