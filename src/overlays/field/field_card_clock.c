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
    s32 frame_offset;
    s32 current_vsync;
    u32 current_time_frames;

    if (D_801227E8[0] != 0)
    {
        (*(s16 *)arg0) = (s16)((((u8 *)D_80122728)[0] >> 4) * 1000 + (((u8 *)D_80122728)[0] & 0xF) * 100 +
                               (((u8 *)D_80122728)[1] >> 4) * 10 + (((u8 *)D_80122728)[1] & 0xF));
        arg0[2] = (s8)((((u8 *)D_80122728)[2] >> 4) * 10 + (((u8 *)D_80122728)[2] & 0xF));
        arg0[3] = (s8)((((u8 *)D_80122728)[3] >> 4) * 10 + (((u8 *)D_80122728)[3] & 0xF));
        arg0[4] = (s8)(((u8 *)D_80122728)[4] & 0xF);
        arg0[5] = (u8)((((u8 *)D_80122728)[5] >> 4) * 10 + (((u8 *)D_80122728)[5] & 0xF));
        arg0[6] = (u8)((((u8 *)D_80122728)[6] >> 4) * 10 + (((u8 *)D_80122728)[6] & 0xF));
        arg0[7] = (u8)((((u8 *)D_80122728)[7] >> 4) * 10 + (((u8 *)D_80122728)[7] & 0xF));

        current_vsync = VSync(-1);
        frame_offset = ((arg0[7] * 60) + (arg0[6] * 3600) + (arg0[5] * 216000)) - D_80122824[0];
        current_time_frames = frame_offset + current_vsync;
        arg0[5] = (u8)(current_time_frames / 216000);
        arg0[6] = (u8)((current_time_frames - (arg0[5] * 216000)) / 3600);
        arg0[7] = (u8)(((current_time_frames - (arg0[5] * 216000)) - (arg0[6] * 3600)) / 60);
        return 1;
    }

    return 0;
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
