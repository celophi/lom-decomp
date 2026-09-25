/**
 * @file field_record_stat_ops.c
 * @brief Equipment compatibility checks, equipping and derived character stats.
 */

#include "common.h"
#include "field_calls.h"
#include "field_records.h"

/** @brief Character type that gets no equipment totals from its armor slots. */
#define FIELD_CHARACTER_TYPE_NO_ARMOR 3

/** @brief Lowest and highest value of a derived stat. */
#define FIELD_STAT_MIN 1
#define FIELD_STAT_MAX 99

/**
 * @brief Two character stats viewed as one word.
 * @note The base value is stored in quarter units; the effective value is the
 *       base plus the equipment modifiers.
 */
typedef struct FieldStatPair
{
    u32 base0 : 9;
    u32 effective0 : 7;
    u32 base1 : 9;
    u32 effective1 : 7;
} FieldStatPair;

/** @brief A character's stats viewed as FieldStatPair words. */
#define STAT_PAIRS(character) ((FieldStatPair*)(character)->stats)

/** @brief Signed stat modifier for a four-bit modifier index. */
#define STAT_MODIFIER(nibble) (D_800F0C38[nibble])

/**
 * @brief Signed stat modifier for a four-bit modifier index, read as an
 *        unsigned byte and then sign-extended.
 * @note The volatile read keeps the original lbu + sll/sra sequence.
 */
#define STAT_MODIFIER_ZX(nibble) ((s8)((volatile u8*)D_800F0C38)[nibble])

extern FieldGameState* g_field_game_state;

/** @brief Conflict bits of weapon types, indexed by FIELD_ITEM_TYPE. */
extern u8 D_800F0BE0[];

/** @brief Conflict bits of armor types, indexed by FIELD_ITEM_TYPE. */
extern u8 D_800F0BEC[];

/** @brief Signed stat modifiers indexed by an item's four-bit modifier values. */
extern s8 D_800F0C38[];

void* func_800C1EC8(void* src, void* dest, s32 size);
FieldStatusState* field_find_object_state(s32 index);

void func_800B7A74(FieldCharacterRecord* character, s32 skip_slot, u8* conflicts);
void func_800B7B98(FieldCharacterRecord* character);

/**
 * @brief Check whether an item may be equipped in a character's slot.
 * @param character Character whose other equipped items are checked.
 * @param slot_index Equipment slot the item would go into.
 * @param item Item to equip.
 * @return -1 when the item fits the slot and the other equipment, otherwise 0.
 */
s32 func_800B7980(FieldCharacterRecord* character, s32 slot_index, FieldItemRecord* item)
{
    u8 conflicts;
    s32 category;

    category = FIELD_ITEM_CATEGORY(item->info.word);
    switch (category)
    {
    case FIELD_ITEM_CATEGORY_WEAPON:
    {
        s32 overlap;

        if (slot_index != FIELD_WEAPON_SLOT)
        {
            return 0;
        }
        func_800B7A74(character, FIELD_WEAPON_SLOT, &conflicts);
        overlap = D_800F0BE0[FIELD_ITEM_TYPE(item->info.word)] & conflicts;
        if (overlap != 0)
        {
            return 0;
        }
        return -1;
    }
    case FIELD_ITEM_CATEGORY_ARMOR:
    {
        s32 overlap;

        if (slot_index < 1 || slot_index > 3)
        {
            return 0;
        }
        func_800B7A74(character, slot_index, &conflicts);
        overlap = D_800F0BEC[FIELD_ITEM_TYPE(item->info.word)] & conflicts;
        if (overlap != 0)
        {
            return 0;
        }
        return -1;
    }
    case FIELD_ITEM_CATEGORY_INSTRUMENT:
        if (slot_index < FIELD_EQUIPMENT_SLOT_COUNT)
        {
            return 0;
        }
        return -1;
    default:
        return 0;
    }
}

/**
 * @brief Collect the conflict bits of every equipped item except one slot.
 * @param character Character whose equipment is scanned.
 * @param skip_slot Equipment slot left out of the scan.
 * @param conflicts Receives the OR of the conflict bits.
 */
void func_800B7A74(FieldCharacterRecord* character, s32 skip_slot, u8* conflicts)
{
    s32 i;
    u32 value;
    u8* bits;

    i = 0;
    *conflicts = 0;
    for (; i < FIELD_EQUIPMENT_SLOT_COUNT; i++)
    {
        if (i != skip_slot)
        {
            value = character->equipment[i].info.word;
            switch (FIELD_ITEM_CATEGORY(value))
            {
            case FIELD_ITEM_CATEGORY_WEAPON:
                bits = &D_800F0BE0[FIELD_ITEM_TYPE(value)];
                break;
            case FIELD_ITEM_CATEGORY_ARMOR:
                bits = &D_800F0BEC[FIELD_ITEM_TYPE(value)];
                break;
            default:
                continue;
            }
            value = *conflicts;
            value |= *bits;
            *conflicts = value;
        }
    }
}

/**
 * @brief Swap an item into a character's equipment slot when it fits.
 * @param character Character being equipped.
 * @param slot_index Equipment slot to fill.
 * @param item Item to equip; receives the previously equipped item.
 * @return -1 when the item was equipped, otherwise 0.
 */
s32 func_800B7B08(FieldCharacterRecord* character, s32 slot_index, FieldItemRecord* item)
{
    FieldItemRecord previous;
    FieldItemRecord* slot;

    if (func_800B7980(character, slot_index, item) != 0)
    {
        slot = &character->equipment[slot_index];
        func_800C1EC8(slot, &previous, sizeof(FieldItemRecord));
        func_800C1EC8(item, slot, sizeof(FieldItemRecord));
        func_800C1EC8(&previous, item, sizeof(FieldItemRecord));
        func_800B7B98(character);
        return -1;
    }
    return 0;
}

/**
 * @brief Recompute a character's equipment totals and reset its effective stats.
 * @param character Character to update.
 */
void func_800B7B98(FieldCharacterRecord* character)
{
    s32 i;
    s32 j;
    u16 base;

    character->unk26 = character->equipment[FIELD_WEAPON_SLOT].derived.values[0];
    if ((character->info.word & 0x7F) != FIELD_CHARACTER_TYPE_NO_ARMOR)
    {
        for (i = 3; i >= 0; i--)
        {
            character->equipment_totals[i] = 0;
        }
        for (i = 1; i < FIELD_EQUIPMENT_SLOT_COUNT; i++)
        {
            FieldItemRecord* armor = &character->equipment[i];

            if (armor->kind != 0)
            {
                for (j = 0; j < 4; j++)
                {
                    character->equipment_totals[j] += armor->derived.values[j];
                }
            }
        }
    }
    for (i = 0; i < FIELD_CHARACTER_STAT_COUNT; i++)
    {
        base = character->stats[i] & 0x1FF;
        character->stats[i] = base | ((base >> 2) << 9);
    }
}

/**
 * @brief Refresh a party character's equipment and reset its field object HP.
 * @param index Party character index.
 */
void func_800B7C58(s32 index)
{
    FieldStatusState* state;

    func_800B7B98(&g_field_game_state->characters[index]);
    state = field_find_object_state(index);
    state->maximum = g_field_game_state->characters[index].hp;
    if (state->maximum == 0)
    {
        state->maximum = 1;
    }
    state->current = g_field_game_state->characters[index].hp;
    state->gauge.bits.value = g_field_game_state->characters[index].hp;
    state->effect_flags = 0;
}

/**
 * @brief Add an item's stat modifiers to a character's effective stats.
 * @param item Item whose modifiers are applied.
 * @param character Character whose effective stats change (modulo 128).
 */
void func_800B7D10(FieldItemRecord* item, FieldCharacterRecord* character)
{
    STAT_PAIRS(character)[0].effective0 = STAT_MODIFIER_ZX(item->stat_nibbles.bits.n0) + STAT_PAIRS(character)[0].effective0;
    {
        u32 effective = STAT_PAIRS(character)[0].effective1;
        effective += STAT_MODIFIER_ZX(item->stat_nibbles.bits.n1);
        STAT_PAIRS(character)[0].effective1 = effective;
    }
    STAT_PAIRS(character)[1].effective0 = STAT_MODIFIER_ZX(item->stat_nibbles.bits.n2) + STAT_PAIRS(character)[1].effective0;
    {
        u32 effective = STAT_PAIRS(character)[1].effective1;
        effective += STAT_MODIFIER_ZX(item->stat_nibbles.bits.n3);
        STAT_PAIRS(character)[1].effective1 = effective;
    }
    STAT_PAIRS(character)[2].effective0 = STAT_MODIFIER_ZX(item->stat_nibbles.bits.n4) + STAT_PAIRS(character)[2].effective0;
    {
        u32 effective = STAT_PAIRS(character)[2].effective1;
        effective += STAT_MODIFIER_ZX(item->stat_nibbles.bits.n5);
        STAT_PAIRS(character)[2].effective1 = effective;
    }
    STAT_PAIRS(character)[3].effective0 = STAT_MODIFIER_ZX(item->stat_nibbles.bits.n6) + STAT_PAIRS(character)[3].effective0;
    {
        u32 effective = STAT_PAIRS(character)[3].effective1;
        effective += STAT_MODIFIER_ZX(item->stat_nibbles.bits.n7);
        STAT_PAIRS(character)[3].effective1 = effective;
    }
}

/**
 * @brief Compute a character's stat with the modifiers of all equipped items.
 * @param character Character whose stat is evaluated.
 * @param stat_index Stat to evaluate.
 * @return The stat clamped to the range 1 through 99.
 */
s32 func_800B7EE8(FieldCharacterRecord* character, u32 stat_index)
{
    s32 i;
    s32 value;
    s32 result;

    value = (character->stats[stat_index] & 0x1FF) >> 2;
    for (i = 0; i < FIELD_EQUIPMENT_SLOT_COUNT; i++)
    {
        if (character->equipment[i].kind != 0)
        {
            switch (stat_index)
            {
            case 0:
                value += STAT_MODIFIER(character->equipment[i].stat_nibbles.bits.n0);
                break;
            case 1:
                value += STAT_MODIFIER(character->equipment[i].stat_nibbles.bits.n1);
                break;
            case 2:
                value += STAT_MODIFIER(character->equipment[i].stat_nibbles.bits.n2);
                break;
            case 3:
                value += STAT_MODIFIER(character->equipment[i].stat_nibbles.bits.n3);
                break;
            case 4:
                value += STAT_MODIFIER(character->equipment[i].stat_nibbles.bits.n4);
                break;
            case 5:
                value += STAT_MODIFIER(character->equipment[i].stat_nibbles.bits.n5);
                break;
            case 6:
                value += STAT_MODIFIER(character->equipment[i].stat_nibbles.bits.n6);
                break;
            case 7:
                value += STAT_MODIFIER(character->equipment[i].stat_nibbles.bits.n7);
                break;
            }
        }
    }

    if (value > 0)
    {
        result = value;
        if (result > FIELD_STAT_MAX)
        {
            result = FIELD_STAT_MAX;
        }
    }
    else
    {
        result = FIELD_STAT_MIN;
    }
    return result;
}
