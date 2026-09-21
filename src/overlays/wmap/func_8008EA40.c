#include "common.h"

extern s32 D_801B2A60;
extern s32 D_801B2A64;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008EA40(void)
{
    D_801B2A60 = 1;
    D_801B2A64 = 1;
}
