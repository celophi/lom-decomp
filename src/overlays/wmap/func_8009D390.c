#include "common.h"

extern void func_8009D3D8(void);
extern s32* D_80139280;
extern s32 D_801B2CC4;
extern s32 D_801B2CC0;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009D390(void)
{
    D_801B2CC4 = 0x20;
    D_80139280[45] = -1;
    D_801B2CC0 += 1;
    func_8009D3D8();
}
