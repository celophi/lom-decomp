#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern u8 D_80122C1F;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];

void func_800A8F8C(void *arg0, u8 *arg1);
void func_800C2A88(s32 arg0);

/**
 * @brief Move the gosub-selected menu record into the first available active slot.
 * @note D_80122C1F reports no result (0), copied (1), empty (2), or duplicate (3).
 * @note WIP: redundant address computation and instruction-order differences remain.
 */
void func_800C80BC(void)
{
    s32 sel;
    u8 *sel_rec;
    s32 i;
    s32 v0;

    D_80122C1F = 0;
    if (g_gosub_result_count == 0)
    {
        return;
    }

    sel = g_gosub_result_values[0];
    sel_rec = g_menuLayoutBuffer + sel * 0x40;

    if (*(s32 *)(sel_rec + 0xD18) == 0)
    {
        if (*(s32 *)(sel_rec + 0xD1C) == 0)
        {
            D_80122C1F = 2;
            return;
        }
    }

    for (i = 0; i < 4; i++)
    {
        u8 *rowp = (u8 *)((s32)g_menuLayoutBuffer + i * 0x40);
        if (rowp[0x3160] != 0)
        {
            if (*(s32 *)(sel_rec + 0xD18) == *(s32 *)(rowp + 0x3198))
            {
                if (*(s32 *)(sel_rec + 0xD1C) == *(s32 *)(rowp + 0x319C))
                {
                    D_80122C1F = 3;
                    return;
                }
            }
        }
    }

    for (i = 0; i < 4; i++)
    {
        u8 *rowp = (u8 *)((s32)g_menuLayoutBuffer + i * 0x40);
        if (rowp[0x3160] == 0)
        {
            func_800A8F8C(g_menuLayoutBuffer + i * 0x40 + 0x3160, g_menuLayoutBuffer + 0xCE0 + sel * 0x40);
            func_800C2A88(sel);
            v0 = *(s32 *)(rowp + 0x3194);
            if (v0 == 0)
            {
                v0 = 1;
            }
            *(s32 *)(rowp + 0x3194) = v0;
            D_80122C1F = 1;
            return;
        }
    }
}
