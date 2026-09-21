#include "common.h"

extern s32 D_801B2E78;
extern s32 D_801B2E7C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AB8C8(void)
{
    D_801B2E78 = 1;
    D_801B2E7C = 1;
}
