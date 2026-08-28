#include "common.h"

extern s32 D_80164B78;
extern s32 D_80164B70;
extern u8 D_80165018[];
extern char D_800ECF7C[];
extern char D_800ECF8C[];

extern s32 func_8001714C(void *, void *, s32);

s32 func_80144BF8(void)
{
    s32 i;
    u8 *entry;

    i = 0;
    if (D_80164B78 > 0)
    {
        do
        {
            entry = D_80165018 + i * 0x28;
            if (func_8001714C(&D_800ECF7C, (void *)(D_80164B70 * 0x320 + (s32)entry), 0xC) == 0 ||
                func_8001714C(&D_800ECF8C, (void *)(D_80164B70 * 0x320 + (s32)entry), 0xC) == 0)
            {
                return 1;
            }
            i++;
        } while (i < D_80164B78);
    }
    return 0;
}
