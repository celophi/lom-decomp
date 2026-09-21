#include "common.h"

extern s32 D_801B2908;
extern s32 D_8013B20C;
extern void func_8008772C(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800876F0(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2908 += 1;
        func_8008772C();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008772C(void)
{
    D_801B2908 += 1;
}
