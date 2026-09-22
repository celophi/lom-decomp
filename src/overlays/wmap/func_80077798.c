#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 func_8006683C(s32 a0);
extern s32 func_800652A8(s32 a0, s32 a1);
extern void func_8007882C(void);
extern s32 D_8013B208;
extern s32 D_801B25F4;
extern s32 D_801B25F0;

/** @brief World-map step handler: set up the actor, register callbacks, advance step. */
void func_80077798(void)
{
    D_8013B208 = 1;
    func_8006683C(0x804020);
    func_8006CAC0(func_8007882C);
    func_800652A8(0x19, 0x80);
    D_801B25F4 = 2;
    D_801B25F0 += 1;
}
