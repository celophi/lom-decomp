#include "common.h"
extern u8 g_menuLayoutBuffer[];
extern s32 D_80122C00;
extern s16 D_80122C10;
void func_800B2844(s32 arg0, u8 *arg1, u8 arg2);
void func_800C6228(void)
{
    s32 count;
    s32 i;
    s32 entry_offset;
    s32 row_offset;
    u8 *menu;
    u8 *base;

    count = 0;
    i = 0;
    menu = g_menuLayoutBuffer;
    base = menu + 0x2B58;
loop:
    row_offset = i << 6;
    entry_offset = menu[D_80122C00 + 0x29D8] * 0x14C;
    if (menu[row_offset + entry_offset + 0x2B58] != 0)
    {
        entry_offset += (s32)base;
        func_800B2844(count, (u8 *)(entry_offset + row_offset), 0xFF);
        count += 1;
    }
    i += 1;
    if (i < 4)
        goto loop;
    D_80122C10 = count;
}
