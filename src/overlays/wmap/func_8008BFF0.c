#include "common.h"

extern s32 D_801B29D8;
extern s32 D_801B29DC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008BFF0(void)
{
    D_801B29D8 = 1;
    D_801B29DC = 1;
}
