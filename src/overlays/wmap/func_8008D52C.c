#include "common.h"

extern s32 D_801B2A08;
extern s32 D_801B2A0C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008D52C(void)
{
    D_801B2A08 = 1;
    D_801B2A0C = 1;
}
