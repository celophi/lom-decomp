#include "common.h"

extern s32 D_801B29F8;
extern s32 D_801B29FC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008C540(void)
{
    D_801B29F8 = 1;
    D_801B29FC = 1;
}
