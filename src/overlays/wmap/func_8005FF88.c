#include "common.h"
#include "sdk/libgpu.h"

typedef struct
{
    s16 x, y;
} WmapPoint;
extern WmapPoint D_800519E4[];
extern SPRT D_80051A08;
extern SPRT D_80051A1C;
extern s32 D_800D0230[];
extern s32 D_800D0254[];
extern s32 D_800D0368;
extern u16 D_800D036C[];
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern u16 D_8011CF78;
extern SPRT D_8011D518;
extern s32 D_801398D0;
extern u16 D_80182DD0;
extern s32 D_80182E04;
extern SPRT D_80182E08;
extern s32 D_801ADAE4;
extern s32 func_8005D528(s32);

/** @brief Change the map label sprite while preserving its outgoing fade. */
void func_8005FF88(s32 selection)
{
    s32 position;
    s32 previous_selection;
    s32 width;
    s32 *scale;

    if (selection == 33)
    {
        selection = 24;
    }
    if (D_801398D0 != 0)
    {
        selection = -1;
    }
    previous_selection = D_801ADAE4;
    if (previous_selection != selection && (D_80182E08.r0 == 0 || selection == -1))
    {
        D_80182E04 = previous_selection;
        D_80182E08 = D_8011D518;
        D_801ADAE4 = selection;
        D_80182DD0 = D_8011CF78;
        D_80182E08.code |= 2;
        D_80182E08.clut = (D_800D0368 << 6) | 47;
        if (selection == -1)
        {
            D_8011D518.r0 = 0;
            return;
        }
        position = D_800DCEEC + D_800DCEF0 * 3;
        if (selection >= 64)
        {
            D_8011CF78 = 45;
            D_8011D518 = D_80051A08;
            D_800D0368 = 506;
        }
        else
        {
            D_8011CF78 = 43;
            D_8011D518 = D_80051A1C;
            if (func_8005D528(selection) == 0)
            {
                D_800D0368 = 508;
            }
            else
            {
                D_800D0368 = 506;
            }
            scale = &D_800D0230[position];
            width = *scale * D_800D0254[selection];
            D_8011D518.y0 = D_800519E4[position].y;
            D_8011D518.x0 = D_800519E4[position].x - width / 2;
        }
        selection &= 63;
        selection = (s16)D_800D036C[selection];
        D_8011D518.r0 = 8;
        D_8011D518.u0 = (selection / 14) * 112;
        D_8011D518.v0 = (selection % 14) * 16;
    }
}
