/**
 * @file field_stat_counter_ops.c
 * @brief Game-state flag bits and the saturating per-index counters.
 */

#include "game_audio.h"
#include "common.h"
#include "field_records.h"

/** @brief Largest value a game-state counter can hold. */
#define FIELD_COUNTER_MAX 99

/** @brief Counter indexes at or above this value are rejected with a diagnostic. */
#define FIELD_COUNTER_LIMIT 0xFF

void func_800B2844(s32, void*, s32);
void func_800C2228(s32 index);

extern FieldGameState* D_80122B74;
extern u16 D_800F0E98[];

/**
 * @brief Set one bit of the game-state flag bits.
 * @param bit_index Bit to set; bit n lives in word n / 32.
 * @return Always -1.
 */
s32 func_800C2094(s32 bit_index)
{
    s32 word;
    s32 bit;

    word = bit_index / 32;
    bit = bit_index % 32;
    D_80122B74->flag_bits[word] |= 1 << bit;
    return -1;
}

/**
 * @brief Run the counter's notification entry and return the counter.
 * @param index Counter index, or >= 0xFF to report an invalid index.
 * @return The counter value, or 0 for an invalid index.
 */
u8 func_800C20D8(s32 index)
{
    if (index < FIELD_COUNTER_LIMIT)
    {
        func_800C2228(index);
        return D_80122B74->counters[index];
    }
    record_game_diagnostic(0x8001, 0x70, index, 0);
    return 0;
}

/**
 * @brief Increment a counter, saturating at FIELD_COUNTER_MAX, then run its notification entry.
 * @param index Counter index, or >= 0xFF to report an invalid index.
 */
void func_800C2138(s32 index)
{
    if (index < FIELD_COUNTER_LIMIT)
    {
        D_80122B74->counters[index]++;
        if (D_80122B74->counters[index] > FIELD_COUNTER_MAX)
        {
            D_80122B74->counters[index] = FIELD_COUNTER_MAX;
        }
        func_800C2228(index);
    }
    else
    {
        record_game_diagnostic(0x8001, 0x71, index, 0);
    }
}

/**
 * @brief Decrement a nonzero counter, then run its notification entry.
 * @param index Counter index, or >= 0xFF to report an invalid index.
 */
void func_800C21C0(s32 index)
{
    u8 value;

    if (index < FIELD_COUNTER_LIMIT)
    {
        value = D_80122B74->counters[index];
        if (value != 0)
        {
            D_80122B74->counters[index] = value - 1;
        }
        func_800C2228(index);
    }
    else
    {
        record_game_diagnostic(0x8001, 0x72, index, 0);
    }
}

/**
 * @brief Run the D_800F0E98 script entry for a counter.
 * @param index Counter index; selects a self-relative offset in D_800F0E98.
 */
void func_800C2228(s32 index)
{
    func_800B2844(0, (u8*)D_800F0E98 + D_800F0E98[index], 0x15);
}
