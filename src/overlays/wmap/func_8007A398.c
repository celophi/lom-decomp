#include "common.h"

extern void func_8007A3D0(void);
extern s32 D_801B269C;
extern s32 D_801B2698;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007A398(void)
{
    D_801B269C = 0x10;
    D_801B2698 += 1;
    func_8007A3D0();
}
