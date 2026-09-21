#include "common.h"

extern void func_8007B020(void);
extern s32 D_801B26DC;
extern s32 D_801B26D8;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007AFE8(void)
{
    D_801B26DC = 0x10;
    D_801B26D8 += 1;
    func_8007B020();
}
