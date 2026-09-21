#include "common.h"

extern s32 D_801B2808;
extern s32 D_801B280C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80082718(void)
{
    D_801B2808 = 1;
    D_801B280C = 1;
}
