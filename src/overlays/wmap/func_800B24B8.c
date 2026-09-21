#include "common.h"

extern void func_8006CAC0(void (*fn)(void));
extern void func_800B27F8(void);
extern s32 D_8011D500;
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

/** @brief World-map step handler: register the next callback and advance the counter. */
void func_800B24B8(void)
{
    func_8006CAC0(func_800B27F8);
    D_8011D500 = 0x87;
    D_801B2F9C = 0xF;
    D_801B2F98 += 1;
}
