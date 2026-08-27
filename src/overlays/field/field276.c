#include "common.h"

extern u32 *func_800875B4(void);
extern void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);

u16 *func_800C1E40(s32 arg0)
{
    u32 *base;
    u16 *record;
    s32 i;
    u32 count;

    base = func_800875B4();
    count = (u32) base[0] >> 2;
    for (i = 0; i < (s32) count; i += 1)
    {
        record = (u16 *) ((u8 *) base + base[i]);
        if (*record == arg0)
        {
            return record;
        }
    }
    akao_set_song_params(0x8001, 0x6B, arg0, 0);
    return NULL;
}
