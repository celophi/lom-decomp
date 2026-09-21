#include "common.h"

extern s32 D_801B29C8;
extern s32 D_801B29CC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008BD40(void)
{
    D_801B29C8 = 1;
    D_801B29CC = 1;
}
