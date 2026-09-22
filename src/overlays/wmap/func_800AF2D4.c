#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_800B1190(void);
extern void func_800B04D0(void);
extern void func_800B01D0(void);
extern s32 D_8013B29C;
extern s32 D_801ADAF4;
extern s32 D_801B2EF8;
extern s32 D_801B2EFC;

/** @brief World-map step handler: register three callbacks, reset state, advance the step. */
void func_800AF2D4(void)
{
    func_8006CAC0(func_800B1190);
    func_8006CAC0(func_800B04D0);
    func_8006CAC0(func_800B01D0);
    D_8013B29C = 0;
    D_801ADAF4 = 3;
    D_801B2EFC = 0x10;
    D_801B2EF8 += 1;
}
