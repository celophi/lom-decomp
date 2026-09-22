#include "common.h"

extern void func_8007E5C4(void);
extern s32 D_801B2764;
extern s32 D_801B2760;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007E58C(void)
{
    D_801B2764 = 0x14;
    D_801B2760 += 1;
    func_8007E5C4();
}
