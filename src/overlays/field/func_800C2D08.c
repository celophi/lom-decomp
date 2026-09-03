#include "common.h"

extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 *D_80122B74;
extern s32 D_801227F0;

extern void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);
extern void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
extern void func_800C2E30(s32 arg0);

/**
 * @brief Dispatch a queued gosub result, or fall back to a default song cue.
 *
 * @note 100% match with the FIELD GCC 2.8.0 G0 toolchain.
 *
 * @return D_80122B74[0xAA9] on a successful dispatch, else 0xFF.
 */
s32 func_800C2D08(void)
{
    s32 index;

    D_801227F0 = 0;
    if (g_gosub_result_count != 0)
    {
        index = g_gosub_result_values[0];
        if (index < 5)
        {
            u8 *entry = D_80122B74 + index * 0x60;

            if (entry[0x2EF4] != 0)
            {
                *(s32 *)&D_80122B74[0x2EF0] = index;
                func_800BD520(0, 0x1F10, g_gosub_result_values[0]);
                func_800C2E30(g_gosub_result_values[0]);
                return D_80122B74[0xAA9];
            }
        }
        akao_set_song_params(0x8001, 0x6E, index, 0);
    }
    return 0xFF;
}
