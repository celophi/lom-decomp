#include "common.h"

extern void func_8008EC34(void);
extern s32 D_800DCEA8;
extern s32 D_801B2A64;
extern s32 D_801B2A60;

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_8008EBF4(void)
{
    D_800DCEA8 = 0;
    D_801B2A64 = 0x40;
    D_801B2A60 += 1;
    func_8008EC34();
}
