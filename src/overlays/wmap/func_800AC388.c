#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_800652A8(s32, s32);
extern s32 D_801B2E80;
extern s32 D_801B2E84;
extern void func_800ACB64(void);
extern void func_800AD23C(void);

/** @brief Play sound 45, register two callbacks, and begin an eight-tick delay. */
void func_800AC388(void)
{
    func_800652A8(0x2D, 0x80);
    func_8006CAC0(&func_800AD23C);
    func_8006CAC0(&func_800ACB64);
    D_801B2E84 = 8;
    D_801B2E80 += 1;
}
