#include "common.h"

extern s32 D_801B2CB0;
extern s32 D_801B2CB4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009CEDC(void)
{
    D_801B2CB0 = 1;
    D_801B2CB4 = 1;
}
