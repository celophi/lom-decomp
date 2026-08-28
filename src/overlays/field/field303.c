#include "common.h"

extern u8 D_80122C19;
extern u8 g_menuLayoutBuffer[];

/**
 * @see decomp.me (100%)
 */
void func_800C92B8(void)
{
    s32 index;
    u8 *base;
    u8 *record;

    index = (&D_80122C19)[D_80122C19 + 4];
    if (index < 5)
    {
        base = g_menuLayoutBuffer;
        record = base + index * 0x60;
        *(u32 *)(record + 0x2F38) &= 0xBFFFFFFF;
    }
    else
    {
        akao_set_song_params(0x8002, 0x4B, index, 0);
    }
}
