#include "common.h"

extern s32 D_801B26D8;
extern s32 D_801B26DC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007AF54(void)
{
    D_801B26D8 = 1;
    D_801B26DC = 1;
}
