#include "common.h"

extern void func_8006CAC0(void (*fn)(void));
extern void func_800A2A80(void);
extern void func_800A25E4(void);
extern void func_800A1D64(void);
extern s32 D_800DBE70;
extern s32 D_80139978;
extern s32 D_801B2D58;
extern s32 D_801B2D5C;

/** @brief World-map step handler: register three callbacks, reset state, advance the step. */
void func_800A1A04(void)
{
    func_8006CAC0(func_800A2A80);
    func_8006CAC0(func_800A25E4);
    func_8006CAC0(func_800A1D64);
    D_800DBE70 = 0;
    D_80139978 = -1;
    D_801B2D5C = 0xF;
    D_801B2D58 += 1;
}
