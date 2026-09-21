#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B28B0;
extern void func_80085B80(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80085B44(void)
{
    if (D_8013B20C == 0)
    {
        D_801B28B0 += 1;
        func_80085B80();
    }
}
