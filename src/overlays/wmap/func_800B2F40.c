#include "common.h"

extern s32 D_801B2FC0;
extern s32 D_801B2FC4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B2F40(void)
{
    D_801B2FC0 = 1;
    D_801B2FC4 = 1;
}
