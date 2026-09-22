#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_80082D70(void);
extern s32 D_801ADAE0;
extern s32 D_801B27F4;
extern s32 D_801B27F0;

/** @brief World-map step handler: register the next callback and advance the counter. */
void func_80081EE8(void)
{
    func_8006CAC0(func_80082D70);
    D_801ADAE0 = 1;
    D_801B27F4 = 4;
    D_801B27F0 += 1;
}
