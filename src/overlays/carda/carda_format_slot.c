#include "common.h"

extern s32 D_801660A0;
extern s32 D_80166B88;
extern s32 D_80165FF4;
extern u8 D_80166440[];
extern char D_800ECFC4[];

void func_80149554(void)
{
    s32 i;
    u8 scratch[16];

    i = 0;
    do
    {
        if (func_80017B3C(D_801660A0 << 4) != 0)
            break;
        i++;
    } while (i < 0x14);

    D_80166B88 = 0;
    D_80165FF4 = 0;
    func_800170BC(D_80166440 + D_801660A0 * 0x320, D_800ECFC4);
}
