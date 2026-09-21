#include "common.h"

extern s32 D_801B2768;
extern s32 D_801B276C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007E7AC(void)
{
    D_801B2768 = 1;
    D_801B276C = 1;
}
