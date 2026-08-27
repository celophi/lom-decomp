#include "common.h"

typedef struct
{
    u8 pad0[0x25];
    u8 unk25;    /* 0x25 */
    u8 pad26[0x2A - 0x26];
    s16 unk2A;   /* 0x2A */
    u8 pad2C[0x54 - 0x2C];
} Entry;

extern Entry D_800FDF58[];

s32 func_80096A00(void)
{
    s32 i;

    for (i = 0; i < 13; i++)
    {
        if (D_800FDF58[i].unk25 != 0xFF)
        {
            if (D_800FDF58[i].unk2A == 0x90 || D_800FDF58[i].unk2A == 0x94 ||
                D_800FDF58[i].unk2A == 0x93 || D_800FDF58[i].unk2A == 0xAE ||
                D_800FDF58[i].unk2A == 0x94 || D_800FDF58[i].unk2A == 0x92)
            {
                return i + 0x100;
            }
        }
    }
    return 0;
}
