#include "common.h"

extern s32 D_801B29A8;
extern s32 D_8013B20C;
extern void func_8008B3EC(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8008B3B0(void)
{
    if (D_8013B20C == 0)
    {
        D_801B29A8 += 1;
        func_8008B3EC();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008B3EC(void)
{
    D_801B29A8 += 1;
}
