#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2A68;
extern void func_8008FE2C(void);
extern void func_8006CAC0(void (*step)(void));
extern void func_8008FEC4(void);
extern void func_8008FE70(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8008FDF0(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2A68 += 1;
        func_8008FE2C();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8008FE2C(void)
{
    func_8006CAC0(func_8008FEC4);
    D_8013B20C = 1;
    D_801B2A68 += 1;
    func_8008FE70();
}
