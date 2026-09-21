#include "common.h"

extern s32 D_801B2A40;
extern s32 D_801B2A44;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008E470(void)
{
    D_801B2A40 = 1;
    D_801B2A44 = 1;
}
