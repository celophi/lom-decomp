#include "common.h"

extern void func_8008A0D8(void);
extern s32 D_801B298C;
extern s32 D_801B2988;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8008A0A0(void)
{
    D_801B298C = 0x28;
    D_801B2988 += 1;
    func_8008A0D8();
}
