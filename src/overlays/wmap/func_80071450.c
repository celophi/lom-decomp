#include "common.h"

extern s32 D_801B24B8;
extern s32 D_8013B20C;
extern void func_80071450(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80071414(void)
{
    if (D_8013B20C == 0)
    {
        D_801B24B8 += 1;
        func_80071450();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80071450(void)
{
    D_801B24B8 += 1;
}
