#include "common.h"

extern s32 D_801227E8[];
extern s32 D_80122728[];
extern s32 D_80122824[];

s32 McxCardType(s32);
s32 func_80032174(s32, void *, s32 *);
s32 func_80032888(s32, void *);
s32 bcopy(void *, void *, s32);
s32 VSync(s32);

/**
 * @see decomp.me (100%)
 */
void func_800B0094(s32 arg0)
{
    s32 card_type;
    s32 status0;
    s32 status1;
    u8 buf[8];

    if (D_801227E8[0] == 0)
    {
        card_type = McxCardType(0);
        if (card_type == 1)
        {
            func_80032174(0, &status0, &status1);
            if (status1 == 0 || status1 == 3)
            {
                if (func_80032888(0, buf) == card_type)
                {
                    func_80032174(0, &status0, &status1);
                }
                if (status1 == 0)
                {
                    bcopy(buf, D_80122728, 8);
                    D_80122824[0] = VSync(-1);
                    D_801227E8[0] = card_type;
                }
            }
        }
    }
}
