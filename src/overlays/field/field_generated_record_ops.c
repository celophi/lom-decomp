/**
 * @file field_generated_record_ops.c
 * @brief Write a staged item back into its item record and derive its
 *        category-specific values.
 */

#include "common.h"
#include "field_calls.h"
#include "field_records.h"

/** @brief Staged levels at or above this are written as FIELD_STAGING_LEVEL_MAX. */
#define FIELD_LEVEL_LIMIT 0x10

/** @brief Clamp a staged level to four bits. */
#define FIELD_CLAMP_LEVEL(level) ((level) >= FIELD_LEVEL_LIMIT ? FIELD_STAGING_LEVEL_MAX : (level))

/** @brief Largest derived value an item record can hold. */
#define FIELD_DERIVED_VALUE_MAX 999

/** @brief Item name table id passed to func_800C1E40. */
#define FIELD_ITEM_NAME_TABLE 8

/** @brief First name entry of the item subtypes in the item name table. */
#define FIELD_SUBTYPE_NAME_BASE 0x24

/** @brief First control code (0x1D-0x1F) that carries one argument byte in item names. */
#define FIELD_TEXT_ARG_CODE_MIN 0x1D

/** @brief Codes below this are control codes in item names. */
#define FIELD_TEXT_CODE_LIMIT 0x20

/** @brief Item name table: a header word, then per-entry offsets relative to @c text. */
typedef struct FieldItemNameTable
{
    u32 header;
    union
    {
        u16 offsets[1];
        u8 text[1];
    } body;
} FieldItemNameTable;

extern FieldGameState* D_80122B74;
extern FieldItemStaging* D_80123FC4;
extern FieldItemTables* D_80123FC0;

void* func_800C1E40(s32 table_id);

void func_800BFE70(s32 type_entry, s32 subtype_entry, u8* dest);

/**
 * @brief Write the staged item back into its item record.
 *
 * A record without a name gets a serial and the name "<subtype> <type>";
 * a named record without a serial only gets the serial. Then the identity,
 * level, stat modifier and slot fields are copied from the staging block.
 */
void func_800BFA34(void)
{
    s32 i;
    FieldItemRecord* record;

    record = D_80123FC4->record;
    if (record->kind == 0)
    {
        func_800C37A8(D_80122B74->unkD8, (struct FieldItemKey*)&record->unk38);
        func_800BFE70(D_80123FC4->category * 16 + D_80123FC4->item_type, D_80123FC4->item_subtype + FIELD_SUBTYPE_NAME_BASE, (u8*)D_80123FC4->record);
    }
    else if (record->unk38 == 0 && record->unk3C == 0)
    {
        func_800C37A8(D_80122B74->unkD8, (struct FieldItemKey*)&record->unk38);
    }

    D_80123FC4->record->info.bits.category = D_80123FC4->category;
    D_80123FC4->record->info.bits.item_type = D_80123FC4->item_type;
    D_80123FC4->record->info.bits.item_subtype = D_80123FC4->item_subtype;

    D_80123FC4->record->bonus_nibbles.bits.n0 = FIELD_CLAMP_LEVEL(D_80123FC4->levels[0].level);
    D_80123FC4->record->bonus_nibbles.bits.n1 = FIELD_CLAMP_LEVEL(D_80123FC4->levels[1].level);
    D_80123FC4->record->bonus_nibbles.bits.n2 = FIELD_CLAMP_LEVEL(D_80123FC4->levels[2].level);
    D_80123FC4->record->bonus_nibbles.bits.n3 = FIELD_CLAMP_LEVEL(D_80123FC4->levels[3].level);
    D_80123FC4->record->bonus_nibbles.bits.n4 = FIELD_CLAMP_LEVEL(D_80123FC4->levels[4].level);
    D_80123FC4->record->bonus_nibbles.bits.n5 = FIELD_CLAMP_LEVEL(D_80123FC4->levels[5].level);
    D_80123FC4->record->bonus_nibbles.bits.n6 = FIELD_CLAMP_LEVEL(D_80123FC4->levels[6].level);
    D_80123FC4->record->bonus_nibbles.bits.n7 = FIELD_CLAMP_LEVEL(D_80123FC4->levels[7].level);

    D_80123FC4->record->stat_nibbles.bits.n0 = D_80123FC4->stats.bytes[0];
    D_80123FC4->record->stat_nibbles.bits.n1 = D_80123FC4->stats.words[0].modifier1;
    D_80123FC4->record->stat_nibbles.bits.n2 = D_80123FC4->stats.words[0].modifier2;
    D_80123FC4->record->stat_nibbles.bits.n3 = D_80123FC4->stats.words[0].modifier3;
    D_80123FC4->record->stat_nibbles.bits.n4 = D_80123FC4->stats.bytes[4];
    D_80123FC4->record->stat_nibbles.bits.n5 = D_80123FC4->stats.words[1].modifier1;
    D_80123FC4->record->stat_nibbles.bits.n6 = D_80123FC4->stats.words[1].modifier2;
    D_80123FC4->record->stat_nibbles.bits.n7 = D_80123FC4->stats.words[1].modifier3;

    for (i = 0; i < 3; i++)
    {
        D_80123FC4->record->special_ids[i] = D_80123FC4->slots[i + 2];
    }
    D_80123FC4->record->special_ids[3] = D_80123FC4->slots[1];
    D_80123FC4->record->handle = 0;
}

/**
 * @brief Build an item name from a subtype name followed by a type name.
 * @param type_entry Name table entry of the item type.
 * @param subtype_entry Name table entry of the item subtype.
 * @param dest Destination buffer; receives the terminated name.
 */
void func_800BFE70(s32 type_entry, s32 subtype_entry, u8* dest)
{
    FieldItemNameTable* table;
    u8* src;

    table = func_800C1E40(FIELD_ITEM_NAME_TABLE);

    src = &table->body.text[table->body.offsets[subtype_entry]];
    if (*src != 0)
    {
        s32 code;

        do
        {
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
        } while (*src != 0);
    }

    src = &table->body.text[table->body.offsets[type_entry]];
    if (*src != 0)
    {
        s32 code;

        do
        {
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
        } while (*src != 0);
    }

    *dest = 0;
}

/**
 * @brief Derive the category 0 values of an item record from the staging block.
 *
 * The power is the type/subtype weight product scaled by the summed levels,
 * capped at FIELD_DERIVED_VALUE_MAX; properties, flags and the effect index
 * are copied and the four factors are scaled by the subtype multipliers.
 *
 * @param record Item record to update.
 */
void func_800BFF90(FieldItemRecord* record)
{
    s32 i;
    s32 weight;
    s32 levels;
    u16 divisor;
    u16 power;

    for (i = 0, weight = 0; i < 4; i++)
    {
        weight += D_80123FC0->item.types[D_80123FC4->item_type].weights[i] * D_80123FC4->weights[i];
    }

    for (i = 0, levels = 0; i < FIELD_STAGING_LEVEL_COUNT; i++)
    {
        levels += D_80123FC4->levels[i].level;
    }

    divisor = D_80123FC0->item.subtypes[D_80123FC4->item_subtype].divisor;
    power = (weight * (levels + divisor) / divisor) >> 7;
    record->derived.weapon.power = power;
    if (power >= 1000)
    {
        record->derived.weapon.power = FIELD_DERIVED_VALUE_MAX;
    }

    for (i = 0; i < FIELD_STAGING_PROPERTY_COUNT; i++)
    {
        record->derived.weapon.stats[i] = D_80123FC4->properties[i];
    }
    record->flags2C = D_80123FC4->flags2C;
    record->effect_index = D_80123FC4->effect_index;

    for (i = 0; i < 4; i++)
    {
        record->attributes[i] = (D_80123FC0->item.types[D_80123FC4->item_type].factors[i] * D_80123FC4->multipliers[i]) >> 6;
    }
}

/**
 * @brief Derive the category 1 values of an item record from the staging block.
 * @param record Item record to update.
 */
void func_800C015C(FieldItemRecord* record)
{
    s32 i;
    u32 value;

    for (i = 0; i < 4; i++)
    {
        value = (D_80123FC0->item.alternate_types[D_80123FC4->item_type].weights[i] * D_80123FC4->multipliers[i]) >> 6;
        record->derived.values[i] = value;
        if (value >= 1000)
        {
            record->derived.values[i] = FIELD_DERIVED_VALUE_MAX;
        }
        record->attributes[i] = (D_80123FC0->item.alternate_types[D_80123FC4->item_type].factors[i] * D_80123FC4->multipliers[i]) >> 6;
    }

    record->flags2C = D_80123FC4->alternate_flags2C;
    record->flags2D = D_80123FC4->flags2D;
    record->effect_index = D_80123FC4->effect_index;
}
