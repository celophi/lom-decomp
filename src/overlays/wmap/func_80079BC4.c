#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_800652A8(s32, s32);
extern s32 func_8006683C(s32);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern s32 D_801B2688;
extern s32 D_801B268C;
extern void func_80079F90(void);
extern void func_8007AA60(void);
extern void func_8007ABB0(void);

/** @brief Set color and flags, play sound 26, and register three sequence callbacks. */
void func_80079BC4(void)
{
    D_8013B208 = 1;
    func_8006683C(0x202020);
    func_800652A8(0x1A, 0x80);
    func_8006CAC0(&func_80079F90);
    D_801ADAE0 = 1;
    func_8006CAC0(&func_8007ABB0);
    func_8006CAC0(&func_8007AA60);
    D_801B268C = 8;
    D_801B2688 += 1;
}
