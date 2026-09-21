#include "common.h"

extern s32 D_801B29E8;
extern s32 D_801B29EC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008C248(void)
{
    D_801B29E8 = 1;
    D_801B29EC = 1;
}
