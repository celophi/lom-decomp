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
