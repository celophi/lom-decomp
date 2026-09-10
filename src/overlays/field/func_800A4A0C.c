#include "common.h"

extern s32 D_8011F334;
extern s32 D_8011F33C;
extern u8 D_8011F358[];
extern s32 D_8011F378;
extern s32 D_8011F37C;
extern s32 D_8011F3A8;
extern s32 D_8011F3AC;
extern s32 D_8011F3B0;
extern s32 D_8011F3B8;
extern s32 D_8011F3C0;
extern s32 g_pad_input;

/**
 * @brief Advance the FIELD selection rotation and process selection input.
 * @note Active rotation interpolates over ten updates before snapping to its target.
 * @note Input branches remain independent so simultaneous button bits retain order.
 * @note Save the current rotation before decrementing the selected index.
 * @note GCC 2.7.2 CDK matches all 196 instructions (784 bytes), with no stack frame.
 */
void func_800A4A0C(void)
{
    s32 previous_index;

    if (D_8011F3A8 != 0)
    {
        D_8011F3C0 = D_8011F3B0 + (((D_8011F33C - D_8011F3B0) * D_8011F3A8) / 10);
        if (D_8011F3A8 == 0xA)
        {
            D_8011F3A8 = 0;
            D_8011F3C0 = (s32) (-D_8011F378 << 12) / (s32) D_8011F3B8;
            return;
        }
        D_8011F3A8 += 1;
        return;
    }
    if (g_pad_input & 0x220)
    {
        D_8011F3AC = 3;
        if (D_8011F37C != -1)
        {
            if (D_8011F378 == D_8011F37C)
            {
                D_8011F358[D_8011F334] = 0;
            }
            else
            {
                goto select_current;
            }
        }
        else
        {
select_current:
            D_8011F358[D_8011F334] = (u8) D_8011F378;
        }
    }
    if ((g_pad_input & 0x40) && (D_8011F37C != -1))
    {
        D_8011F378 = D_8011F37C;
        D_8011F3A8 = 1;
        D_8011F3B0 = D_8011F3C0;
        D_8011F33C = (s32) (-D_8011F37C << 12) / (s32) D_8011F3B8;
    }
    if (g_pad_input & 0x9008)
    {
        D_8011F3A8 = 1;
        D_8011F3B0 = D_8011F3C0;
        previous_index = D_8011F378 - 1;
        D_8011F378 = previous_index;
        D_8011F33C = D_8011F3C0 + (0x1000 / (s32) D_8011F3B8);
        if (previous_index < 0)
        {
            D_8011F378 = D_8011F3B8 - 1;
        }
    }
    if (g_pad_input & 0x6004)
    {
        D_8011F3A8 = 1;
        D_8011F3B0 = D_8011F3C0;
        D_8011F33C = D_8011F3C0 - (0x1000 / (s32) D_8011F3B8);
        D_8011F378 = (s32) (D_8011F378 + 1) % (s32) D_8011F3B8;
    }
}
