/**
 * @file field_record_growth_ops.c
 * @brief Money, experience and level-up growth for party characters and
 *        stored companion records.
 */

#include "game_audio.h"
#include "common.h"
#include "field_records.h"

/** @brief Money saturates at this amount. */
#define FIELD_MONEY_MAX 10000000

/** @brief Largest experience value of a progress word. */
#define FIELD_EXPERIENCE_MAX 9999999

/** @brief Progress word with the maximum experience and a zero level byte. */
#define FIELD_EXPERIENCE_MAX_WORD ((u32)FIELD_EXPERIENCE_MAX << 8)

/** @brief Highest level a record can reach. */
#define FIELD_LEVEL_MAX 99

/** @brief Largest counter value in FieldGameState::words. */
#define FIELD_COUNTER_MAX 9999999

/** @brief First FieldGameState::words entry of the per-character counters. */
#define FIELD_CHARACTER_COUNTER_BASE 0x68

/** @brief Largest base stat value (bits 0-8 of a stat halfword). */
#define FIELD_STAT_MAX 0x18C

/** @brief Character type (low seven bits of FieldCharacterRecord info byte 0) of a companion. */
#define FIELD_CHARACTER_TYPE_COMPANION 3

/** @brief Value of an empty pending-effect slot in FieldRegionRecord::status. */
#define FIELD_NO_EFFECT 0xFF

/** @brief Signal passed to func_8008B500 after a level-up. */
#define FIELD_SIGNAL_LEVEL_UP 0x23

/**
 * @brief Experience needed to advance from @p level.
 * @note Equal to 10 * level * (2 * level - 1), written the way the original
 *       code computes it.
 */
#define FIELD_LEVEL_THRESHOLD(level) (((level) - 1) * (((level) * 5) << 2) + (((level) * 5) << 1))

/**
 * @brief View of the game state shifted by @p bytes bytes.
 * @note characters[0] or regions[0] of the view is the record @p bytes past
 *       the real first one. The original code addresses a record's fields
 *       from the shifted state base like this (offsets such as 0x610 relative
 *       to state + index * 0x250); indexing the array directly does not
 *       produce the same code.
 */
#define FIELD_STATE_AT(state, bytes) ((FieldGameState *)((u8 *)(state) + (bytes)))

/** @brief View of a companion record shifted by @p bytes bytes (see FIELD_STATE_AT). */
#define FIELD_REGION_AT(region, bytes) ((FieldRegionRecord *)((u8 *)(region) + (bytes)))

/**
 * @brief Growth byte: low nibble the per-level rate, high nibble an
 *        accumulator whose bit 3 (bit 7 of the byte) carries into a total.
 */
typedef struct FieldGrowthByte
{
    u8 rate : 4;
    u8 accumulator : 4;
} FieldGrowthByte;

/**
 * @brief FieldRegionRecord::extra_growth as a growth byte inside a word.
 * @note The rate is read as a byte and the accumulator is written as part of
 *       the word, which is what the mixed field types reproduce.
 */
typedef struct FieldGrowthWord
{
    u8 rate : 4;
    unsigned accumulator : 4;
    unsigned unk8 : 24;
} FieldGrowthWord;

/** @brief Growth-byte view of a u8 growth field. */
#define GROWTH_BYTE(field) (*(FieldGrowthByte *)&(field))

/** @brief Growth-word view of FieldRegionRecord::extra_growth. */
#define GROWTH_WORD(field) (*(FieldGrowthWord *)&(field))

/** @brief Byte access at offset @p o from @p p (func_800C1658 only). */
#define GROW_U8(p, o) (*(u8 *)((u8 *)(p) + (o)))

/** @brief Halfword access at offset @p o from @p p (func_800C1658 only). */
#define GROW_U16(p, o) (*(u16 *)((u8 *)(p) + (o)))

/** @brief Word access at offset @p o from @p p (func_800C1658 only). */
#define GROW_U32(p, o) (*(u32 *)((u8 *)(p) + (o)))

/** @brief Game state as a byte pointer (func_800C1658 only). */
#define FIELD_STATE_BYTES ((u8 *)D_80122B74)

extern FieldGameState *D_80122B74;

/** @brief Per item type: eight four-bit stat increases applied on a level-up. */
extern u32 D_800F18CC[];

FieldStatusState *func_80087F0C(s32 actor_id);
void func_800C10F0(s32 amount);
void func_800C1154(u32 amount);
void func_800C11F0(s32 index, s32 notify);
s32 func_800C14A4(s32 index, s32 notify);
s32 func_800C19D0(s32 value, s32 increase, s32 flags);
void func_800C1658(u8 *character, FieldGameState *state, FieldGameState *view, s32 offset);
void func_800C15AC(FieldCharacterRecord *character, FieldGameState *state, FieldGameState *view, s32 offset);
void func_800B7C58(s32 index);
s32 func_8008B500(s32 index, s32 value);

/**
 * @brief Add money, saturating at FIELD_MONEY_MAX.
 * @param recipient Reward recipient; only recipients below 3 are credited.
 * @param amount Money to add.
 */
void func_800C0E18(s32 recipient, s32 amount)
{
    if (recipient < 3)
    {
        D_80122B74->money += amount;
        if (D_80122B74->money > FIELD_MONEY_MAX)
        {
            D_80122B74->money = FIELD_MONEY_MAX;
        }
    }
}

/**
 * @brief Award experience, optionally shared among the living party members.
 * @param record_index Status record id of the recipient (0 to 2).
 * @param amount Experience to award.
 * @note When the recipient's status flags have bit 2 set, the amount is split
 *       evenly (at least 1) among the party members that are present and
 *       alive. On overflow the shared path clamps the recipient's record, not
 *       the member that overflowed.
 */
void func_800C0E54(s32 record_index, s32 amount)
{
    s32 eligible_count;
    s32 index;
    u32 packed_value;
    u32 updated_value;
    FieldStatusRecord *entry;
    u32 distributed_value;
    u32 clamped_value;
    FieldGameState *record_view;
    FieldGameState *target_view;

    if ((record_index < 3) && (entry = func_800B2A9C(record_index), (entry != NULL)))
    {
        switch (entry->meta.bytes.id)
        {
        case 0:
            func_800C1154(amount);
            break;
        case 1:
            func_800C10F0(amount);
            break;
        }
        index = 0;
        if (entry->status_flags & 4)
        {
            eligible_count = 0;
            for (; index < FIELD_PARTY_SIZE; index++)
            {
                if ((D_80122B74->characters[index].name[0] != 0) && (func_80087F0C(index)->current != 0))
                {
                    eligible_count += 1;
                }
            }
            if (eligible_count <= 0)
            {
                eligible_count = 1;
            }
            amount /= eligible_count;
            amount = amount == 0 ? 1 : amount;
            for (index = 0; index < FIELD_PARTY_SIZE; index++)
            {
                if ((D_80122B74->characters[index].name[0] != 0) && (func_80087F0C(index)->current != 0))
                {
                    if (index == 1)
                    {
                        func_800C10F0(amount);
                    }
                    record_view = FIELD_STATE_AT(D_80122B74, index * sizeof(FieldCharacterRecord));
                    packed_value = record_view->characters[0].progress.word;
                    distributed_value = (packed_value & 0xFF) | (((packed_value >> 8) + amount) << 8);
                    record_view->characters[0].progress.word = distributed_value;
                    if ((s32)(distributed_value >> 8) > FIELD_EXPERIENCE_MAX)
                    {
                        target_view = FIELD_STATE_AT(D_80122B74, record_index * sizeof(FieldCharacterRecord));
                        target_view->characters[0].progress.word = target_view->characters[0].progress.level | FIELD_EXPERIENCE_MAX_WORD;
                    }
                    func_800C11F0(index, 1);
                }
            }
            return;
        }
        record_view = FIELD_STATE_AT(D_80122B74, record_index * sizeof(FieldCharacterRecord));
        packed_value = record_view->characters[0].progress.word;
        updated_value = (packed_value & 0xFF) | (((packed_value >> 8) + amount) << 8);
        record_view->characters[0].progress.word = updated_value;
        if ((s32)(updated_value >> 8) > FIELD_EXPERIENCE_MAX)
        {
            clamped_value = updated_value & 0xFF;
            clamped_value |= FIELD_EXPERIENCE_MAX_WORD;
            record_view->characters[0].progress.word = clamped_value;
        }
        func_800C11F0(record_index, 1);
    }
}

/**
 * @brief Add to the counter word selected by party member 1, saturating at FIELD_COUNTER_MAX.
 * @param amount Amount to add.
 * @note The counter is words[FIELD_CHARACTER_COUNTER_BASE + byte 1 of characters[1].info].
 */
void func_800C10F0(s32 amount)
{
    FieldGameState *state;

    state = D_80122B74;
    state->words[state->characters[1].info.bytes[1] + FIELD_CHARACTER_COUNTER_BASE] += amount;
    if ((u32)state->words[state->characters[1].info.bytes[1] + FIELD_CHARACTER_COUNTER_BASE] > FIELD_COUNTER_MAX)
    {
        state->words[state->characters[1].info.bytes[1] + FIELD_CHARACTER_COUNTER_BASE] = FIELD_COUNTER_MAX;
    }
}

/**
 * @brief Give an eighth of @p amount as experience to every stored companion that gains experience.
 * @param amount Experience awarded to the party.
 */
void func_800C1154(u32 amount)
{
    u32 region_index;
    u32 packed_value;
    u32 updated_value;
    FieldGameState *state;

    amount >>= 3;
    state = D_80122B74;
    for (region_index = 0; region_index < FIELD_REGION_COUNT; region_index++)
    {
        if ((state->regions[region_index].name[0] != 0) && ((state->regions[region_index].status.word >> 30) & 1))
        {
            packed_value = state->regions[region_index].progress.word;
            updated_value = (packed_value & 0xFF) | (((packed_value >> 8) + amount) << 8);
            state->regions[region_index].progress.word = updated_value;
            if ((s32)(updated_value >> 8) > FIELD_EXPERIENCE_MAX)
            {
                state->regions[region_index].progress.word = (updated_value & 0xFF) | FIELD_EXPERIENCE_MAX_WORD;
            }
        }
    }
}

/**
 * @brief Apply every level-up a party member's experience allows.
 * @param index Party member index.
 * @param notify Nonzero to signal each level-up (see func_800C14A4).
 */
void func_800C11F0(s32 index, s32 notify)
{
    while (func_800C14A4(index, notify) != 0)
    {
    }
}

/**
 * @brief Apply every level-up a stored companion record's experience allows.
 * @param slot Companion record index (below FIELD_REGION_COUNT).
 * @note Each level adds the stat growth nibbles, rebuilds the effective stat
 *       bits, carries the total accumulators, recomputes hp and clears the
 *       pending effects.
 */
void func_800C1230(s32 slot)
{
    s32 level;
    s32 i;
    s32 pending;
    u16 stat;
    u32 low;
    u32 base_stat;
    s32 rate;
    u32 carry;
    u32 threshold;
    u32 next_level;
    u32 experience;
    FieldRegionRecord *record;
    FieldRegionRecord *effect_view;
    u32 empty;

    if (slot >= FIELD_REGION_COUNT)
    {
        record_game_diagnostic(0x8001, 0x1F3, slot, 0);
        return;
    }
    record = &D_80122B74->regions[slot];
    pending = -1;
    if (record->name[0] != 0)
    {
        do
        {
            level = record->progress.level;
            threshold = FIELD_LEVEL_THRESHOLD(level);
            experience = record->progress.word >> 8;
            if ((experience != 0) && (experience >= threshold))
            {
                next_level = level + 1;
                record->progress.level = next_level;
                if ((next_level & 0xFF) > FIELD_LEVEL_MAX)
                {
                    record->progress.level = FIELD_LEVEL_MAX;
                    pending = 0;
                    continue;
                }
                for (i = 0; i < FIELD_CHARACTER_STAT_COUNT; i++)
                {
                    stat = record->stats[i];
                    low = ((u8)record->stat_growth[i] >> 4) + (stat & 0x1FF);
                    low &= 0x1FF;
                    stat = (stat & 0xFE00) | low;
                    record->stats[i] = stat;
                    if ((stat & 0x1FF) > FIELD_STAT_MAX)
                    {
                        record->stats[i] = (stat & 0xFE00) | FIELD_STAT_MAX;
                    }
                    base_stat = record->stats[i] & 0x1FF;
                    record->stats[i] = base_stat | ((base_stat >> 2) << 9);
                    rate = record->stat_growth[i] & 0xF;
                    record->stat_growth[i] = rate | (rate * 0x10);
                }
                for (i = 0; i < 4; i++)
                {
                    record->equipment_totals[i] += (u8)record->total_growth[i] >> 7;
                    GROWTH_BYTE(record->total_growth[i]).accumulator &= 7;
                    GROWTH_BYTE(record->total_growth[i]).accumulator += GROWTH_BYTE(record->total_growth[i]).rate;
                }
                /* Two statements: a single `>> 7` expression swaps two scheduled insns. */
                carry = record->extra_growth.bytes[0];
                carry >>= 7;
                GROWTH_WORD(record->extra_growth).accumulator &= 7;
                GROWTH_WORD(record->extra_growth).accumulator += GROWTH_WORD(record->extra_growth).rate;
                record->unk1E += carry;
                record->hp = func_800C19D0(record->hp, (u32)(record->stats[4] & 0x1FF) >> 2, 3);
                record->extra_growth.halves[1] += record->extra_growth.bytes[1];
                record->status.word &= 0xF8FFFFFF;
                empty = FIELD_NO_EFFECT;
                i = 2;
                effect_view = FIELD_REGION_AT(record, i);
                do
                {
                    effect_view->status.effects[0] = empty;
                    i--;
                    effect_view = FIELD_REGION_AT(effect_view, -1);
                } while (i >= 0);
            }
            else
            {
                pending = 0;
            }
        } while (pending != 0);
    }
}

/**
 * @brief Advance a party member one level when its experience reaches the threshold.
 * @param index Party member index.
 * @param notify Nonzero to send FIELD_SIGNAL_LEVEL_UP after advancing.
 * @return -1 when the member advanced a level, otherwise 0.
 * @note Companions (character type 3) grow through func_800C1658, the others
 *       through func_800C15AC.
 */
s32 func_800C14A4(s32 index, s32 notify)
{
    FieldGameState *record_view;
    FieldGameState *current_view;
    u32 level_or_state;

    record_view = FIELD_STATE_AT(D_80122B74, index * sizeof(FieldCharacterRecord));
    level_or_state = record_view->characters[0].progress.level;
    if ((s32)(record_view->characters[0].progress.word >> 8) >= FIELD_LEVEL_THRESHOLD((s32)level_or_state))
    {
        record_view->characters[0].progress.level = level_or_state + 1;
        /* One variable for the level and the reloaded state pointer: separate ones allocate differently. */
        level_or_state = (u32)D_80122B74;
        current_view = FIELD_STATE_AT(level_or_state, index * sizeof(FieldCharacterRecord));
        if (current_view->characters[0].progress.level > FIELD_LEVEL_MAX)
        {
            current_view->characters[0].progress.level = FIELD_LEVEL_MAX;
            return 0;
        }
        if ((current_view->characters[0].info.bytes[0] & 0x7F) == FIELD_CHARACTER_TYPE_COMPANION)
        {
            func_800C1658((u8 *)&((FieldGameState *)level_or_state)->characters[index], (FieldGameState *)level_or_state, record_view, index * sizeof(FieldCharacterRecord));
        }
        else
        {
            func_800C15AC(&((FieldGameState *)level_or_state)->characters[index], (FieldGameState *)level_or_state, record_view, index * sizeof(FieldCharacterRecord));
        }
        func_800B7C58(index);
        if (notify != 0)
        {
            func_8008B500(index, FIELD_SIGNAL_LEVEL_UP);
        }
        return -1;
    }
    return 0;
}

/**
 * @brief Level-up growth of a non-companion party member.
 * @param character Party member record.
 * @param unused_state Unused (the game state).
 * @param unused_view Unused (the shifted state view of the member).
 * @param unused_offset Unused (the member's byte offset in the character array).
 * @note Adds the eight four-bit increases of the weapon's item type to the
 *       base stats, clamped to FIELD_STAT_MAX, then recomputes hp.
 */
void func_800C15AC(FieldCharacterRecord *character, FieldGameState *unused_state, FieldGameState *unused_view, s32 unused_offset)
{
    s32 i;
    u32 deltas;
    u16 stat;
    u32 low;

    deltas = D_800F18CC[character->equipment[0].info.bits.item_type];
    for (i = 0; i < FIELD_CHARACTER_STAT_COUNT; i++)
    {
        stat = character->stats[i];
        low = (stat & 0x1FF) + (deltas & 0xF);
        low &= 0x1FF;
        stat = (stat & 0xFE00) | low;
        character->stats[i] = stat;
        if ((stat & 0x1FF) > FIELD_STAT_MAX)
        {
            character->stats[i] = (stat & 0xFE00) | FIELD_STAT_MAX;
        }
        deltas >>= 4;
    }
    character->hp = func_800C19D0(character->hp, (u32)(character->stats[4] & 0x1FF) >> 2, 3);
}

/**
 * @brief Level-up growth of the companion party member, written back to its stored record.
 * @param arg0 Party member record (characters[2]) as bytes.
 * @param unused_state Unused (the game state).
 * @param unused_view Unused (the shifted state view of the member).
 * @param unused_offset Unused (the member's byte offset in the character array).
 * @note Same algorithm as func_800C1230, applied to the party record and its
 *       stored record regions[region_index]. Kept in its byte-offset form: a
 *       typed version (FieldCharacterRecord and regions[] access, growth
 *       views) reached 98.83% at best; the remaining register choice needs the
 *       shared work_value walker and the net-zero pairs below.
 */
void func_800C1658(u8 *arg0, FieldGameState *unused_state, FieldGameState *unused_view, s32 unused_offset)
{
    s32 value_offset;
    s32 extra_mask;
    s32 stat_growth_nibble;
    s32 growth_sum;
    s32 clear_offset;
    s32 index;
    u16 packed_value;
    u16 low_value;
    u16 carried_value;
    u16 saved_value;
    u16 post_result;
    u32 slot;
    u32 extra_word;
    u8 stat_growth_byte;
    u8 value_growth_byte;
    u8 *stat_growth_record;
    u8 *value_growth_record;
    u8 *active_record;
    u8 *growth_flags_record;
    u8 *slot_record;
    u8 *state_record;
    u8 *post_base;
    u8 *saved_value_cursor;
    u32 work_value;

    slot = GROW_U32(FIELD_STATE_BYTES, 0x2EF0);
    if (slot >= 5U)
    {
        record_game_diagnostic(0x8001, 0x79, slot, 0);
    }
    index = 0;
    do
    {
        packed_value = GROW_U16(arg0 + index * 2, 0x30);
        low_value = ((u8) GROW_U8((FIELD_STATE_BYTES + (index - -(s32)(GROW_U32(FIELD_STATE_BYTES, 0x2EF0) * 0x60))), 0x2F40) >> 4) + (packed_value & 0x1FF);
        low_value &= 0x1FF;
        packed_value &= 0xFE00;
        packed_value |= low_value;
        GROW_U16(arg0 + index * 2, 0x30) = packed_value;
        if ((u32) (packed_value & 0x1FF) >= 0x18DU)
        {
            GROW_U16(arg0 + index * 2, 0x30) = (u16) ((packed_value & 0xFE00) | 0x18C);
        }
        stat_growth_record = FIELD_STATE_BYTES + (index + (GROW_U32(FIELD_STATE_BYTES, 0x2EF0) * 0x60));
        stat_growth_byte = GROW_U8(stat_growth_record, 0x2F40);
        index += 1;
        stat_growth_nibble = stat_growth_byte & 0xF;
        GROW_U8(stat_growth_record, 0x2F40) = (s8) (stat_growth_nibble | (stat_growth_nibble * 0x10));
    } while (index < 8);
    carried_value = GROW_U16(arg0, 0x26) + (GROW_U8((u8 *)((s32)FIELD_STATE_BYTES - -(s32)(GROW_U32(FIELD_STATE_BYTES, 0x2EF0) * 0x60)), 0x2F4C) >> 7);
    GROW_U16(arg0, 0x26) = carried_value;
    index = 0;
    GROW_U16((u8 *)((s32)FIELD_STATE_BYTES - -(s32)(GROW_U32(FIELD_STATE_BYTES, 0x2EF0) * 0x60)), 0x2F12) = carried_value;
    GROW_U16(arg0, 0x74) = (u16) GROW_U16(arg0, 0x26);
    slot_record = FIELD_STATE_BYTES + (GROW_U32(FIELD_STATE_BYTES, 0x2EF0) * 0x60);
    extra_mask = -0xF1 + (((s32)slot_record) & ~((s32)slot_record));
    work_value = (u32) extra_mask | 0x70;
    work_value &= GROW_U32(slot_record, 0x2F4C);
    GROW_U32(slot_record, 0x2F4C) = work_value;
    work_value = (u32) arg0;
    active_record = FIELD_STATE_BYTES + (GROW_U32(FIELD_STATE_BYTES, 0x2EF0) * 0x60);
    extra_word = GROW_U32(active_record, 0x2F4C);
    GROW_U32(active_record, 0x2F4C) = (u32) ((extra_word & extra_mask) | (((((extra_word >> 4) & 0xF) + (GROW_U8(active_record, 0x2F4C) & 0xF)) & 0xF) * 0x10));
    do
    {
        work_value++;
        work_value--;
        index++;
        index--;
        saved_value = GROW_U16((u8 *)work_value, 0x28) + ((u8) GROW_U8((FIELD_STATE_BYTES + (index - -(s32)(GROW_U32(FIELD_STATE_BYTES, 0x2EF0) * 0x60))), 0x2F48) >> 7);
        GROW_U16((u8 *)work_value, 0x28) = saved_value;
        value_offset = index * 2;
        GROW_U16((FIELD_STATE_BYTES + (value_offset - -(s32)(GROW_U32(FIELD_STATE_BYTES, 0x2EF0) * 0x60))), 0x2F14) = saved_value;
        saved_value_cursor = arg0;
        saved_value_cursor += value_offset;
        GROW_U16(saved_value_cursor, 0xB4) = (u16) GROW_U16((u8 *)work_value, 0x28);
        growth_flags_record = FIELD_STATE_BYTES + (index + (GROW_U32(FIELD_STATE_BYTES, 0x2EF0) * 0x60));
        GROW_U8(growth_flags_record, 0x2F48) = (u8) (GROW_U8(growth_flags_record, 0x2F48) & 0x7F);
        value_growth_record = FIELD_STATE_BYTES + (index + (GROW_U32(FIELD_STATE_BYTES, 0x2EF0) * 0x60));
        value_growth_byte = GROW_U8(value_growth_record, 0x2F48);
        index += 1;
        growth_sum = value_growth_byte >> 4;
        value_growth_byte &= 0xF;
        growth_sum += value_growth_byte;
        growth_sum *= 0x10;
        value_growth_byte |= growth_sum;
        GROW_U8(value_growth_record, 0x2F48) = value_growth_byte;
        work_value += 2;
    } while (index < 4);
    post_result = func_800C19D0(GROW_U16(arg0, 0x24), (u32) (GROW_U16(arg0, 0x38) & 0x1FF) >> 2, 3);
    post_base = FIELD_STATE_BYTES;
    GROW_U16(arg0, 0x24) = post_result;
    post_base += GROW_U32(post_base, 0x2EF0) * 0x60;
    GROW_U8(post_base, 0x2F0C) = (u8) GROW_U8(arg0, 0x20);
    state_record = FIELD_STATE_BYTES;
    GROW_U16((u8 *)((s32)state_record - -(s32)(GROW_U32(state_record, 0x2EF0) * 0x60)), 0x2F10) = (u16) GROW_U16(arg0, 0x24);
    state_record = (u8 *)((s32)state_record - -(s32)(GROW_U32(state_record, 0x2EF0) * 0x60));
    GROW_U32(state_record, 0x2F38) = (s32) (GROW_U32(state_record, 0x2F38) & 0xF8FFFFFF);
    index = 0;
    work_value = 0xFF;
    do
    {
        clear_offset = index + (GROW_U32(FIELD_STATE_BYTES, 0x2EF0) * 0x60);
        index += 1;
        GROW_U8((FIELD_STATE_BYTES + clear_offset), 0x2F38) = (u8)work_value;
    } while (index < 3);
}

/**
 * @brief Add a stat-derived increase to a value, with optional damping and cap.
 * @param value Starting value.
 * @param increase Amount to add.
 * @param flags Bit 0: increases of 6 or more add 5 plus half of the excess;
 *        bit 1: cap the result at 999.
 * @return The increased value.
 */
s32 func_800C19D0(s32 value, s32 increase, s32 flags)
{
    s32 result;

    if ((u32) increase >= 6 && (flags & 1))
    {
        result = value + ((u32) (increase - 5) >> 1) + 5;
    }
    else
    {
        result = value + increase;
    }
    if ((flags & 2) && (u32) result >= 0x3E8)
    {
        result = 0x3E7;
    }
    return result;
}
