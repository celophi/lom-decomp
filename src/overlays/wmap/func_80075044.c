#include "common.h"

extern s32 D_8011CF50;
extern s32 D_8013B20C;
extern s32 D_801B2568;
extern void func_8006C81C(void);
extern void func_8006CAC0(void (*fn)(void));
extern void func_80075094(void);

/**
 * @brief Register a world-map callback and advance to the next step.
 */
void func_80075044(void)
{
    D_8011CF50 = 1;
    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2568 += 1;
    func_80075094();
}
