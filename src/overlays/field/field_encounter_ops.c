#include "common.h"

/** @brief Record with a counter word at 0x4 and an id at 0x14 (func_800B30B8). */
typedef struct
{
    char pad0[4];
    s32 unk4;   /* 0x4 */
    char pad4[0x14 - 8];
    s32 unk14;  /* 0x14 */
} SomeStruct;

typedef struct
{
    u32 cap;
    u32 value;
} SaturatingCounter;

typedef struct
{
    u8 pad0[4];
    u8 unk4;
} UnkStruct800B313C;

/** @brief Flag word owned by a field record. */
typedef struct StateB3160
{
    u8 pad0[0xC];
    u32 flags;
} StateB3160;

/** @brief Field record containing a flag owner and indexed halfword states. */
typedef struct RecordB3160
{
    u8 pad0[0x10];
    StateB3160 *state;
} RecordB3160;

/** @brief Sub-record of func_800B2A9C's result; unk48 is a 0..0xFF gauge. */
typedef struct
{
    u8 pad[0x48];
    u16 unk48;
} RecordB2A9CSub;

typedef struct
{
    u8 pad[0x10];
    RecordB2A9CSub *unk10;
} RecordB2A9C;

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    u8 *unk4;
    u8 *unk8;
    u8 unkC[8];
    s32 unk14;
    s32 unk18;
} StructB3580;

/** @brief View of the D_80122B74 block: a byte at 0x2E5 and 0xC-byte rows at 0x2F4. */
typedef struct
{
    u8 pad[0x2E5];
    u8 unk2E5;
    u8 pad2E6[0x2F4 - 0x2E6];
    u8 unk2F4[1][0xC];
} StructB74;

#define FIELD_B74 ((StructB74 *)D_80122B74)

s32 func_800B2D34(u8 *arg0, s32 arg1);
s32 func_8008B288(void *arg0);
s32 func_80087F44(void *arg0, s32 *out);
void func_80089D44(u8 arg0);
void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);
RecordB2A9C *func_800B2A9C(s32 value);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
u32 func_800C9ED4(s32 arg0);
s32 func_800B37D4(void);
s32 func_800B3DF4(s32 arg0);
void func_800B4390(void);
void func_800C1EC8(s32 arg0, void *arg1, s32 arg2);
u8 *func_800C1E40(s32 arg0);
u32 func_800BD414(s32 arg0, s32 arg1);
s32 func_800C3688(s32 arg0);
void func_800B3580(void);
s32 func_800B3670(s32 arg0);

extern u8 *D_80122B74;
extern s32 D_8010D020;
extern u8 D_800EF8C0[];
extern u8 D_800F0B48[];
extern u8 D_800F0AE8[];
extern StructB3580 D_80123B08;
extern u8 *D_80123FAC;
extern StructB3580 *D_80123FB0;
extern u16 g_music_track_index;

/**
 * @brief Roll a percentage chance derived from a record's stat 7.
 * @param arg0 Record passed to func_800B2D34.
 * @return 1 when a random byte is below the chance, else 0.
 */
s32 func_800B2FF8(u8 *arg0)
{
    s32 chance;

    chance = func_800B2D34(arg0, 7);
    return (u32) (rand() & 0xFF) < (u32) chance;
}

/**
 * @brief Decide facing between two objects from their X order and the second object's kind.
 * @param arg0 First object.
 * @param arg1 Second object; its kind from func_8008B288 flips the result when in 0x40..0xC0.
 * @return -1 or 0.
 */
s32 func_800B302C(void *arg0, void *arg1)
{
    s32 kind;
    s32 a[4];
    s32 b[4];

    kind = func_8008B288(arg1);
    func_80087F44(arg0, a);
    func_80087F44(arg1, b);
    if (a[0] - b[0] < 0)
    {
        if ((u32)(kind - 0x40) >= 0x81)
        {
            return 0;
        }
        return -1;
    }
    else
    {
        if ((u32)(kind - 0x40) >= 0x81)
        {
            return -1;
        }
        return 0;
    }
}

/**
 * @brief Subtract from a record's counter, clamping at zero; a negative amount is reported to the audio driver.
 * @param arg0 Record.
 * @param arg1 Amount to subtract.
 */
void func_800B30B8(SomeStruct *arg0, s32 arg1)
{
    s32 v0;

    if (arg1 < 0)
    {
        akao_set_song_params(0x8001, 0x7A, arg0->unk14, arg1);
    }
    else
    {
        v0 = arg0->unk4 - arg1;
        if (v0 >= 0)
        {
            arg0->unk4 = v0;
        }
        else
        {
            arg0->unk4 = 0;
        }
    }
}

/**
 * @brief Add to a counter and clamp it to its cap on overflow.
 * @param counter Counter to update.
 * @param delta Amount to add to the counter's value.
 */
void func_800B3114(SaturatingCounter *counter, s32 delta)
{
    u32 cap;
    u32 sum;

    cap = counter->cap;
    sum = counter->value + delta;
    counter->value = sum;
    if (cap < sum)
    {
        counter->value = cap;
    }
}

/**
 * @brief Forward a record's byte at 0x4 to func_80089D44.
 * @param arg0 Record.
 */
void func_800B313C(UnkStruct800B313C *arg0)
{
    func_80089D44(arg0->unk4);
}

/**
 * @brief Clears one indexed record state or all twelve states.
 *
 * Indices zero through eleven clear the corresponding low flag bit and
 * halfword at record offset 0x50. Any other index clears all twelve halfwords
 * and the low sixteen bits of the flag word.
 *
 * @param record Record whose states are cleared.
 * @param index State index, or an out-of-range value to clear all states.
 */
void func_800B3160(RecordB3160 *record, u32 index)
{
    s32 count;
    u8 *cursor;

    if (index < 0xC)
    {
        record->state->flags &= ~(1 << index);
        *(u16 *)((u8 *)record + (index << 1) + 0x50) = 0;
        return;
    }
    count = 0xB;
    record->state->flags &= 0xFFFF0000;
    cursor = (u8 *)record + 0x16;
    do
    {
        *(u16 *)(cursor + 0x50) = 0;
        count--;
        cursor -= 2;
    } while (count >= 0);
}

/**
 * @brief Pick the value for script variable 0xD028 from the encounter chance and gauge.
 * @param arg0 Forwarded to func_800C9ED4 to pick a slot index.
 * @see decomp.me (100%)
 */
void func_800B31CC(s32 arg0)
{
    u32 chance;
    u8 pad[0x20]; /* unreferenced; reserves the original's stack slot */
    u32 count;
    u32 index;

    chance = D_80122B74[0xC06];
    if ((u32)(rand() % 100) < chance)
    {
        func_800BD520(2, 0xD028, 0x64);
    }
    else
    {
        count = D_80122B74[0xC04] >> 4;
        if ((count < 4) || (count >= 8))
        {
            akao_set_song_params(0x74, count, 0, 0);
            func_800BD520(2, 0xD028, 0x63);
        }
        index = func_800C9ED4(arg0);
        if (index >= count)
        {
            index = count - 1;
        }
        func_800BD520(2, 0xD028, (((func_800B2A9C(2)->unk10->unk48 * count) >> 8) * 6) + index);
    }
}

/**
 * @brief Set script variables 0xD030, 0xD038 and 0xD040 from the encounter table or the chance roll.
 * @param arg0 Encounter table row, valid below 0x24.
 * @see decomp.me (100%)
 */
void func_800B32FC(s32 arg0)
{
    s32 chance;
    s32 offset;

    chance = D_80122B74[0xC06];
    if (((rand() * 100) / 0x8000) < chance)
    {
        func_800BD520(2, 0xD030, 0x81);
        func_800BD520(2, 0xD038, 0);
        func_800BD520(2, 0xD040, 0x64);
    }
    else if (arg0 < 0x24)
    {
        offset = arg0 * 4;
        func_800BD520(2, 0xD030, *(D_80122B74 + offset + 0x2A7C));
        func_800BD520(2, 0xD038, *(D_80122B74 + offset + 0x2A7D));
        func_800BD520(2, 0xD040, *(D_80122B74 + offset + 0x2A7E));
    }
    else
    {
        func_800BD520(2, 0xD030, 0x81);
        func_800BD520(2, 0xD038, 0);
        func_800BD520(2, 0xD040, 0x63);
    }
}

/**
 * @brief Advance the 0..0xFF gauge by arg0 scaled 4x, 3x, 2x or 1x by its current quarter, wrapping to 0.
 * @param arg0 Base increment.
 * @see decomp.me (100%)
 */
void func_800B3420(s32 arg0)
{
    RecordB2A9C *rec;
    RecordB2A9CSub *sub;
    u32 value;

    rec = func_800B2A9C(2);
    sub = rec->unk10;
    value = sub->unk48;

    switch (value >> 6)
    {
        case 0:
            sub->unk48 = value + (arg0 * 4);
            break;
        case 1:
            sub->unk48 = value + (arg0 * 3);
            break;
        case 2:
            sub->unk48 = value + (arg0 * 2);
            break;
        case 3:
            sub->unk48 = value + arg0;
            break;
    }

    if (rec->unk10->unk48 >= 0x100)
    {
        rec->unk10->unk48 = 0;
    }
}

/**
 * @brief Start a battle setup: publish script variables 0x4280 and 0x4284, or call func_800B4390 when arg0 is 0.
 * @param arg0 Nonzero starts the setup and is forwarded to func_800B3DF4.
 * @see decomp.me (100%)
 */
void func_800B34D0(s32 arg0)
{
    s32 value;

    if (arg0 != 0)
    {
        func_800B3580();
        value = func_800B37D4();
        if (D_8010D020 != 0)
        {
            func_800BD520(0, 0x4280, 1);
        }
        else
        {
            func_800BD520(0, 0x4280, value);
        }
        value = func_800B3DF4(arg0);
        if (D_8010D020 != 0)
        {
            func_800BD520(0, 0x4284, 1);
        }
        else
        {
            func_800BD520(0, 0x4284, value);
        }
    }
    else
    {
        func_800B4390();
    }
}

/**
 * @brief Reset the D_80123B08 block and fill it from the current track's row and resource 1.
 * @see decomp.me (100%)
 */
void func_800B3580(void)
{
    s32 i;
    u8 *p;

    D_80123FAC = D_800EF8C0;
    D_80123FB0 = &D_80123B08;
    func_800C1EC8(0, &D_80123B08, 0x4A4);
    D_80123FB0->unk18 = 0;
    D_80123FB0->unk0 = func_800B3670(0);

    for (i = 0; i < 8; i++)
    {
        D_80123FB0->unkC[i] = D_800F0B48[FIELD_B74->unk2F4[g_music_track_index][i]];
    }

    p = func_800C1E40(1);
    D_80123FB0->unk4 = p + *(s32 *)(p + 4);
    D_80123FB0->unk8 = p + *(s32 *)(p + 8);
    func_800BD520(0, 0x428C, -1);
}

/**
 * @brief Compute the encounter level from the difficulty table, clamped to the script's bounds and 0x63.
 * @param arg0 Nonzero forces the block-based index; script flag 0x52F0 bit 7 also forces it.
 * @return Level in 0..0x63.
 * @see decomp.me (100%)
 */
s32 func_800B3670(s32 arg0)
{
    s32 flag;
    s32 mode;
    s32 index;
    s32 value;
    u32 lo;
    u32 hi;

    flag = arg0;
    if (func_800BD414(0, 0x52F0) & 0x80)
    {
        flag = 1;
    }
    mode = func_800BD414(0, 0x2938);

    if (flag != 0)
    {
        switch (mode)
        {
            case 1:
                index = FIELD_B74->unk2E5 + 0x14;
                break;
            case 2:
                index = 0x3F;
                break;
            default:
                index = FIELD_B74->unk2E5;
                break;
        }
        index = (index * 3) / 2;
    }
    else
    {
        index = func_800C3688(g_music_track_index);
        switch (mode)
        {
            case 1:
                index += 0x14;
                break;
            case 2:
                index = 0x3F;
                break;
        }
    }

    if (index >= 0x40)
    {
        index = 0x3F;
    }

    value = D_800F0AE8[index];
    lo = func_800BD414(0, 0x52E0);
    hi = func_800BD414(0, 0x52E8);
    if (value < lo)
    {
        value = lo;
    }
    else if (value > hi)
    {
        value = hi;
    }

    if (value >= 0x64)
    {
        value = 0x63;
    }
    return value;
}
