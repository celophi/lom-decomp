#include "common.h"

extern s32 D_801B2CB8;
extern s32 D_801B2CBC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009D0E4(void)
{
    D_801B2CB8 = 1;
    D_801B2CBC = 1;
}
