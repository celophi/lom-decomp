#include "common.h"

typedef struct
{
    s32 unk0; /* 0x00 */
    u8 pad4[0x8 - 0x4];
    s32 unk8; /* 0x08 */
} Struct_UnkVec8;

typedef struct
{
    u8 pad0[0x50];
    s32 unk50;
    u8 pad54[4];
    s32 unk58;
} Obj80087F0C;

/** @brief View of D_80122B74 exposing the 64-word bitset at 0x2E8. */
typedef struct
{
    u8 pad0[0x2E8];
    s32 arr2E8[0x40]; /* 0x2E8 */
} StructB74;

#define FLAG_BITSET ((StructB74 *)D_80122B74)

Obj80087F0C *func_80087F0C(s32 arg0);
s32 func_80087F44(s32 arg0, s32 *out);
void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);
void func_800B2844(s32, void *, s32);
void func_800C2228(s32 idx);

extern u8 *D_80122B74;
extern u16 D_800F0E98[];

/**
 * @brief Sort a (value, key) pair list in place by ascending key.
 * @param arg0 Word 0 holds the pair count; pairs follow as (value, key) from word 1.
 */
void func_800C1F28(u32 *arg0)
{
    u32 i;
    u32 j;
    u32 key;
    u32 data;

    for (i = 0; i < arg0[0]; i++)
    {
        for (j = 1; j < arg0[0]; j++)
        {
            key = arg0[2 * i + 2];
            if (arg0[2 * j + 2] < key)
            {
                data = arg0[2 * i + 1];
                arg0[2 * i + 1] = arg0[2 * j + 1];
                arg0[2 * i + 2] = arg0[2 * j + 2];
                arg0[2 * j + 1] = data;
                arg0[2 * j + 2] = key;
            }
        }
    }
}

/**
 * @brief Manhattan distance between two positions on the X and Z axes.
 * @param arg0 First position.
 * @param arg1 Second position.
 * @return |dx| + |dz|.
 * @see decomp.me (100%) TODO
 */
s32 func_800C1FBC(Struct_UnkVec8 *arg0, Struct_UnkVec8 *arg1)
{
    s32 dx;
    s32 dz;

    dx = arg0->unk0 - arg1->unk0;
    if (dx < 0)
    {
        dx = -dx;
    }

    dz = arg0->unk8 - arg1->unk8;
    if (dz < 0)
    {
        dz = -dz;
    }

    return dx + dz;
}

/**
 * @brief Test whether an object's stored position lies within a box around its live position.
 * @param arg0 Object id.
 * @param arg1 Half-width on X.
 * @param arg2 Half-width on Z.
 * @return -1 when inside the box, 0 otherwise.
 */
s32 func_800C1FFC(s32 arg0, s32 arg1, s32 arg2)
{
    Obj80087F0C *obj;
    s32 buf[4];
    s32 bx;
    s32 by;

    obj = func_80087F0C(arg0);
    func_80087F44(arg0, buf);
    bx = buf[0];
    if ((bx - arg1) < obj->unk50 && obj->unk50 < (bx + arg1))
    {
        by = buf[2];
        if ((by - arg2) < obj->unk58 && obj->unk58 < (by + arg2))
        {
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Set bit arg0 in the 64-word bitset at 0x2E8.
 * @param arg0 Bit index.
 * @return Always -1.
 */
s32 func_800C2094(s32 arg0)
{
    s32 word;
    s32 bit;
    s32 q;

    q = arg0 / 32;
    bit = arg0 % 32;
    word = q;
    FLAG_BITSET->arr2E8[word] |= 1 << bit;
    return -1;
}

/**
 * @brief Refresh a party member's entry and return its counter byte.
 * @param arg0 Party member index, or >= 0xFF to take the song fallback.
 * @return The counter at 0x25E0 + arg0, or 0 for an invalid index.
 */
u8 func_800C20D8(s32 arg0)
{
    if (arg0 < 0xFF)
    {
        u8 *p;

        func_800C2228(arg0);
        p = D_80122B74 + arg0;
        return p[0x25E0];
    }
    akao_set_song_params(0x8001, 0x70, arg0, 0);
    return 0;
}

/**
 * @brief Increment a party member's counter, saturating at 0x63, then refresh its entry.
 * @param arg0 Party member index, or >= 0xFF to take the song fallback.
 */
void func_800C2138(s32 arg0)
{
    u8 *p;
    u8 *q;

    if (arg0 < 0xFF)
    {
        p = D_80122B74 + arg0;
        p[0x25E0] += 1;
        q = D_80122B74 + arg0;
        if (q[0x25E0] >= 0x64)
        {
            q[0x25E0] = 0x63;
        }
        func_800C2228(arg0);
    }
    else
    {
        akao_set_song_params(0x8001, 0x71, arg0, 0);
    }
}

/**
 * @brief Ticks down a party member's counter, or triggers a fallback song.
 *
 * For a valid member index (@p arg0 < 0xFF), decrements the counter byte at
 * @c D_80122B74[arg0 + 0x25E0] when nonzero and runs func_800C2228; for an
 * out-of-range index, issues akao_set_song_params(0x8001, 0x72, arg0, 0).
 *
 * @param arg0 Party member index, or >= 0xFF to take the song fallback.
 */
void func_800C21C0(s32 arg0)
{
    u8 *rec;
    u8 v;

    if (arg0 < 0xFF)
    {
        rec = D_80122B74 + arg0;
        v = rec[0x25E0];
        if (v != 0)
        {
            rec[0x25E0] = v - 1;
        }
        func_800C2228(arg0);
    }
    else
    {
        akao_set_song_params(0x8001, 0x72, arg0, 0);
    }
}

/**
 * @brief Dispatch the D_800F0E98 entry for a party member index.
 * @param idx Party member index.
 */
void func_800C2228(s32 idx)
{
    func_800B2844(0, (u8 *)D_800F0E98 + D_800F0E98[idx], 0x15);
}
