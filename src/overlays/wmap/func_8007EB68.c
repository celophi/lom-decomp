#include "common.h"

extern void func_8007EBA8(void);
extern s32 D_800DCEA8;
extern s32 D_801B277C;
extern s32 D_801B2778;

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007EB68(void)
{
    D_800DCEA8 = 0;
    D_801B277C = 0x40;
    D_801B2778 += 1;
    func_8007EBA8();
}
