#include "common.h"

extern void func_80075D1C(void);
extern s32 D_801B2594;
extern s32 D_801B2590;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80075CE4(void)
{
    D_801B2594 = 0x20;
    D_801B2590 += 1;
    func_80075D1C();
}
