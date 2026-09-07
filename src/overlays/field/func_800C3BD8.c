#include "common.h"

typedef union
{
    u32 raw;
    struct
    {
        unsigned mode : 2;
        unsigned pad2 : 14;
        unsigned enabled : 1;
        unsigned rest : 15;
    } bits;
} MenuWord;

extern u8 g_menuLayoutBuffer[];

void func_800C3CB4(void);
void func_800C3D38(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Clear menu layout state and process enabled entries for the requested mode.
 * @param arg0 Menu entry mode to process.
 */
void func_800C3BD8(s32 arg0)
{
    s32 i;
    u8 *v1;
    u8 *s1;
    u8 *s2;
    u32 a3;
    MenuWord word;

    i = 0;
    do
    {
        v1 = g_menuLayoutBuffer + i * 4;
        v1[0x2A7C] = 0;
        v1[0x2A7D] = 0;
        i += 1;
    } while (i < 0x24);

    if (arg0 != 3)
    {
        i = 0;
        if (g_menuLayoutBuffer[0x29D6] != 0)
        {
            s2 = g_menuLayoutBuffer;
            s1 = s2;
        loop:
            a3 = *(u32 *)(s1 + 0x29DC);
            word.raw = a3;
            if ((word.bits.enabled == 1) && (word.bits.mode == arg0))
            {
                func_800C3D38(i, (a3 >> 0x11) & 3, (s32)(a3 << 8) >> 0x1B, (s32)(a3 << 3) >> 0x1B);
            }
            s1 += 4;
            i += 1;
            if (i < s2[0x29D6])
            {
                goto loop;
            }
        }
        func_800C3CB4();
    }
}
