#include "common.h"

extern void func_800996B0(void);
extern s32* D_80139280;
extern s32 D_801B2C44;
extern s32 D_801B2C40;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80099668(void)
{
    D_801B2C44 = 0x20;
    D_80139280[35] = -1;
    D_801B2C40 += 1;
    func_800996B0();
}
