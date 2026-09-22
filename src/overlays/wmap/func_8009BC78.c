#include "wmap_view_effects.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801ADAF4;
extern s32 D_801B2C68;
extern s32 D_801B2C6C;
extern void func_8009C3E4(void);

/** @brief World-map step handler: register a callback, kick a job, advance the step. */
void func_8009BC78(void)
{
    func_8006CAC0(func_8009C3E4);
    func_8006683C(0x651035);
    D_801ADAF4 = 4;
    D_801B2C6C = 2;
    D_801B2C68 += 1;
}
