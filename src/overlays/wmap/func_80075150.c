#include "common.h"

extern s32 D_801B2568;
extern s32 D_8013B20C;
extern void func_80075150(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80075114(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2568 += 1;
        func_80075150();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80075150(void)
{
    D_801B2568 += 1;
}
