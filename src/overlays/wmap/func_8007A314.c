#include "common.h"

extern s32 D_801B2698;
extern s32 D_801B269C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007A314(void)
{
    D_801B2698 = 1;
    D_801B269C = 1;
}
