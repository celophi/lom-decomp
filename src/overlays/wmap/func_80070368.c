#include "common.h"

extern void func_8006CFE4(void* a0, void* a1, s32 a2, s32 a3, s32 a4, s32 a5);
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32 D_80182DEC;
extern s32 D_801B2450;
extern s32 D_801B2454;

/** @brief World-map step: draw the active overlay while its timer runs, then tick down. */
void func_80070368(void)
{
    s32 c;

    if (D_80182DEC > 0)
    {
        func_8006CFE4(D_800DA448, D_80139CC8, 0x10, 0, D_80182DEC, 3);
    }
    D_80182DEC -= 4;
    c = D_801B2454 - 1;
    D_801B2454 = c;
    if (c == 0)
    {
        D_801B2450 += 1;
    }
}
