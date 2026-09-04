#include "common.h"

typedef struct {
    u8 pad0[0x25];
    u8 unk25;
    u8 pad26[0x2A - 0x26];
    s16 unk2A;
    u8 pad2C[0x54 - 0x2C];
} Entry;

typedef struct {
    u8 pad0[0x23A];
    u8 unk23A;
    u8 pad23B[0x244 - 0x23B];
} Actor;

extern Entry D_800FDF58[];
extern Actor g_field_actor_slots[];
extern s32 D_8010D020;

s32 func_80096A90(void)
{
    s32 i;

    if (D_8010D020 == 0)
        return 0;

    for (i = 0; i < 2; i++)
    {
        if (D_800FDF58[i].unk25 != 0xFF)
        {
            if (D_800FDF58[i].unk2A == 0x90 || D_800FDF58[i].unk2A == 0x94 ||
                D_800FDF58[i].unk2A == 0x93 || D_800FDF58[i].unk2A == 0xAE ||
                D_800FDF58[i].unk2A == 0x94 || D_800FDF58[i].unk2A == 0x8E ||
                D_800FDF58[i].unk2A == 0x92)
            {
                if (g_field_actor_slots[i + 64].unk23A != 0)
                    return 1;
            }
        }
    }
    return 0;
}
