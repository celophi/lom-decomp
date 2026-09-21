#include "common.h"

extern s32 D_801B2CE0;
extern s32 D_801B2CE4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009ECF4(void)
{
    D_801B2CE0 = 1;
    D_801B2CE4 = 1;
}
