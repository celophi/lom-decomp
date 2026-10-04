/**
 * @file field_generated_record_ops.c
 * @brief Write a staged item back into its item record and derive its
 *        category-specific values.
 */

#include "common.h"
#include "../internal/field_calls.h"
#include "../internal/field_records.h"

/** @brief Staged levels at or above this are written as FIELD_STAGING_LEVEL_MAX. */
#define FIELD_LEVEL_LIMIT 0x10

/** @brief Level written to a four-bit record field; JP truncates, US saturates. */
#if defined(VERSION_JP)
#define FIELD_RECORD_LEVEL(level) (level)
#else
#define FIELD_RECORD_LEVEL(level) ((level) >= FIELD_LEVEL_LIMIT ? FIELD_STAGING_LEVEL_MAX : (level))
#endif

/** @brief Largest derived value an item record can hold. */
#define FIELD_DERIVED_VALUE_MAX 999

/** @brief Resource id (field_find_resource) of the item name table. */
#define FIELD_ITEM_NAME_TABLE 8

/** @brief Name table entries per item category: the type names of one category. */
#define FIELD_TYPE_NAMES_PER_CATEGORY 16

/** @brief First name entry of the materials in the item name table. */
#define FIELD_MATERIAL_NAME_BASE 36

/** @brief First control code (0x1D-0x1F) that carries one argument byte in item names. */
#define FIELD_TEXT_ARG_CODE_MIN 0x1D

/** @brief Codes below this are control codes in item names. */
#define FIELD_TEXT_CODE_LIMIT 0x20

/** @brief Item name table: a header word, then per-entry offsets relative to @c text. */
typedef struct FieldGeneratedItemNameTable
{
    u32 header;
    union
    {
        u16 offsets[1];
        u8 text[1];
    } body;
} FieldGeneratedItemNameTable;

static void field_build_item_name(s32 type_entry, s32 material_entry, u8* dest);

/**
 * @brief Write the staged item back into its item record.
 *
 * A record without a name gets a serial and the name "<material> <type>";
 * a named record without a serial only gets the serial. Then the identity,
 * level, stat modifier and slot fields are copied from the staging block.
 * @note JP stores the low four bits of each level; US caps levels at 15.
 */
void field_write_staged_item(void)
{
    s32 i;
    FieldItemRecord* record;

    record = g_field_item_staging->record;
    if (record->name[0] == 0)
    {
        field_generate_item_key(g_field_game_state->guest_origin.ids.game_id, &record->key);
        field_build_item_name(g_field_item_staging->category * FIELD_TYPE_NAMES_PER_CATEGORY + g_field_item_staging->item_type,
                              g_field_item_staging->material + FIELD_MATERIAL_NAME_BASE, g_field_item_staging->record->name);
    }
    else if (record->key.first == 0 && record->key.second == 0)
    {
        field_generate_item_key(g_field_game_state->guest_origin.ids.game_id, &record->key);
    }

    g_field_item_staging->record->info.bits.category = g_field_item_staging->category;
    g_field_item_staging->record->info.bits.item_type = g_field_item_staging->item_type;
    g_field_item_staging->record->info.bits.material = g_field_item_staging->material;

    g_field_item_staging->record->bonus_nibbles.bits.n0 = FIELD_RECORD_LEVEL(g_field_item_staging->levels[0].level);
    g_field_item_staging->record->bonus_nibbles.bits.n1 = FIELD_RECORD_LEVEL(g_field_item_staging->levels[1].level);
    g_field_item_staging->record->bonus_nibbles.bits.n2 = FIELD_RECORD_LEVEL(g_field_item_staging->levels[2].level);
    g_field_item_staging->record->bonus_nibbles.bits.n3 = FIELD_RECORD_LEVEL(g_field_item_staging->levels[3].level);
    g_field_item_staging->record->bonus_nibbles.bits.n4 = FIELD_RECORD_LEVEL(g_field_item_staging->levels[4].level);
    g_field_item_staging->record->bonus_nibbles.bits.n5 = FIELD_RECORD_LEVEL(g_field_item_staging->levels[5].level);
    g_field_item_staging->record->bonus_nibbles.bits.n6 = FIELD_RECORD_LEVEL(g_field_item_staging->levels[6].level);
    g_field_item_staging->record->bonus_nibbles.bits.n7 = FIELD_RECORD_LEVEL(g_field_item_staging->levels[7].level);

    g_field_item_staging->record->stat_nibbles.bits.n0 = g_field_item_staging->stats.bytes[0];
    g_field_item_staging->record->stat_nibbles.bits.n1 = g_field_item_staging->stats.words[0].modifier1;
    g_field_item_staging->record->stat_nibbles.bits.n2 = g_field_item_staging->stats.words[0].modifier2;
    g_field_item_staging->record->stat_nibbles.bits.n3 = g_field_item_staging->stats.words[0].modifier3;
    g_field_item_staging->record->stat_nibbles.bits.n4 = g_field_item_staging->stats.bytes[4];
    g_field_item_staging->record->stat_nibbles.bits.n5 = g_field_item_staging->stats.words[1].modifier1;
    g_field_item_staging->record->stat_nibbles.bits.n6 = g_field_item_staging->stats.words[1].modifier2;
    g_field_item_staging->record->stat_nibbles.bits.n7 = g_field_item_staging->stats.words[1].modifier3;

    for (i = 0; i < 3; i++)
    {
        g_field_item_staging->record->special_ids[i] = g_field_item_staging->slots[i + 2];
    }
    g_field_item_staging->record->special_ids[3] = g_field_item_staging->slots[1];
    g_field_item_staging->record->value = 0;
}

/**
 * @brief Build an item name from a material name followed by a type name.
 * @param type_entry Name table entry of the item type.
 * @param material_entry Name table entry of the material.
 * @param dest Destination buffer; receives the terminated name.
 */
static void field_build_item_name(s32 type_entry, s32 material_entry, u8* dest)
{
    FieldGeneratedItemNameTable* table;
    u8* src;

    table = field_find_resource(FIELD_ITEM_NAME_TABLE);

    src = &table->body.text[table->body.offsets[material_entry]];
    while (*src != 0)
    {
        s32 code;

        code = *src;
        if (code < FIELD_TEXT_CODE_LIMIT)
        {
            if (code >= FIELD_TEXT_ARG_CODE_MIN)
            {
                *dest = code;
                src++;
                dest++;
            }
        }
        *dest++ = *src++;
    }

    src = &table->body.text[table->body.offsets[type_entry]];
    while (*src != 0)
    {
        s32 code;

        code = *src;
        if (code < FIELD_TEXT_CODE_LIMIT)
        {
            if (code >= FIELD_TEXT_ARG_CODE_MIN)
            {
                *dest = code;
                src++;
                dest++;
            }
        }
        *dest++ = *src++;
    }

    *dest = 0;
}

/**
 * @brief Derive a weapon's attack power and other values from the staging block.
 *
 * The power is the type/material weight product scaled by the summed levels,
 * capped at FIELD_DERIVED_VALUE_MAX; properties, flags and the effect index
 * are copied and the four factors are scaled by the material multipliers.
 *
 * @param record Item record to update.
 * @note JP does not cap the power at FIELD_DERIVED_VALUE_MAX.
 */
void field_derive_weapon_values(FieldItemRecord* record)
{
    s32 i;
    s32 weight;
    s32 levels;
    u16 divisor;
#if !defined(VERSION_JP)
    u16 power;
#endif

    for (i = 0, weight = 0; i < FIELD_STAGING_FACTOR_COUNT; i++)
    {
        weight += g_field_item_tables->item.weapon_types[g_field_item_staging->item_type].weights[i] * g_field_item_staging->weights[i];
    }

    for (i = 0, levels = 0; i < FIELD_STAGING_LEVEL_COUNT; i++)
    {
        levels += g_field_item_staging->levels[i].level;
    }

    divisor = g_field_item_tables->item.materials[g_field_item_staging->material].divisor;
#if defined(VERSION_JP)
    record->derived.weapon.power = (weight * (levels + divisor) / divisor) >> 7;
#else
    power = (weight * (levels + divisor) / divisor) >> 7;
    record->derived.weapon.power = power;
    if (power >= 1000)
    {
        record->derived.weapon.power = FIELD_DERIVED_VALUE_MAX;
    }
#endif

    for (i = 0; i < FIELD_STAGING_PROPERTY_COUNT; i++)
    {
        record->derived.weapon.stats[i] = g_field_item_staging->properties[i];
    }
    record->status_flags = g_field_item_staging->power_flags;
    record->effect_index = g_field_item_staging->effect_index;

    for (i = 0; i < FIELD_STAGING_FACTOR_COUNT; i++)
    {
        record->attributes[i] =
            (g_field_item_tables->item.weapon_types[g_field_item_staging->item_type].factors[i] * g_field_item_staging->multipliers[i]) >> 6;
    }
}

/**
 * @brief Derive a piece of armor's defense values from the staging block.
 * @param record Item record to update.
 * @note JP does not cap the derived values at FIELD_DERIVED_VALUE_MAX.
 */
void field_derive_armor_values(FieldItemRecord* record)
{
    s32 i;
#if !defined(VERSION_JP)
    u32 value;
#endif

    for (i = 0; i < FIELD_STAGING_FACTOR_COUNT; i++)
    {
#if defined(VERSION_JP)
        record->derived.values[i] =
            (g_field_item_tables->item.armor_types[g_field_item_staging->item_type].weights[i] * g_field_item_staging->multipliers[i]) >> 6;
#else
        value = (g_field_item_tables->item.armor_types[g_field_item_staging->item_type].weights[i] * g_field_item_staging->multipliers[i]) >> 6;
        record->derived.values[i] = value;
        if (value >= 1000)
        {
            record->derived.values[i] = FIELD_DERIVED_VALUE_MAX;
        }
#endif
        record->attributes[i] = (g_field_item_tables->item.armor_types[g_field_item_staging->item_type].factors[i] * g_field_item_staging->multipliers[i]) >> 6;
    }

    record->status_flags = g_field_item_staging->immunity_flags;
    record->element_flags = g_field_item_staging->element_flags;
    record->effect_index = g_field_item_staging->effect_index;
}
