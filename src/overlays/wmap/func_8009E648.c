#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2CD8;
extern s32 D_801B2CDC;
extern void func_8009EC7C(void);

/** @brief World-map step handler: register a callback and advance the step. */
void func_8009E648(void)
{
    func_8006CAC0(func_8009EC7C);
    D_801B2CDC = 1;
    D_801B2CD8 += 1;
}
