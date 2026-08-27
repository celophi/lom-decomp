#include "common.h"

s32 func_800A9D70(s32);
extern s32 D_801227BC;
extern s32 D_801227C0;
extern s32 D_801227D8;
extern s32 D_801227E4;
extern s32 D_801229F8;
extern s32 g_pad_input;
extern s32 g_pad_input_inject;

/**
 * @brief Resets field menu input and its two control handles.
 *
 * Clears the live and injected pad state, acquires control handles zero and
 * one, initializes their masks to 0xF, and clears the pending input state.
 */
void func_800AA02C(void)
{
    g_pad_input = 0;
    D_801227BC = func_800A9D70(0);
    D_801227C0 = 0xF;
    g_pad_input_inject = 0;
    D_801227D8 = func_800A9D70(1);
    D_801227E4 = 0xF;
    D_801229F8 = 0;
}
