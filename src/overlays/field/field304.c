#include "common.h"

extern u8 D_80122C11;
extern u8 g_menuLayoutBuffer[];

/**
 * @see decomp.me (100%)
 */
void func_800C9330(void)
{
    s32 index;
    u8 val;
    u8 *base;
    u8 *record;

    val = D_80122C11;
    index = (&D_80122C11)[1];
    if (index < 5)
    {
        base = g_menuLayoutBuffer;
        record = base + index * 0x60;
        record[0x2F09] = val;
        *(u32 *)(record + 0x2F38) &= 0x7FFFFFFF;
        if (record[0x2EF4] == 0)
        {
            record[0x2EF4] = 0x41;
        }
    }
    else
    {
        akao_set_song_params(0x8002, 0x4C, index, 0);
    }
}
