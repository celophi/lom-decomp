#include "common.h"

extern s32 D_80165FEC;
extern s32 D_80166078;
extern s32 D_801660A0;
extern u8 D_80166440[];
extern char D_800ECF7C[];
extern char D_800ECF8C[];

s32 func_80147C94(void)
{
    s32 i;
    u8 *entry;

    i = 0;
    if (D_80165FEC > 0)
    {
        do
        {
            entry = D_80166440 + i * 0x28;
            if (D_80166078 != 2)
            {
                if (func_8001714C(&D_800ECF7C, (void *)(D_801660A0 * 0x320 + (s32)entry), 0xC) == 0 ||
                    func_8001714C(&D_800ECF8C, (void *)(D_801660A0 * 0x320 + (s32)entry), 0xC) == 0)
                    return 1;
            }
            if (D_80166078 == 2 &&
                func_8001714C(&D_800ECF8C, (void *)(D_801660A0 * 0x320 + (s32)entry), 0xC) == 0)
                return 1;
            i++;
        } while (i < D_80165FEC);
    }
    return 0;
}
