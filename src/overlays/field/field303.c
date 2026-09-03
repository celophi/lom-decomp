#include "common.h"

extern u8 D_80122C19;
extern u8 D_80122C11;
extern u8 g_menuLayoutBuffer[];
extern s32 func_800BD414(s32 arg0, s32 arg1);
extern void func_800AD194(s32 arg0);

/** @see decomp.me (100%) */
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

/** @see decomp.me (100%) */
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

/**
 * @brief Checks field state 0x2F08 and performs the corresponding update.
 */
void func_800C93B4(void)
{
    if (func_800BD414(0, 0x2F08) == 0x80)
    {
        func_800AD194(1);
    }
    else if (func_800BD414(0, 0x2F08) == 0xFF)
    {
        func_800AD194(0);
    }
}
