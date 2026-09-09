#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern s32 D_80122C00;
extern void *func_800A9060(void);
extern void func_800A8F8C(void *, void *);
/**
 * @brief Clear the selected menu group and release its four active records.
 * @note Refresh the selection after each allocation call and preserve the packed count.
 */
void func_800C5BCC(void)
{
    s32 i, offset, group_offset;
    u8 *layout, *scan, *entries, *final_layout, *first_layout, *loop_base;
    u32 flags;
    void *item;
    first_layout = g_menuLayoutBuffer;
    i = 0;
    if (first_layout[0x29D6] != 0)
    {
        loop_base = first_layout;
        scan = loop_base;
        do
        {
            flags = *(u32 *)(loop_base + i * 4 + 0x29DC);
            if ((flags & 3) == loop_base[D_80122C00 + 0x29D8])
            {
                *(u32 *)(loop_base + i * 4 + 0x29DC) = (flags | 3) & 0xFFFEFFFF;
            }
            i++;
            scan += 4;
        } while (i < loop_base[0x29D6]);
    }
    i = 0;
    do
    {
        group_offset = g_menuLayoutBuffer[D_80122C00 + 0x29D8] * 0x14C;
        layout = g_menuLayoutBuffer;
        entries = layout + 0x2B58;
        offset = i * 0x40;
        if (layout[offset + group_offset + 0x2B58] != 0)
        {
            if (func_800A9060() != 0)
            {
                item = func_800A9060();
                group_offset = layout[D_80122C00 + 0x29D8] * 0x14C;
                func_800A8F8C(item, group_offset + entries + offset);
            }
        }
        i++;
    } while (i < 4);
    final_layout = g_menuLayoutBuffer;
    final_layout[final_layout[D_80122C00 + 0x29D8] * 0x14C + 0x2B0C] = 0;
    final_layout[D_80122C00 + 0x29D8] = 3;
    flags = *(u32 *)(final_layout + 0x29D4);
    if ((flags & 0xF) != 0)
    {
        *(u32 *)(final_layout + 0x29D4) =
            (flags & ~0xF) | (((final_layout[0x29D4] & 0xF) - 1) & 0xF);
    }
}
