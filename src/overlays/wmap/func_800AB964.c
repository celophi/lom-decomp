#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2E78;
extern void func_800AB9A0(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800AB964(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2E78 += 1;
        func_800AB9A0();
    }
}
