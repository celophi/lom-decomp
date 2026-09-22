#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_80089298(void);
extern s32 D_8011CF4C;
extern s32 D_8013B20C;
extern s32 D_80182D58;
extern s32 D_801B2958;
extern void func_800892EC(void);

/** @brief Save the coordinate pair, register a callback, and run the next sequence step. */
void func_80089244(void)
{
    D_80182D58 = D_8011CF4C;
    func_8006CAC0(&func_800892EC);
    D_8013B20C = 1;
    D_801B2958 += 1;
    func_80089298();
}
