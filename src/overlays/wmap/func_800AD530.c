#include "common.h"

extern s32 D_801B2ED8;
extern s32 D_801B2EDC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AD530(void)
{
    D_801B2ED8 = 1;
    D_801B2EDC = 1;
}
