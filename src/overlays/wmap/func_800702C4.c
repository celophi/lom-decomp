#include "common.h"

extern s16 D_801398CC;
extern s32 D_801B2448;

/**
 * @brief Clear the sequence flag and advance the step counter.
 */
void func_800702C4(void)
{
    D_801398CC = 0;
    D_801B2448 += 1;
}
