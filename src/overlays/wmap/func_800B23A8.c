#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_800B3020(void);
extern s32 D_8011D500;
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

/** @brief World-map step handler: register the next callback and advance the counter. */
void func_800B23A8(void)
{
    func_8006CAC0(func_800B3020);
    D_8011D500 = 0x71;
    D_801B2F9C = 0x5A;
    D_801B2F98 += 1;
}
