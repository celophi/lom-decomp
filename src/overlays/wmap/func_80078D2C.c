#include "common.h"

extern void func_80078D64(void);
extern s32 D_801B264C;
extern s32 D_801B2648;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80078D2C(void)
{
    D_801B264C = 0x64;
    D_801B2648 += 1;
    func_80078D64();
}
