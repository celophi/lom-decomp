#include "common.h"

extern s32 D_801B2728;
extern s32 D_801B272C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007CD48(void)
{
    D_801B2728 = 1;
    D_801B272C = 1;
}
