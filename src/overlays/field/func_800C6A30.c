#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern s16 D_80122C10;

/**
 * @brief Counts active menu-layout slots whose packed field matches D_80122C10.
 *
 * Scans all 100 slots of @c g_menuLayoutBuffer (stride 0x40). A slot counts
 * when its active byte at +0xCE0 is nonzero and bits 8-9 of the packed word at
 * +0xCF4 equal the value in @c D_80122C10 (read once up front). The total is
 * written back to @c D_80122C10.
 */
void func_800C6A30(void)
{
    s32 i;
    s32 count;
    u8 *p;
    u32 field;
    s32 target_val;

    target_val = D_80122C10;
    count = 0;
    for (i = 0; i < 0x64; i++)
    {
        p = &g_menuLayoutBuffer[i * 0x40];
        if (p[0xCE0] != 0)
        {
            field = *(u32 *) &p[0xCF4];
            field = (field >> 8) & 3;
            if (field == target_val)
            {
                count++;
            }
        }
    }
    D_80122C10 = count;
}
