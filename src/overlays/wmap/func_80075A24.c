#include "common.h"

extern s32 D_801B2588;
extern s32 D_801B258C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80075A24(void)
{
    D_801B2588 = 1;
    D_801B258C = 1;
}
