#include "common.h"

extern s32 D_801B2A68;
extern s32 D_8013B20C;
extern void func_8008FEAC(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8008FE70(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2A68 += 1;
        func_8008FEAC();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008FEAC(void)
{
    D_801B2A68 += 1;
}
