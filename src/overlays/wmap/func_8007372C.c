#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8011CF50;
extern s32 D_8013B20C;
extern s32 D_801B2518;
extern void func_8007377C(void);

/**
 * @brief Register a world-map callback and advance to the next step.
 */
void func_8007372C(void)
{
    D_8011CF50 = 1;
    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2518 += 1;
    func_8007377C();
}
