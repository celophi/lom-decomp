#include "common.h"

extern s32 D_801B2930;
extern s32 D_801B2934;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80088080(void)
{
    D_801B2930 = 1;
    D_801B2934 = 1;
}
