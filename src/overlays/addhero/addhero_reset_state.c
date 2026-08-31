#include "common.h"

extern s32 D_801609B4;
extern void *D_80165488;
extern s32 D_801609A4;
extern s32 D_801609BC;
extern s32 D_80160938;
extern s32 D_80160928;
extern s32 D_801609AC;
extern s32 D_801609A8;
extern s32 D_801609B8;
extern s32 D_80122988;

void func_801449F0();
void func_800AA02C();

/** @see decomp.me (100%) */
void func_80140C18(void)
{
    D_801609B4 = 0;
    D_80165488 = 0;
    D_801609A4 = 0xFF;
    D_801609BC = 0;
    D_80160938 = 0;
    D_80160928 = 0;
    D_801609AC = 0;
    D_801609B8 = 0;
    D_801609A8 ^= 1;
    func_801449F0();
    func_800AA02C();
    D_80122988 = 0;
}
