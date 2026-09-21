#include "common.h"

extern s32 D_801B2AF0;
extern s32 D_801B2AF4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80092770(void)
{
    D_801B2AF0 = 1;
    D_801B2AF4 = 1;
}
