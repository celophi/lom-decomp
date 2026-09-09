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
 * @brief Decode the cached card clock and advance its time by elapsed frames.
 * @param arg0 Eight-byte output containing a halfword year and six byte fields.
 * @return One when the clock is available, otherwise zero.
 */
s32 func_800AFE14(u8 *arg0)
{
    s32 temp_a1_2;
    s32 temp_v0;
    u32 temp_a1;
    u32 temp_a1_3;

    if (D_801227E8[0] == 0)

    {
        return 0;
    }
    temp_a1 = (u8) ((u8 *)D_80122728)[1] >> 4;
    (*(s16 *)arg0) = (s16) ((((u8 *)D_80122728)[1] & 0xF) + ((((u8) ((u8 *)D_80122728)[0] >> 4) * 0x3E8) + ((((u8 *)D_80122728)[0] & 0xF) * 0x64) + (temp_a1 * 0xA)));
    arg0[2] = (s8) ((((u8) ((u8 *)D_80122728)[2] >> 4) * 0xA) + (((u8 *)D_80122728)[2] & 0xF));
    arg0[3] = (s8) ((((u8) ((u8 *)D_80122728)[3] >> 4) * 0xA) + (((u8 *)D_80122728)[3] & 0xF));
    arg0[4] = (s8) (((u8 *)D_80122728)[4] & 0xF);
    arg0[5] = (u8) ((((u8) ((u8 *)D_80122728)[5] >> 4) * 0xA) + (((u8 *)D_80122728)[5] & 0xF));
    arg0[6] = (u8) ((((u8) ((u8 *)D_80122728)[6] >> 4) * 0xA) + (((u8 *)D_80122728)[6] & 0xF));
    arg0[7] = (u8) ((((u8) ((u8 *)D_80122728)[7] >> 4) * 0xA) + (((u8 *)D_80122728)[7] & 0xF));
    temp_v0 = VSync(-1);
    temp_a1_2 = ((arg0[7] * 0x3C) + (arg0[6] * 0xE10) + (arg0[5] * 0x34BC0)) - D_80122824[0];
    temp_a1_3 = temp_a1_2 + temp_v0;
    arg0[5] = (u8) ((temp_a1_2 + temp_v0) / 216000);
    arg0[6] = (u8) ((temp_a1_3 - (arg0[5] * 0x34BC0)) / 3600);
    arg0[7] = (u8) (((temp_a1_3 - (arg0[5] * 0x34BC0)) - (arg0[6] * 0xE10)) / 60);
    return 1;
}

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
