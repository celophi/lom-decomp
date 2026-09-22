#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2788;
extern s32 D_801B278C;
extern void func_8007FD90(void);

/** @brief World-map step handler: register a callback and advance the step. */
void func_8007F8B8(void)
{
    func_8006CAC0(func_8007FD90);
    D_801B278C = 1;
    D_801B2788 += 1;
}
