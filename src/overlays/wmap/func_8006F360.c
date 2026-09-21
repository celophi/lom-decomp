#include "common.h"

extern void func_8006E190(void);
extern s32 D_801B2424;
extern s32 D_801B2420;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8006F360(void)
{
    D_801B2424 = 0x20;
    D_801B2420 += 1;
    func_8006E190();
}
