#include "common.h"

extern s32 D_800D9164;
extern s32 D_801B2C54;
extern void func_8009ADC8(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009AD8C(void)
{
    if (D_800D9164 == 0)
    {
        D_801B2C54 += 1;
        func_8009ADC8();
    }
}
