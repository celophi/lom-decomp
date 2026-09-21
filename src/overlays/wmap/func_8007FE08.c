#include "common.h"

extern s32 D_801B27A0;
extern s32 D_801B27A4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007FE08(void)
{
    D_801B27A0 = 1;
    D_801B27A4 = 1;
}
