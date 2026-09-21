#include "common.h"

extern s32 D_801B2CF0;
extern s32 D_801B2CF4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009EFEC(void)
{
    D_801B2CF0 = 1;
    D_801B2CF4 = 1;
}
