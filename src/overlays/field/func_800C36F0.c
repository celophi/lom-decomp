#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
} PairC36F0;

typedef struct
{
    u8 unk0;
    u8 pad1[0x37];
    s32 unk38;
    s32 unk3C;
} RecC36F0;

typedef struct
{
    u8 pad0[0x640];
    RecC36F0 unk640[8];
    u8 pad840[0x4A0];
    RecC36F0 unkCE0[100];
} StructC36F0;

extern StructC36F0 *D_80122B74;

/**
 * @see decomp.me (100%)
 */
s32 func_800C36F0(PairC36F0 *key)
{
    s32 i;
    s32 a;
    s32 b;

    a = key->unk0;
    b = key->unk4;

    for (i = 0; i < 100; i++)
    {
        if ((D_80122B74->unkCE0[i].unk0 != 0) && (D_80122B74->unkCE0[i].unk38 == a) && (D_80122B74->unkCE0[i].unk3C == b))
        {
            return 1;
        }
    }

    for (i = 0; i < 8; i++)
    {
        if ((D_80122B74->unk640[i].unk0 != 0) && (D_80122B74->unk640[i].unk38 == a) && (D_80122B74->unk640[i].unk3C == b))
        {
            return 1;
        }
    }

    return 0;
}

/**
 * @see decomp.me (100%)
 */
void func_800C37A8(u32 seed, PairC36F0 *out)
{
    PairC36F0 key;
    u32 value;
    u32 hi;
    u32 lo;
    u32 mask_hi;
    u32 mask_lo;

    mask_hi = 0xF0F0F0F0;
    hi = seed & mask_hi;
    mask_lo = 0x0F0F0F0F;
    lo = seed & mask_lo;

    do
    {
        value = rand();
        value += rand() << 16;
        key.unk0 = hi | (value & mask_lo);
        key.unk4 = lo | (value & mask_hi);
    } while (func_800C36F0(&key) != 0);

    out->unk0 = key.unk0;
    out->unk4 = key.unk4;
}
