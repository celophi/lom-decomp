#include "common.h"

extern s32 D_801B26C8;
extern s32 D_801B26CC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007AC20(void)
{
    D_801B26C8 = 1;
    D_801B26CC = 1;
}
