#include "common.h"

extern s32 D_801B2790;
extern s32 D_801B2794;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007FB54(void)
{
    D_801B2790 = 1;
    D_801B2794 = 1;
}
