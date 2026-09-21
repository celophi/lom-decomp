#include "common.h"

extern s32 D_801B2FC8;
extern s32 D_801B2FCC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B3098(void)
{
    D_801B2FC8 = 1;
    D_801B2FCC = 1;
}
