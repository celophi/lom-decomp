#include "common.h"

extern s32 D_801B27E8;
extern s32 D_8013B20C;
extern void func_80081D30(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80081CF4(void)
{
    if (D_8013B20C == 0)
    {
        D_801B27E8 += 1;
        func_80081D30();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80081D30(void)
{
    D_801B27E8 += 1;
}
