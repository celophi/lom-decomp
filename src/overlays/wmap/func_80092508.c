#include "common.h"

extern s32 D_801B2AE8;
extern s32 D_801B2AEC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80092508(void)
{
    D_801B2AE8 = 1;
    D_801B2AEC = 1;
}
