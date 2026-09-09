#include "common.h"

#define GROW_U8(p, o) (*(u8 *)((u8 *)(p) + (o)))
#define GROW_U16(p, o) (*(u16 *)((u8 *)(p) + (o)))
#define GROW_U32(p, o) (*(u32 *)((u8 *)(p) + (o)))

extern u8 *D_80122B74;
extern u8 *func_800B2A9C(s32);
extern void *func_80087F0C(s32);
void func_800C10F0(s32);
void func_800C1154(u32);
void func_800C11F0(s32, s32);

typedef struct
{
    s8 pad[0x2C];
    s32 unk2C;
} UnkStruct80122B74;



void func_800C0E18(s32 type, s32 amount)
{
    if (type < 3)
    {
        ((UnkStruct80122B74 *)D_80122B74)->unk2C += amount;
        if ((u32)((UnkStruct80122B74 *)D_80122B74)->unk2C > 0x989680)
        {
            ((UnkStruct80122B74 *)D_80122B74)->unk2C = 0x989680;
        }
    }
}

/** @brief Distributes experience and advances eligible active records.
 * @note Initial nonmatching C; retains the original overflow-store target.
 */
void func_800C0E54(s32 arg0, s32 arg1)
{
    s32 temp_lo;
    s32 var_s0;
    s32 var_s0_2;
    s32 var_s1;
    s32 var_s1_2;
    s32 var_s2;
    s32 var_v0;
    u32 temp_v0_2;
    u32 temp_v0_3;
    u32 temp_v1_2;
    u32 temp_v1_4;
    u8 temp_v1;
    u8 *temp_a1;
    u8 *temp_a1_2;
    u8 *temp_v0;
    u8 *temp_v1_3;

    if ((arg0 < 3) && (temp_v0 = func_800B2A9C(arg0), (temp_v0 != NULL)))

    {
        temp_v1 = GROW_U8(temp_v0, 0x4);
        switch (temp_v1)
        {                          /* irregular */
        case 0:
            func_800C1154(arg1);
            break;
        case 1:
            func_800C10F0(arg1);
            break;
        }
        var_s1 = 0;
        if (GROW_U16(temp_v0, 0xA) & 4)
        {
            var_s0 = 0;
            var_s2 = 0;
            do
            {
                if ((GROW_U8(D_80122B74 + var_s2, 0x5F0) != 0) && (GROW_U32(func_80087F0C(var_s1), 4) != 0))
                {
                    var_s0 += 1;
                }
                var_s1 += 1;
                var_s2 += 0x250;
            } while (var_s1 < 3);
            if (var_s0 <= 0)
            {
                var_s0 = 1;
            }
            temp_lo = arg1 / var_s0;
            var_v0 = temp_lo;
            if (temp_lo == 0)
            {
                var_v0 = 1;
            }
            var_s1_2 = 0;
            var_s0_2 = 0;
            do
            {
                if ((GROW_U8(D_80122B74 + var_s0_2, 0x5F0) != 0) && (GROW_U32(func_80087F0C(var_s1_2), 4) != 0))
                {
                    if (var_s1_2 == 1)
                    {
                        func_800C10F0(var_v0);
                    }
                    temp_a1 = D_80122B74 + var_s0_2;
                    temp_v0_2 = GROW_U32(temp_a1, 0x610);
                    temp_v1_2 = (temp_v0_2 & 0xFF) | (((temp_v0_2 >> 8) + var_v0) << 8);
                    GROW_U32(temp_a1, 0x610) = temp_v1_2;
                    if ((s32) (temp_v1_2 >> 8) > 0x98967F)
                    {
                        temp_v1_3 = D_80122B74 + (arg0 * 0x250);
                        GROW_U32(temp_v1_3, 0x610) = (s32) (GROW_U8(temp_v1_3, 0x610) | 0x98967F00);
                    }
                    func_800C11F0(var_s1_2, 1);
                }
                var_s1_2 += 1;
                var_s0_2 += 0x250;
            } while (var_s1_2 < 3);
            return;
        }
        temp_a1_2 = D_80122B74 + (arg0 * 0x250);
        temp_v0_3 = GROW_U32(temp_a1_2, 0x610);
        temp_v1_4 = (temp_v0_3 & 0xFF) | (((temp_v0_3 >> 8) + arg1) << 8);
        GROW_U32(temp_a1_2, 0x610) = temp_v1_4;
        if ((s32) (temp_v1_4 >> 8) > 0x98967F)
        {
            GROW_U32(temp_a1_2, 0x610) = (u32) ((temp_v1_4 & 0xFF) | 0x98967F00);
        }
        func_800C11F0(arg0, 1);
    }
}


extern u8 *D_80122B74;

/**
 * @brief Adds to a party record's counter field and clamps it to 0x98967F.
 *
 * Selects the record at @c D_80122B74 offset (base[0x859] + 0x68) * 4 + 0xE4
 * (a 32-bit counter), adds @p arg0 to it, then re-reads the same record and
 * clamps the counter to a maximum of 0x98967F (9,999,999).
 *
 * @param arg0 Amount to add to the counter.
 */
void func_800C10F0(s32 arg0)
{
    u8 *base;
    s32 *rec;
    s32 idx;
    s32 idx2;

    base = D_80122B74;

    idx = base[0x859] + 0x68;
    rec = (s32 *)(base + idx * 4 + 0xE4);
    *rec += arg0;

    idx2 = base[0x859] + 0x68;
    rec = (s32 *)(base + idx2 * 4 + 0xE4);
    if ((u32)*rec > 0x98967F)
    {
        *rec = 0x98967F;
    }
}


extern u8 *D_80122B74;

void func_800C1154(u32 arg0)
{
    u32 temp_v0;
    u32 temp_v1;
    u32 var_a2;
    u8 *base;

    arg0 = arg0 >> 3;
    var_a2 = 0;
    base = D_80122B74;
    do
    {
        if ((base[(var_a2 * 0x60) + 0x2EF4] != 0) &&
            (((u32) *(u32 *) (base + (var_a2 * 0x60) + 0x2F38) >> 0x1E) & 1))
        {
            temp_v0 = *(u32 *) (base + (var_a2 * 0x60) + 0x2F0C);
            temp_v1 = (temp_v0 & 0xFF) | (((temp_v0 >> 8) + arg0) << 8);
            *(u32 *) (base + (var_a2 * 0x60) + 0x2F0C) = temp_v1;
            if ((s32) (temp_v1 >> 8) > 0x98967F)
            {
                *(u32 *) (base + (var_a2 * 0x60) + 0x2F0C) =
                    (temp_v1 & 0xFF) | 0x98967F00;
            }
        }
        var_a2 += 1;
    } while (var_a2 < 5U);
}


s32 func_800C14A4(s32 arg0, s32 arg1);

void func_800C11F0(s32 arg0, s32 arg1)
{
    do
    {
    } while (func_800C14A4(arg0, arg1) != 0);
}

void akao_set_song_params(s32, s32, s32, s32); /* extern */
s32 func_800C19D0(s32, s32, s32);              /* extern */
/** Packed state accessed at byte, halfword, and word widths by the game. */
typedef union PackedWord
{
    u32 word;
    u8 bytes[4];
    u16 halves[2];
} PackedWord;
/** One 0x60-byte character record containing level and stat-growth state. */
typedef struct Record
{
    u8 active;
    u8 pad01[0x17];
    PackedWord progress;
    u16 hp, mp;
    u16 values[4];
    u16 stats[8];
    u8 pad38[12];
    PackedWord status;
    u8 pad48[4];
    u8 growth[8];
    u8 value_growth[4];
    PackedWord extra;
    u8 pad5c[4];
} Record;
extern u8 *D_80122B74;

/**
 * Apply every pending level increase for an active character slot.
 *
 * Updates packed stat growth, fractional carries, and derived values, then clears
 * the three cached status bytes. The level is capped at 99.
 * @param slot Character slot; values at least five take the original error-sound path.
 * @note Partial assembly match; retained probes document the remaining scheduling differences.
 */
void func_800C1230(s32 slot)
{
    s32 previous_level;
    s32 scaled_level;
    s32 mask;
    s32 threshold;
    s32 updated_extra;
    s32 growth_nibble;
    s32 growth_high;
    s32 stat_index;
    s32 index;
    s32 pending;
    u16 stat;
    u32 extra_word;
    u32 experience;
    u32 growth_value;
    u32 carry;
    u32 base_stat;
    s32 level;
    u32 next_level;
    u8 *value_growth_cursor;
    u8 *growth_cursor;
    u8 *record;
    u8 *stat_cursor;
    u8 *value_cursor;
    u8 *status_cursor;

    if (slot >= 5)
    {
        akao_set_song_params(0x8001, 0x1F3, slot, 0);
        return;
    }
    record = D_80122B74 + ((slot * 0x60) + 0x2EF4);
    pending = -1;
    if (((Record *)record)->active != 0)
    {
        mask = -0xF1;
        do
        {
            level = ((Record *)record)->progress.bytes[0];
            previous_level = level - 1;
            scaled_level = level * 5;
            threshold = previous_level * (scaled_level << 2) + (scaled_level << 1);
            experience = ((Record *)record)->progress.word >> 8;
            if ((experience != 0) && (experience >= (u32)threshold))
            {
                next_level = level + 1;
                ((Record *)record)->progress.bytes[0] = next_level;
                stat_index = 0;
                if ((u32)(next_level & 0xFF) >= 0x64U)
                {
                    ((Record *)record)->progress.bytes[0] = 0x63U;
                    goto block_17;
                }
                stat_cursor = record;
            stat_loop:
            {
                growth_cursor = record + stat_index;
                stat = ((Record *)stat_cursor)->stats[0];
                stat =
                    ((((u8)((Record *)growth_cursor)->growth[0] >> 4) + (stat & 0x1FF)) & 0x1FF) |
                    (stat & 0xFE00);
                ((Record *)stat_cursor)->stats[0] = stat;
                if ((u32)(stat & 0x1FF) >= 0x18DU)
                {
                    ((Record *)stat_cursor)->stats[0] = (u16)((stat & 0xFE00) | 0x18C);
                }
                stat_index += 1;
                base_stat = ((Record *)stat_cursor)->stats[0] & 0x1FF;
                ((Record *)stat_cursor)->stats[0] = (u16)(base_stat | ((base_stat >> 2) << 9));
                growth_nibble = ((Record *)growth_cursor)->growth[0] & 0xF;
                ((Record *)growth_cursor)->growth[0] = (u8)(growth_nibble | (growth_nibble * 0x10));
                stat_cursor += 2;
            }
                if (stat_index < 8)
                {
                    goto stat_loop;
                }
                index = 0;
                value_cursor = record;
            value_loop:
            {
                value_growth_cursor = record + index;
                index += 1;
                ((Record *)value_cursor)->values[0] =
                    (u16)(((Record *)value_cursor)->values[0] +
                          ((u8)((Record *)value_growth_cursor)->value_growth[0] >> 7));
                growth_value = ((Record *)value_growth_cursor)->value_growth[0] & 0x7F;
                growth_high = growth_value >> 4;
                growth_value &= 15;
                ((Record *)value_growth_cursor)->value_growth[0] =
                    growth_value | ((growth_high + growth_value) << 4);
                value_cursor += 2;
            }
                if (index < 4)
                {
                    goto value_loop;
                }
                carry = ((Record *)record)->extra.bytes[0] >> 7;
                extra_word = (mask | 0x70) & (s32)((Record *)record)->extra.word;
                ((Record *)record)->extra.word = extra_word;
                ((Record *)record)->mp = (u16)(((Record *)record)->mp + carry);
                updated_extra =
                    (extra_word & mask) |
                    (((((extra_word >> 4) & 0xF) + (((Record *)record)->extra.bytes[0] & 0xF)) &
                      0xF) *
                     0x10);
                ((Record *)record)->extra.word = updated_extra;
                ((Record *)record)->hp = func_800C19D0(
                    ((Record *)record)->hp, (u32)(((Record *)record)->stats[4] & 0x1FF) >> 2, 3);
                index = 2;
                status_cursor = record + index;
                ((Record *)record)->extra.halves[1] =
                    (u16)(((Record *)record)->extra.halves[1] + ((Record *)record)->extra.bytes[1]);
                ((Record *)record)->status.word =
                    (s32)(((Record *)record)->status.word & 0xF8FFFFFF);
                do
                {
                    ((Record *)status_cursor)->status.bytes[0] = 0xFF;
                    index -= 1;
                    status_cursor -= 1;
                } while (index >= 0);
            }
            else
            {
            block_17:
                pending = 0;
            }
        } while (pending != 0);
    }
}


extern u8 *D_80122B74;
void func_800C1658(u8 *record, u8 *base, u8 *current, s32 offset);
void func_800C15AC(u8 *record, u8 *base, u8 *current, s32 offset);
void func_800B7C58(s32 index);
s32 func_8008B500(s32 index, s32 value);

/**
 * @brief Advance the indexed field record when its packed progression reaches the next threshold.
 * @param index Index of the 0x250-byte field record to update.
 * @param notify Nonzero to emit update code 0x23 after advancing the record.
 * @return -1 when the record advances, otherwise 0.
 */
s32 func_800C14A4(s32 index, s32 notify)
{
    u8 *initial_base;
    u8 *record;
    u8 *current;
    u_long level_or_base;
    u32 packed;
    s32 previous_level;
    s32 scaled_level;
    s32 call_offset;

    initial_base = D_80122B74;
    record = initial_base + index * 0x250;
    level_or_base = record[0x610];
    packed = *(u32 *)(record + 0x610);
    previous_level = level_or_base - 1;
    scaled_level = (level_or_base << 2) + level_or_base;
    if ((s32)(packed >> 8) >= previous_level * (scaled_level << 2) + (scaled_level << 1))
    {
        record[0x610] = (u8)(level_or_base + 1);
        level_or_base = (u_long)D_80122B74;
        current = (u8 *)level_or_base + index * 0x250;
        if (current[0x610] >= 0x64)
        {
            current[0x610] = 0x63;
            return 0;
        }

        call_offset = index * 0x250 + 0x5F0;
        if ((current[0x608] & 0x7F) == 3)
        {
            func_800C1658((u8 *)level_or_base + call_offset, (u8 *)level_or_base, record, index * 0x250);
        }
        else
        {
            func_800C15AC((u8 *)level_or_base + call_offset, (u8 *)level_or_base, record, index * 0x250);
        }

        func_800B7C58(index);
        if (notify != 0)
        {
            func_8008B500(index, 0x23);
        }
        return -1;
    }
    return 0;
}


extern u8 D_800F18CC[];
extern s32 func_800C19D0(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Advance eight packed 9-bit counters by a per-slot nibble delta, clamped.
 *
 * Looks up a 32-bit table word in @c D_800F18CC indexed by
 * @c ((*(u32 *)(arg0 + 0x64) >> 8) & 0xFC), treating its low 32 bits as eight
 * 4-bit deltas. For each of the eight u16 values at @c arg0+0x30, adds the next
 * nibble to the low 9 bits (mod 0x200), preserving the upper 7 bits, and clamps
 * the result to 0x18C when it reaches or exceeds 0x18D. Finally updates the u16
 * at @c arg0+0x24 from the fifth counter (at @c arg0+0x38) via @c func_800C19D0.
 *
 * @param arg0 Pointer to the record holding the packed counters and control fields.
 */
void func_800C15AC(u8 *arg0, u8 *unused_base, u8 *unused_current, s32 unused_offset)
{
    s32 i;
    u32 bits;
    u16 x;
    u32 low;
    u8 *table;

    i = 0;
    table = D_800F18CC;
    bits = *(u32 *)(table + ((*(u32 *)(arg0 + 0x64) >> 8) & 0xFC));
    do
    {
        x = *(u16 *)(arg0 + 0x30 + i * 2);
        low = (x & 0x1FF) + (bits & 0xF);
        low &= 0x1FF;
        x = (x & 0xFE00) | low;
        *(u16 *)(arg0 + 0x30 + i * 2) = x;
        if ((u32)(x & 0x1FF) >= 0x18D)
        {
            *(u16 *)(arg0 + 0x30 + i * 2) = (x & 0xFE00) | 0x18C;
        }
        bits >>= 4;
        i++;
    } while (i < 8);
    *(u16 *)(arg0 + 0x24) = func_800C19D0(*(u16 *)(arg0 + 0x24), ((u32)(*(u16 *)(arg0 + 0x38) & 0x1FF)) >> 2, 3);
}

/** @brief Applies saved-slot growth to an active record and saves derived values.
 * @note Initial nonmatching C; extra arguments preserve the existing caller ABI.
 */
void func_800C1658(u8 *arg0, u8 *unused_base, u8 *unused_current, s32 unused_offset)
{
    s32 temp_a0_5;
    s32 temp_v0_2;
    s32 temp_v0_6;
    s32 temp_v0_7;
    s32 var_a3;
    s32 var_a3_2;
    s32 var_a3_3;
    u16 temp_a0;
    u16 temp_a0_2;
    u16 temp_a0_4;
    u16 temp_a1_2;
    u32 temp_a2;
    u32 temp_v0_4;
    u8 temp_v0;
    u8 temp_v0_5;
    u8 *temp_a0_3;
    u8 *temp_a0_6;
    u8 *temp_a1;
    u8 *temp_a2_2;
    u8 *temp_v0_3;
    u8 *temp_v1;
    u8 *var_a1;
    u8 *var_t0;

    temp_a2 = GROW_U32(D_80122B74, 0x2EF0);
    if (temp_a2 >= 5U)
    {
        akao_set_song_params(0x8001, 0x79, temp_a2, 0);
    }
    var_a3 = 0;
    var_a1 = arg0;
    do
    {
        temp_a0 = GROW_U16(var_a1, 0x30);
        temp_a0_2 = (temp_a0 & 0xFE00) | ((((u8) GROW_U8((D_80122B74 + (var_a3 + (GROW_U32(D_80122B74, 0x2EF0) * 0x60))), 0x2F40) >> 4) + (temp_a0 & 0x1FF)) & 0x1FF);
        GROW_U16(var_a1, 0x30) = temp_a0_2;
        if ((u32) (temp_a0_2 & 0x1FF) >= 0x18DU)
        {
            GROW_U16(var_a1, 0x30) = (u16) ((temp_a0_2 & 0xFE00) | 0x18C);
        }
        temp_a0_3 = D_80122B74 + (var_a3 + (GROW_U32(D_80122B74, 0x2EF0) * 0x60));
        temp_v0 = GROW_U8(temp_a0_3, 0x2F40);
        var_a3 += 1;
        temp_v0_2 = temp_v0 & 0xF;
        GROW_U8(temp_a0_3, 0x2F40) = (s8) (temp_v0_2 | (temp_v0_2 * 0x10));
        var_a1 += 2;
    } while (var_a3 < 8);
    temp_a0_4 = GROW_U16(arg0, 0x26) + (GROW_U8((D_80122B74 + (GROW_U32(D_80122B74, 0x2EF0) * 0x60)), 0x2F4C) >> 7);
    GROW_U16(arg0, 0x26) = temp_a0_4;
    var_a3_2 = 0;
    GROW_U16((D_80122B74 + (GROW_U32(D_80122B74, 0x2EF0) * 0x60)), 0x2F12) = temp_a0_4;
    GROW_U16(arg0, 0x74) = (u16) GROW_U16(arg0, 0x26);
    temp_v0_3 = D_80122B74 + (GROW_U32(D_80122B74, 0x2EF0) * 0x60);
    GROW_U32(temp_v0_3, 0x2F4C) = (s32) ((-0xF1 | 0x70) & GROW_U32(temp_v0_3, 0x2F4C));
    var_t0 = arg0;
    temp_a1 = D_80122B74 + (GROW_U32(D_80122B74, 0x2EF0) * 0x60);
    temp_v0_4 = GROW_U32(temp_a1, 0x2F4C);
    GROW_U32(temp_a1, 0x2F4C) = (u32) ((temp_v0_4 & ~0xF0) | (((((temp_v0_4 >> 4) & 0xF) + (GROW_U8(temp_a1, 0x2F4C) & 0xF)) & 0xF) * 0x10));
    do
    {
        temp_a1_2 = GROW_U16(var_t0, 0x28) + ((u8) GROW_U8((D_80122B74 + (var_a3_2 + (GROW_U32(D_80122B74, 0x2EF0) * 0x60))), 0x2F48) >> 7);
        GROW_U16(var_t0, 0x28) = temp_a1_2;
        temp_a0_5 = var_a3_2 * 2;
        GROW_U16((D_80122B74 + (temp_a0_5 + (GROW_U32(D_80122B74, 0x2EF0) * 0x60))), 0x2F14) = temp_a1_2;
        GROW_U16((arg0 + temp_a0_5), 0xB4) = (u16) GROW_U16(var_t0, 0x28);
        temp_a2_2 = D_80122B74 + (var_a3_2 + (GROW_U32(D_80122B74, 0x2EF0) * 0x60));
        GROW_U8(temp_a2_2, 0x2F48) = (u8) (GROW_U8(temp_a2_2, 0x2F48) & 0x7F);
        temp_a0_6 = D_80122B74 + (var_a3_2 + (GROW_U32(D_80122B74, 0x2EF0) * 0x60));
        temp_v0_5 = GROW_U8(temp_a0_6, 0x2F48);
        var_a3_2 += 1;
        temp_v0_6 = temp_v0_5 & 0xF;
        GROW_U8(temp_a0_6, 0x2F48) = (s8) (temp_v0_6 | (((temp_v0_5 >> 4) + temp_v0_6) * 0x10));
        var_t0 += 2;
    } while (var_a3_2 < 4);
    GROW_U16(arg0, 0x24) = func_800C19D0(GROW_U16(arg0, 0x24), (u32) (GROW_U16(arg0, 0x38) & 0x1FF) >> 2, 3);
    GROW_U8((D_80122B74 + (GROW_U32(D_80122B74, 0x2EF0) * 0x60)), 0x2F0C) = (u8) GROW_U8(arg0, 0x20);
    GROW_U16((D_80122B74 + (GROW_U32(D_80122B74, 0x2EF0) * 0x60)), 0x2F10) = (u16) GROW_U16(arg0, 0x24);
    var_a3_3 = 0;
    temp_v1 = D_80122B74 + (GROW_U32(D_80122B74, 0x2EF0) * 0x60);
    GROW_U32(temp_v1, 0x2F38) = (s32) (GROW_U32(temp_v1, 0x2F38) & 0xF8FFFFFF);
    do
    {
        temp_v0_7 = var_a3_3 + (GROW_U32(D_80122B74, 0x2EF0) * 0x60);
        var_a3_3 += 1;
        GROW_U8((D_80122B74 + temp_v0_7), 0x2F38) = 0xFF;
    } while (var_a3_3 < 3);
}


s32 func_800C19D0(s32 arg0, s32 arg1, s32 arg2)
{
    s32 result;

    if ((u32) arg1 >= 6 && (arg2 & 1))
    {
        result = arg0 + ((u32) (arg1 - 5) >> 1) + 5;
    }
    else
    {
        result = arg0 + arg1;
    }
    if ((arg2 & 2) && (u32) result >= 0x3E8)
    {
        result = 0x3E7;
    }
    return result;
}
