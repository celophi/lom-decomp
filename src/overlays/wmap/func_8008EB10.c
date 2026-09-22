#include "common.h"

extern void func_8008EB48(void);
extern s32 D_801B2A64;
extern s32 D_801B2A60;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8008EB10(void)
{
    D_801B2A64 = 0x40;
    D_801B2A60 += 1;
    func_8008EB48();
}
