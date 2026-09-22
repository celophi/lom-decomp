#include "common.h"

extern void func_80080D2C(void);
extern s32 D_801B27DC;
extern s32 D_801B27D8;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80080CF4(void)
{
    D_801B27DC = 0x30;
    D_801B27D8 += 1;
    func_80080D2C();
}
