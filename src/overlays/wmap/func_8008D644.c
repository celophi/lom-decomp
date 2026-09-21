#include "common.h"

extern s32 D_801B2A08;
extern s32 D_8013B20C;
extern void func_8008D644(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8008D608(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2A08 += 1;
        func_8008D644();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008D644(void)
{
    D_801B2A08 += 1;
}
