#include "common.h"
s32 func_800A9D70(s32);
extern s32 D_8012269C;
extern s32 D_801227BC;
extern s32 D_801227C0;
extern s32 D_801227D8;
extern s32 D_801227E4;
extern s32 D_801229F8;
extern s32 g_pad_input;
extern s32 g_pad_input_inject;

/**
 * @brief Apply initial-delay and repeat timing to both field controller inputs.
 * @note Held input waits fifteen ticks initially, then repeats every three ticks.
 * @note The direction filter is enabled by D_8012269C.
 * @note 100% match with GCC 2.7.2 CDK: 109 instructions, 436 bytes.
 */
void func_800A9E78(void)
{
    s32 directions;
    s32 buttons;

    buttons = func_800A9D70(0);
    g_pad_input = 0;
    D_801229F8 = 0;
    if ((buttons == D_801227BC) || ((D_801227BC != 0) && (buttons & (D_801227BC | 0xB6F))))
    {
        directions = buttons & 0xF000;
        if (buttons != 0)
        {
            if ((directions != 0) && (D_8012269C != 0))
            {
                buttons = directions;
            }
            if (D_801227C0 == 0)
            {
                g_pad_input = buttons;
                D_801227C0 = 2;
            }
            else
            {
                D_801227C0 -= 1;
                g_pad_input = 0;
            }
        }
        else
        {
            goto clear_primary;
        }
    }
    else if (buttons == 0)
    {
    clear_primary:
        D_801227C0 = 0;
        D_801227BC = 0;
    }
    else
    {
        g_pad_input = buttons;
        D_801227BC = buttons;
        D_801227C0 = 0xF;
    }
    buttons = func_800A9D70(1);
    g_pad_input_inject = 0;
    if ((buttons == D_801227D8) || ((D_801227D8 != 0) && (buttons & (D_801227D8 | 0xB6F))))
    {
        directions = buttons & 0xF000;
        if (buttons != 0)
        {
            if ((directions != 0) && (D_8012269C != 0))
            {
                buttons = directions;
            }
            if (D_801227E4 == 0)
            {
                g_pad_input_inject = buttons;
                D_801227E4 = 2;
            }
            else
            {
                D_801227E4 -= 1;
                g_pad_input_inject = 0;
            }
        }
        else
        {
            goto clear_injected;
        }
    }
    else if (buttons == 0)
    {
    clear_injected:
        D_801227E4 = 0;
        D_801227D8 = 0;
    }
    else
    {
        g_pad_input_inject = buttons;
        D_801227D8 = buttons;
        D_801227E4 = 0xF;
    }
    D_801229F8 = g_pad_input;
}
