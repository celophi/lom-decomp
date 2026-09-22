#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801ADAF4;
extern s32 D_801B2CD8;
extern s32 D_801B2CDC;
extern void func_800A03B4(void);
extern void func_8006683C(s32 arg);

/** @brief World-map step handler: register a callback, kick a job, advance the step. */
void func_8009E8F4(void)
{
    func_8006CAC0(func_800A03B4);
    func_8006683C(0x302050);
    D_801ADAF4 = 3;
    D_801B2CDC = 8;
    D_801B2CD8 += 1;
}
