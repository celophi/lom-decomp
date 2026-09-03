#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    u8 unk4;
    u8 pad5;
    u16 unk6;
    u16 unk8[16];
    u8 pad28[0x68];
    s32 unk90;
} RecC1B98;

typedef struct
{
    u8 pad0[0x430];
    RecC1B98 unk430[16];
} StructC1B98;

extern StructC1B98 *D_80122B78;

/**
 * @see decomp.me (100%)
 */
RecC1B98 *func_800C1B98(s32 id)
{
    s32 i;

    if (id < 3)
    {
        return &D_80122B78->unk430[id];
    }
    if (id < 0x80)
    {
        for (i = 0; i < 16; i++)
        {
            if ((D_80122B78->unk430[i].unk90 < 0) && (D_80122B78->unk430[i].unk0 == id))
            {
                goto found;
            }
        }
        return NULL;
    }
    return &D_80122B78->unk430[id - 0x70];
found:
    return &D_80122B78->unk430[i];
}

/**
 * @see decomp.me (100%)
 */
RecC1B98 *func_800C1C50(s32 id)
{
    s32 i;
    s32 j;

    for (i = 0; i < 16; i++)
    {
        if (D_80122B78->unk430[i].unk90 >= 0)
        {
            D_80122B78->unk430[i].unk0 = id;
            D_80122B78->unk430[i].unk4 = 0xFF;
            D_80122B78->unk430[i].unk6 = 0xFFFF;
            D_80122B78->unk430[i].unk90 &= ~0xF;
            D_80122B78->unk430[i].unk90 &= ~0x20000000;
            D_80122B78->unk430[i].unk90 &= ~0x40000000;
            D_80122B78->unk430[i].unk90 |= 0x80000000;
            for (j = 0; j < 16; j++)
            {
                D_80122B78->unk430[i].unk8[j] = 0xFFFF;
            }
            return &D_80122B78->unk430[i];
        }
    }
    return NULL;
}
