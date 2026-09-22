#include "common.h"

extern void func_80070260(void);
extern s32 D_801B244C;
extern s32 D_801B2448;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80070228(void)
{
    D_801B244C = 0x80;
    D_801B2448 += 1;
    func_80070260();
}
