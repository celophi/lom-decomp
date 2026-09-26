/**
 * @file field_group_derived_stats.c
 * @brief Build a golem group's name and derived stats from the items it is made of.
 */

#include "saved_game.h"
#include "common.h"
#include "field_calls.h"
#include "field_golem_layout.h"
#include "field_records.h"

/** @brief func_800C1E40 id of the golem name text table. */
#define FIELD_RESOURCE_GOLEM_NAME_TEXT 0x100
/** @brief Name text entry appended after the golem number (entries 0-9 are the digits). */
#define GOLEM_NAME_TEXT_SUFFIX 10
/** @brief Name text entry of the first name prefix; one prefix per GOLEM_NAME_PREFIX_SPAN golems. */
#define GOLEM_NAME_TEXT_PREFIX 11
/** @brief Golems created per name prefix. */
#define GOLEM_NAME_PREFIX_SPAN 50
/** @brief Creation count from which golems get no number and a bonus on every nibble. */
#define GOLEM_VETERAN_COUNT 200
/** @brief Bonus added to every bonus nibble of a veteran golem. */
#define GOLEM_VETERAN_BONUS 2

/** @brief Number of weapon types in the logic class table. */
#define GOLEM_WEAPON_TYPE_COUNT 27

/** @brief Grid bound with fewer than two armor items. */
#define GOLEM_GRID_BOUND_BASE 4
/** @brief Number of bonus nibbles: eight from the weapons, then eight from the armor. */
#define GOLEM_BONUS_COUNT 16
/** @brief Offset of the armor sums in the bonus accumulators. */
#define GOLEM_ARMOR_BONUS_BASE 8
/** @brief Power range. */
#define GOLEM_POWER_MIN 10
#define GOLEM_POWER_MAX 200
/** @brief Largest equipment total. */
#define GOLEM_EQUIPMENT_TOTAL_MAX 99
/** @brief Largest bonus nibble value. */
#define GOLEM_BONUS_MAX 9
/** @brief Effective stat range. */
#define GOLEM_STAT_MIN 20
#define GOLEM_STAT_MAX 99
/** @brief Max HP range. */
#define GOLEM_MAX_HP_MIN 50
#define GOLEM_MAX_HP_MAX 999

/** @brief Golem name text: a four-byte header, then the entry offsets and the text bytes. */
typedef struct
{
    u8 header[4];
    u8 bytes[1]; /**< Entry n spans bytes[offset(n)] to bytes[offset(n + 1)]; offsets are little-endian halfwords at bytes[2 * n]. */
} GolemNameText;

/** @brief Logic class of each weapon type. */
typedef struct
{
    s32 classes[GOLEM_WEAPON_TYPE_COUNT];
} GolemWeaponClassTable;

/** @brief The game state viewed as FIELD's record layout. */
#define GAME_STATE ((FieldGameState*)g_saved_game.bytes)
/** @brief The golem name text table. */
#define GOLEM_NAME_TEXT ((GolemNameText*)func_800C1E40(FIELD_RESOURCE_GOLEM_NAME_TEXT))

extern u8* func_800C1E40(s32 resource_id);
extern GolemWeaponClassTable D_80051C50;
extern s8 D_800F0C38[];
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];

/**
 * @brief Start offset of a golem name text entry in GolemNameText.bytes.
 * @param entry Text entry index.
 * @return Byte offset of the entry's first character.
 */
static inline s32 golem_name_text_offset(s32 entry)
{
    return GOLEM_NAME_TEXT->bytes[entry * 2] + (GOLEM_NAME_TEXT->bytes[entry * 2 + 1] << 8);
}

/**
 * @brief Build a golem group's name and derived stats from the items selected in GOSUB.
 *
 * The name is a prefix picked by the creation count, the golem number and a
 * suffix. Power, equipment totals, bonus nibbles, stats and flags are summed
 * over the selected weapons and armor and clamped; the last weapon's type
 * picks the logic class and the armor count the grid bound.
 *
 * @param group Golem group record to fill.
 */
void field_golem_build_group_record(s32 group)
{
    GolemWeaponClassTable weapon_classes;
    s32 work[GOLEM_BONUS_COUNT]; /* text range, sums and clamped values; [15] holds the creation count while the name is built */
    s32 length;
    s32 i;
    s32 entry;
    s32 created;
    s32 max_hp;
    s32 hundreds;
    s32 index;

    weapon_classes = D_80051C50;

    length = 0;
    work[15] = GOLEM.header.fields.created_count;
    entry = work[15] / GOLEM_NAME_PREFIX_SPAN;
    entry += GOLEM_NAME_TEXT_PREFIX;
    work[0] = golem_name_text_offset(entry);
    work[1] = golem_name_text_offset(entry + 1);
    for (i = work[0]; i < work[1]; i++)
    {
        if (length < GOLEM_NAME_LENGTH)
        {
            GOLEM.group_records[group].name[length] = GOLEM_NAME_TEXT->bytes[i];
        }
        length++;
    }

    created = work[15];
    if (created < GOLEM_VETERAN_COUNT)
    {
        work[15] = created % GOLEM_NAME_PREFIX_SPAN + 1;
        hundreds = work[15] / 100;
        if (work[15] >= 100)
        {
            work[0] = golem_name_text_offset(hundreds);
            work[1] = golem_name_text_offset(hundreds + 1);
            for (i = work[0]; i < work[1]; i++)
            {
                if (length < GOLEM_NAME_LENGTH)
                {
                    GOLEM.group_records[group].name[length] = GOLEM_NAME_TEXT->bytes[i];
                }
                length++;
            }
        }
        entry = (work[15] % 100) / 10;
        if (work[15] >= 10)
        {
            work[0] = golem_name_text_offset(entry);
            work[1] = golem_name_text_offset(entry + 1);
            for (i = work[0]; i < work[1]; i++)
            {
                if (length < GOLEM_NAME_LENGTH)
                {
                    GOLEM.group_records[group].name[length] = GOLEM_NAME_TEXT->bytes[i];
                }
                length++;
            }
        }
        entry = work[15] % 10;
        work[0] = golem_name_text_offset(entry);
        work[1] = golem_name_text_offset(entry + 1);
        for (i = work[0]; i < work[1]; i++)
        {
            if (length < GOLEM_NAME_LENGTH)
            {
                GOLEM.group_records[group].name[length] = GOLEM_NAME_TEXT->bytes[i];
            }
            length++;
        }
        work[0] = golem_name_text_offset(GOLEM_NAME_TEXT_SUFFIX);
        work[1] = golem_name_text_offset(GOLEM_NAME_TEXT_SUFFIX + 1);
        for (i = work[0]; i < work[1]; i++)
        {
            if (length < GOLEM_NAME_LENGTH)
            {
                GOLEM.group_records[group].name[length] = GOLEM_NAME_TEXT->bytes[i];
            }
            length++;
        }
    }
    if (length < GOLEM_NAME_LENGTH)
    {
        GOLEM.group_records[group].name[length] = 0;
    }

    work[0] = 0;
    for (i = 0; i < g_gosub_result_count; i++)
    {
        index = g_gosub_result_values[i];
        if (GAME_STATE->items[index].info.bits.category == FIELD_ITEM_CATEGORY_WEAPON)
        {
            FieldItemRecord* item = &GAME_STATE->items[index];

            work[0] += item->derived.values[0];
        }
    }
    work[0] = work[0] < GOLEM_POWER_MIN ? GOLEM_POWER_MIN : work[0] > GOLEM_POWER_MAX ? GOLEM_POWER_MAX : work[0];
    GOLEM.group_records[group].power = work[0];

    work[0] = 0;
    work[1] = 0;
    work[2] = 0;
    work[3] = 0;
    for (i = 0; i < g_gosub_result_count; i++)
    {
        index = g_gosub_result_values[i];
        if (GAME_STATE->items[index].info.bits.category == FIELD_ITEM_CATEGORY_ARMOR)
        {
            FieldItemRecord* item = &GAME_STATE->items[index];

            work[0] += item->derived.values[0];
            work[1] += item->derived.values[1];
            work[2] += item->derived.values[2];
            work[3] += item->derived.values[3];
        }
    }
    for (i = 0; i < HISTORY_RECORD_STAT_COUNT; i++)
    {
        work[i] = work[i] < 0 ? 0 : work[i] > GOLEM_EQUIPMENT_TOTAL_MAX ? GOLEM_EQUIPMENT_TOTAL_MAX : work[i];
        GOLEM.group_records[group].equipment_totals[i] = work[i];
    }

    work[0] = 0;
    work[1] = 0;
    work[2] = 0;
    work[3] = 0;
    work[4] = 0;
    work[5] = 0;
    work[6] = 0;
    work[7] = 0;
    work[8] = 0;
    work[9] = 0;
    work[10] = 0;
    work[11] = 0;
    work[12] = 0;
    work[13] = 0;
    work[14] = 0;
    work[15] = 0;
    for (i = 0; i < g_gosub_result_count; i++)
    {
        index = g_gosub_result_values[i];
        if (GAME_STATE->items[index].info.bits.category == FIELD_ITEM_CATEGORY_WEAPON)
        {
            work[0] += GAME_STATE->items[index].bonus_nibbles.bits.n0;
            work[1] += GAME_STATE->items[index].bonus_nibbles.bits.n1;
            work[2] += GAME_STATE->items[index].bonus_nibbles.bits.n2;
            work[3] += GAME_STATE->items[index].bonus_nibbles.bits.n3;
            work[4] += GAME_STATE->items[index].bonus_nibbles.bits.n4;
            work[5] += GAME_STATE->items[index].bonus_nibbles.bits.n5;
            work[6] += GAME_STATE->items[index].bonus_nibbles.bits.n6;
            work[7] += GAME_STATE->items[index].bonus_nibbles.bits.n7;
        }
        else if (GAME_STATE->items[index].info.bits.category == FIELD_ITEM_CATEGORY_ARMOR)
        {
            work[GOLEM_ARMOR_BONUS_BASE + 0] += GAME_STATE->items[index].bonus_nibbles.bits.n0;
            work[GOLEM_ARMOR_BONUS_BASE + 1] += GAME_STATE->items[index].bonus_nibbles.bits.n1;
            work[GOLEM_ARMOR_BONUS_BASE + 2] += GAME_STATE->items[index].bonus_nibbles.bits.n2;
            work[GOLEM_ARMOR_BONUS_BASE + 3] += GAME_STATE->items[index].bonus_nibbles.bits.n3;
            work[GOLEM_ARMOR_BONUS_BASE + 4] += GAME_STATE->items[index].bonus_nibbles.bits.n4;
            work[GOLEM_ARMOR_BONUS_BASE + 5] += GAME_STATE->items[index].bonus_nibbles.bits.n5;
            work[GOLEM_ARMOR_BONUS_BASE + 6] += GAME_STATE->items[index].bonus_nibbles.bits.n6;
            work[GOLEM_ARMOR_BONUS_BASE + 7] += GAME_STATE->items[index].bonus_nibbles.bits.n7;
        }
    }
    for (i = 0; i < GOLEM_BONUS_COUNT; i++)
    {
        if (GOLEM.header.fields.created_count >= GOLEM_VETERAN_COUNT)
        {
            work[i] += GOLEM_VETERAN_BONUS;
        }
        work[i] = work[i] < 0 ? 0 : work[i] > GOLEM_BONUS_MAX ? GOLEM_BONUS_MAX : work[i];
    }
    GOLEM.group_records[group].weapon_bonus.n0 = work[0];
    GOLEM.group_records[group].weapon_bonus.n1 = work[1];
    GOLEM.group_records[group].weapon_bonus.n2 = work[2];
    GOLEM.group_records[group].weapon_bonus.n3 = work[3];
    GOLEM.group_records[group].weapon_bonus.n4 = work[4];
    GOLEM.group_records[group].weapon_bonus.n5 = work[5];
    GOLEM.group_records[group].weapon_bonus.n6 = work[6];
    GOLEM.group_records[group].weapon_bonus.n7 = work[7];
    GOLEM.group_records[group].armor_bonus.n0 = work[GOLEM_ARMOR_BONUS_BASE + 0];
    GOLEM.group_records[group].armor_bonus.n1 = work[GOLEM_ARMOR_BONUS_BASE + 1];
    GOLEM.group_records[group].armor_bonus.n2 = work[GOLEM_ARMOR_BONUS_BASE + 2];
    GOLEM.group_records[group].armor_bonus.n3 = work[GOLEM_ARMOR_BONUS_BASE + 3];
    GOLEM.group_records[group].armor_bonus.n4 = work[GOLEM_ARMOR_BONUS_BASE + 4];
    GOLEM.group_records[group].armor_bonus.n5 = work[GOLEM_ARMOR_BONUS_BASE + 5];
    GOLEM.group_records[group].armor_bonus.n6 = work[GOLEM_ARMOR_BONUS_BASE + 6];
    GOLEM.group_records[group].armor_bonus.n7 = work[GOLEM_ARMOR_BONUS_BASE + 7];

    work[0] = 0;
    work[1] = 0;
    work[2] = 0;
    work[3] = 0;
    work[4] = 0;
    work[5] = 0;
    work[6] = 0;
    work[7] = 0;
    for (i = 0; i < g_gosub_result_count; i++)
    {
        index = g_gosub_result_values[i];
        work[0] += D_800F0C38[GAME_STATE->items[index].stat_nibbles.bits.n0];
        work[1] += D_800F0C38[GAME_STATE->items[index].stat_nibbles.bits.n1];
        work[2] += D_800F0C38[GAME_STATE->items[index].stat_nibbles.bits.n2];
        work[3] += D_800F0C38[GAME_STATE->items[index].stat_nibbles.bits.n3];
        work[4] += D_800F0C38[GAME_STATE->items[index].stat_nibbles.bits.n4];
        work[5] += D_800F0C38[GAME_STATE->items[index].stat_nibbles.bits.n5];
        work[6] += D_800F0C38[GAME_STATE->items[index].stat_nibbles.bits.n6];
        work[7] += D_800F0C38[GAME_STATE->items[index].stat_nibbles.bits.n7];
    }
    for (i = 0; i < FIELD_CHARACTER_STAT_COUNT; i++)
    {
        work[i] = work[i] * 5 + GOLEM_STAT_MIN;
        work[i] = work[i] < GOLEM_STAT_MIN ? GOLEM_STAT_MIN : work[i] > GOLEM_STAT_MAX ? GOLEM_STAT_MAX : work[i];
        /* The base clear is overwritten by the full store below; both stores are in the original. */
        GOLEM.group_records[group].stats[i].bits.base = 0;
        GOLEM.group_records[group].stats[i].value = work[i] << FIELD_STAT_EFFECTIVE_SHIFT;
    }

    work[0] = 0;
    for (i = 0; i < g_gosub_result_count; i++)
    {
        index = g_gosub_result_values[i];
        if (GAME_STATE->items[index].info.bits.category == FIELD_ITEM_CATEGORY_ARMOR)
        {
            FieldItemRecord* item = &GAME_STATE->items[index];

            work[0] |= item->flags2C;
        }
    }
    GOLEM.group_records[group].armor_flags = work[0];
    work[0] = 0;
    for (i = 0; i < g_gosub_result_count; i++)
    {
        index = g_gosub_result_values[i];
        if (GAME_STATE->items[index].info.bits.category == FIELD_ITEM_CATEGORY_WEAPON)
        {
            FieldItemRecord* item = &GAME_STATE->items[index];

            work[0] |= item->flags2C;
        }
    }
    GOLEM.group_records[group].weapon_flags = work[0];
    work[0] = 0;
    for (i = 0; i < g_gosub_result_count; i++)
    {
        index = g_gosub_result_values[i];
        if (GAME_STATE->items[index].info.bits.category == FIELD_ITEM_CATEGORY_ARMOR)
        {
            FieldItemRecord* item = &GAME_STATE->items[index];

            work[0] |= item->flags2D;
        }
    }
    GOLEM.group_records[group].armor_flags2 = work[0];
    GOLEM.group_records[group].unknown_0x3F = 1;
    GOLEM.group_records[group].unknown_0x40 = 0;

    GOLEM.group_records[group].logic_class = 0;
    for (i = 0; i < g_gosub_result_count; i++)
    {
        index = g_gosub_result_values[i];
        if (GAME_STATE->items[index].info.bits.category == FIELD_ITEM_CATEGORY_WEAPON)
        {
            GOLEM.group_records[group].logic_class = weapon_classes.classes[GAME_STATE->items[index].info.bits.item_type];
        }
    }

    work[0] = 0;
    GOLEM.group_records[group].grid_bound = GOLEM_GRID_BOUND_BASE;
    for (i = 0; i < g_gosub_result_count; i++)
    {
        index = g_gosub_result_values[i];
        if (GAME_STATE->items[index].info.bits.category == FIELD_ITEM_CATEGORY_ARMOR)
        {
            work[0]++;
        }
    }
    if (work[0] == 2)
    {
        GOLEM.group_records[group].grid_bound = GOLEM_GRID_BOUND_BASE + 1;
    }
    if (work[0] == 3)
    {
        GOLEM.group_records[group].grid_bound = GOLEM_GRID_BOUND_BASE + 2;
    }

    GOLEM.group_records[group].unknown_0x45 = 0;
    work[0] = 75 - GOLEM.group_records[group].grid_bound * 10;
    work[0] = work[0] < 0 ? 0 : work[0] > 50 ? 50 : work[0];
    GOLEM.group_records[group].unknown_0x46 = work[0];
    GOLEM.group_records[group].unknown_0x47 = 0;
    GOLEM.group_records[group].unknown_0x48 = 0;

    work[0] = GOLEM.group_records[group].power;
    work[1] = GOLEM.group_records[group].equipment_totals[0];
    work[2] = GOLEM.group_records[group].equipment_totals[1];
    work[3] = GOLEM.group_records[group].equipment_totals[2];
    work[4] = GOLEM.group_records[group].equipment_totals[3];
    /* The name length local doubles as the stat total; a separate local changes the register allocation. */
    length = work[0] + work[1] + work[2] + work[3] + work[4];
    length = length * 5 >> 1;
    max_hp = length < GOLEM_MAX_HP_MIN ? GOLEM_MAX_HP_MIN : length > GOLEM_MAX_HP_MAX ? GOLEM_MAX_HP_MAX : length;
    GOLEM.group_records[group].hp = max_hp;
}
