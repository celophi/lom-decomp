#include "common.h"

extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 *D_80122B74;
extern s32 D_801227F0;
extern void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);
extern void func_800B2844(s32 arg0, void *arg1, s32 arg2);

s32 func_800C23F4(void)
{
    s32 index;

    D_801227F0 = 0;
    if (g_gosub_result_count != 0)
    {
        index = g_gosub_result_values[0];
        if (index < 5)
        {
            u8 **buffer = &D_80122B74;
            s32 offset = index * 0x60 + 0x2EF4;
            func_800B2844(0, *buffer + offset, 0x15);
            index = g_gosub_result_values[0];
            if (index != *(s32 *)(*buffer + 0x2EF0))
            {
                {
                    u8 *entry = *buffer;
                    entry += index * 0x60;
                    entry[0x2EF4] = 0;
                }
                return g_gosub_result_values[0];
            }
            return 5;
        }
        akao_set_song_params(0x8001, 0x6E, index, 0);
    }
    return 0xFF;
}
