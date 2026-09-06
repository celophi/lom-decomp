#include "common.h"

extern u8 *D_80122B74;
extern s32 akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);
extern void func_800B2844(s32 arg0, void *arg1, s32 arg2);

/**
 * @brief Process the indexed field audio entry and report its state.
 * @param index Field audio entry index.
 * @return Entry state code, or the audio command result for indices outside the table.
 */
s32 func_800C24BC(s32 index)
{
    s32 offset;
    s32 value;
    u8 *base;

    if (index >= 5)
    {
        return akao_set_song_params(0x8001, 0x76, index, 0);
    }

    base = D_80122B74;
    offset = index * 0x60;
    if ((base + offset)[0x2EF4] != 0)
    {
        u8 *entry;

        func_800B2844(0, D_80122B74 + (offset + 0x2EF4), 0x15);
        entry = D_80122B74 + offset;
        value = *(s32 *)(entry + 0x2F38);
        if (value < 0)
        {
            if (*(u16 *)(entry + 0x2F36) != 0)
            {
                return 0;
            }
            *(s32 *)(entry + 0x2F38) = value & 0x7FFFFFFF;
            return 1;
        }
        if (((u32) value >> 30) & 1)
        {
            return 2;
        }
        return 3;
    }
    return 0xFF;
}
