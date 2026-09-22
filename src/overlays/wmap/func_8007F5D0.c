#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8013B208;
extern s32 D_801ADAF4;
extern s32 D_801B2788;
extern s32 D_801B278C;
extern void func_8006683C(s32 arg);
extern void func_800652A8(s32 a, s32 b);
extern void func_8007FC38(void);
extern void func_8007FADC(void);

/** @brief World-map state entry: load resources, register callbacks, advance. */
void func_8007F5D0(void)
{
    D_8013B208 = 1;
    func_8006683C(0x803030);
    D_801ADAF4 = 4;
    func_800652A8(0x1D, 0x80);
    func_8006CAC0(func_8007FC38);
    func_8006CAC0(func_8007FADC);
    D_801B278C = 8;
    D_801B2788 += 1;
}
