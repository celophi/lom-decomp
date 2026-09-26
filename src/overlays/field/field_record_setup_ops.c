/**
 * @file field_record_setup_ops.c
 * @brief Stage an item record: fill the staging block for a new or existing
 *        item, run the generation scripts and write the result back.
 */

#include "common.h"
#include "field_calls.h"
#include "field_records.h"
#include "field_script.h"

/** @brief func_800C1E40 id of the category 0/1 item table. */
#define FIELD_ITEM_TABLE 4

/** @brief func_800C1E40 id of the category 2 item table. */
#define FIELD_ITEM_GRID_TABLE 0xF

/** @brief Script variable that receives the staged item's inventory index. */
#define FIELD_ITEM_RESULT_VARIABLE 0x7100

/** @brief Inventory index reported when the inventory is full. */
#define FIELD_ITEM_RESULT_FULL 0xFE

/** @brief Inventory index reported when no gosub result was queued. */
#define FIELD_ITEM_RESULT_NONE 0xFF

/** @brief Default stat modifier index written to every staged stat. */
#define FIELD_DEFAULT_STAT_MODIFIER 4

/** @brief Staged slot values below this set the staging slot class. */
#define FIELD_SLOT_CLASS_LIMIT 0x10

/** @brief Largest row or column of the category 2 grid. */
#define FIELD_GRID_MAX 7

/** @brief func_800BE710 kind that stages a new category 2 item. */
#define FIELD_STAGE_NEW_GRID_ITEM 2

/** @brief func_800BE710 kind that restages an existing inventory item. */
#define FIELD_STAGE_EXISTING_ITEM 3

extern s32 D_801227F0;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern FieldGameState* g_field_game_state;
extern FieldRuntimeContext* g_field_runtime;
extern FieldItemStaging* D_80123FC4;
extern FieldItemTables* D_80123FC0;

FieldItemRecord* field_find_free_inventory_record(void);
void func_800BD520(s32 owner, s32 variable, s32 value);
s32* func_800C1EC8(s32* src, s32* dest, s32 size);
void* func_800C1E40(s32 table_id);

void func_800BE888(FieldItemRecord* record, s32 category, s32 item_type, s32 item_subtype);
void func_800BEA10(FieldItemRecord* record, s32 category, s32 item_type, s32 row, s32 command_index);
void func_800BEC44(FieldItemRecord* record, s32 command_index);
void func_800BEF74(void);
void func_800BF158(void);

/**
 * @brief Stage an item from the queued gosub results and report its inventory index.
 *
 * Kind 2 creates a category 2 item, kind 3 restages the inventory item
 * given by the first result, any other kind creates an item of that
 * category. The inventory index, FIELD_ITEM_RESULT_FULL or
 * FIELD_ITEM_RESULT_NONE is written to script variable
 * FIELD_ITEM_RESULT_VARIABLE.
 *
 * @param kind Staging kind; also the category of a new category 0/1 item.
 */
void func_800BE710(s32 kind)
{
    FieldItemRecord* record;

    D_801227F0 = 0;
    func_800C1EC8(NULL, (s32*)D_80123FC4, sizeof(FieldItemStaging));

    if (g_gosub_result_count != 0)
    {
        switch (kind)
        {
        case FIELD_STAGE_NEW_GRID_ITEM:
            record = field_find_free_inventory_record();
            if (record != NULL)
            {
                func_800BEA10(record, FIELD_STAGE_NEW_GRID_ITEM, g_gosub_result_values[0], g_gosub_result_values[1], g_gosub_result_values[2]);
                func_800BD520(0, FIELD_ITEM_RESULT_VARIABLE, record - g_field_game_state->items);
            }
            else
            {
                func_800BD520(0, FIELD_ITEM_RESULT_VARIABLE, FIELD_ITEM_RESULT_FULL);
            }
            break;
        case FIELD_STAGE_EXISTING_ITEM:
            func_800BEC44(&g_field_game_state->items[g_gosub_result_values[0]], g_gosub_result_values[1]);
            func_800BD520(0, FIELD_ITEM_RESULT_VARIABLE, g_gosub_result_values[0]);
            break;
        default:
            record = field_find_free_inventory_record();
            if (record != NULL)
            {
                func_800BE888(record, kind, g_gosub_result_values[0], g_gosub_result_values[1]);
                func_800BD520(0, FIELD_ITEM_RESULT_VARIABLE, record - g_field_game_state->items);
            }
            else
            {
                func_800BD520(0, FIELD_ITEM_RESULT_VARIABLE, FIELD_ITEM_RESULT_FULL);
            }
            break;
        }
    }
    else
    {
        func_800BD520(0, FIELD_ITEM_RESULT_VARIABLE, FIELD_ITEM_RESULT_NONE);
    }
}

/**
 * @brief Stage a new category 0/1 item and generate it.
 * @param record Item record that receives the item.
 * @param category Item category.
 * @param item_type Item type.
 * @param item_subtype Item subtype.
 */
void func_800BE888(FieldItemRecord* record, s32 category, s32 item_type, s32 item_subtype)
{
    FieldItemStaging* staging;
    s32 i;

    func_800C21C0(item_subtype);

    /* The first two stores go through a local copy of D_80123FC4. */
    staging = D_80123FC4;
    staging->category = category;
    staging->record = record;
    D_80123FC4->item_type = item_type;
    D_80123FC4->item_subtype = item_subtype;
    D_80123FC4->command_index = 0xFF;

    for (i = 0; i < FIELD_STAGING_STAT_COUNT; i++)
    {
        D_80123FC4->stats.bytes[i] = (D_80123FC4->stats.bytes[i] & 0xF0) | FIELD_DEFAULT_STAT_MODIFIER;
        D_80123FC4->base_stats[i] = FIELD_DEFAULT_STAT_MODIFIER;
    }

    for (i = 0; i < FIELD_STAGING_SLOT_COUNT; i++)
    {
        D_80123FC4->slots[i] = FIELD_STAGING_SLOT_EMPTY;
    }

    D_80123FC4->properties[0] = D_80123FC4->item_type << 4;
    D_80123FC4->properties[1] = (D_80123FC4->item_type << 4) + 0xC;
    D_80123FC4->properties[2] = (D_80123FC4->item_type << 4) + 0xD;
    D_80123FC4->properties[3] = (D_80123FC4->item_type << 4) + 0xE;
    D_80123FC4->properties[4] = (D_80123FC4->item_type << 4) + 0xF;
    D_80123FC4->properties[5] = 0xFF;

    func_800BEF74();
}

/**
 * @brief Stage a new category 2 item, run its command script and store its grid values.
 * @param record Item record that receives the item.
 * @param category Item category.
 * @param item_type Item type.
 * @param row Row of the grid table pair and item subtype.
 * @param command_index Command selector, biased by FIELD_STAGING_COMMAND_BASE.
 */
void func_800BEA10(FieldItemRecord* record, s32 category, s32 item_type, s32 row, s32 command_index)
{
    FieldScriptContext* saved_script;
    s32 column;
    s32 grid_row;
    s32 i;

    func_800C21C0(row);
    func_800C21C0(command_index);
    D_80123FC4->record = record;
    D_80123FC4->category = category;
    D_80123FC4->item_type = item_type;
    D_80123FC4->item_subtype = row;
    D_80123FC4->command_index = command_index;

    for (i = 0; i < FIELD_STAGING_STAT_COUNT; i++)
    {
        D_80123FC4->stats.bytes[i] = (D_80123FC4->stats.bytes[i] & 0xF0) | FIELD_DEFAULT_STAT_MODIFIER;
    }
    for (i = 0; i < FIELD_STAGING_SLOT_COUNT; i++)
    {
        D_80123FC4->slots[i] = FIELD_STAGING_SLOT_EMPTY;
    }

    D_80123FC0 = func_800C1E40(FIELD_ITEM_GRID_TABLE);
    D_80123FC4->properties[1] = D_80123FC0->grid.pairs[row][item_type][0] & 7;
    D_80123FC4->properties[2] = D_80123FC0->grid.pairs[row][item_type][0] >> 3;
    D_80123FC4->properties[3] = D_80123FC0->grid.pairs[row][item_type][1];

    saved_script = g_field_script;
    g_field_script = (FieldScriptContext*)&g_field_runtime->events[0].script;
    func_800BF2F0(D_80123FC0->grid.commands[command_index - FIELD_STAGING_COMMAND_BASE]);
    g_field_script = saved_script;
    field_write_staged_item();

    record->derived.bytes[0] = D_80123FC4->properties[0];

    if ((s8)D_80123FC4->properties[1] >= 0)
    {
        column = FIELD_GRID_MAX;
        if (D_80123FC4->properties[1] <= FIELD_GRID_MAX)
        {
            column = D_80123FC4->properties[1];
        }
    }
    else
    {
        column = 0;
    }

    if ((s8)D_80123FC4->properties[2] >= 0)
    {
        grid_row = FIELD_GRID_MAX;
        if (D_80123FC4->properties[2] <= FIELD_GRID_MAX)
        {
            grid_row = D_80123FC4->properties[2];
        }
    }
    else
    {
        grid_row = 0;
    }

    record->derived.bytes[1] = D_80123FC0->grid.grid[grid_row][column];
    record->derived.bytes[2] = D_80123FC4->properties[3];
}

/**
 * @brief Stage an existing item record and regenerate it.
 * @param record Item record to restage.
 * @param command_index Command selector, biased by FIELD_STAGING_COMMAND_BASE.
 */
void func_800BEC44(FieldItemRecord* record, s32 command_index)
{
    s32 i;

    D_80123FC0 = func_800C1E40(FIELD_ITEM_TABLE);
    func_800C21C0(command_index);

    D_80123FC4->record = record;
    D_80123FC4->category = record->info.bits.category;
    D_80123FC4->item_type = record->info.bits.item_type;
    D_80123FC4->item_subtype = record->info.bits.item_subtype;
    D_80123FC4->command_index = command_index;
    D_80123FC4->pool += D_80123FC0->item.commands[command_index - FIELD_STAGING_COMMAND_BASE].pool_bonus;

    D_80123FC4->levels[0].level = record->bonus_nibbles.bits.n0;
    D_80123FC4->levels[1].level = record->bonus_nibbles.bits.n1;
    D_80123FC4->levels[2].level = record->bonus_nibbles.bits.n2;
    D_80123FC4->levels[3].level = record->bonus_nibbles.bits.n3;
    D_80123FC4->levels[4].level = record->bonus_nibbles.bits.n4;
    D_80123FC4->levels[5].level = record->bonus_nibbles.bits.n5;
    D_80123FC4->levels[6].level = record->bonus_nibbles.bits.n6;
    D_80123FC4->levels[7].level = record->bonus_nibbles.bits.n7;
    D_80123FC4->effect_index = record->effect_index;

    D_80123FC4->stats.words[0].modifier0 = record->stat_nibbles.bits.n0;
    D_80123FC4->stats.words[0].modifier1 = record->stat_nibbles.bits.n1;
    D_80123FC4->stats.words[0].modifier2 = record->stat_nibbles.bits.n2;
    D_80123FC4->stats.words[0].modifier3 = record->stat_nibbles.bits.n3;
    D_80123FC4->stats.words[1].modifier0 = record->stat_nibbles.bits.n4;
    D_80123FC4->stats.words[1].modifier1 = record->stat_nibbles.bits.n5;
    D_80123FC4->stats.words[1].modifier2 = record->stat_nibbles.bits.n6;
    D_80123FC4->stats.words[1].modifier3 = record->stat_nibbles.bits.n7;

    for (i = 0; i < FIELD_STAGING_STAT_COUNT; i++)
    {
        D_80123FC4->base_stats[i] = FIELD_DEFAULT_STAT_MODIFIER;
    }

    D_80123FC4->slots[0] = FIELD_STAGING_SLOT_EMPTY;
    D_80123FC4->slots[1] = record->special_ids[3];
    for (i = 0; i < 3; i++)
    {
        D_80123FC4->slots[i + 2] = record->special_ids[i];
    }
    D_80123FC4->slots[5] = FIELD_STAGING_SLOT_EMPTY;

    switch (D_80123FC4->category)
    {
    case FIELD_ITEM_CATEGORY_WEAPON:
        for (i = 0; i < FIELD_STAGING_PROPERTY_COUNT; i++)
        {
            D_80123FC4->properties[i] = record->derived.weapon.stats[i];
        }
        D_80123FC4->flags2C = 0;
        break;
    case FIELD_ITEM_CATEGORY_ARMOR:
        D_80123FC4->flags2D = 0;
        D_80123FC4->alternate_flags2C = record->flags2C;
        break;
    }

    func_800BEF74();
}

/**
 * @brief Run the generation scripts of the staged item and write it back.
 *
 * The type, subtype, command and slot scripts run on the event script
 * context; pending levels, flags and stat clamping are applied, the item is
 * written back and the category 0/1 derived values are computed.
 *
 */
void func_800BEF74(void)
{
    FieldScriptContext* saved_script;

    saved_script = g_field_script;
    g_field_script = (FieldScriptContext*)&g_field_runtime->events[0].script;
    func_800BF158();
    if (D_80123FC4->category == FIELD_ITEM_CATEGORY_WEAPON)
    {
        func_800BF2F0(D_80123FC0->item.types[D_80123FC4->item_type].scripts[0]);
    }
    else
    {
        func_800BF2F0(D_80123FC0->item.alternate_types[D_80123FC4->item_type].scripts[0]);
    }
    func_800BF2F0(D_80123FC0->item.subtypes[D_80123FC4->item_subtype].script);
    func_800BF2F0(D_80123FC0->item.commands[D_80123FC4->command_index - FIELD_STAGING_COMMAND_BASE].script);
    func_800BF3D8();
    func_800BF800();
    if (D_80123FC4->category == FIELD_ITEM_CATEGORY_WEAPON)
    {
        func_800BF2F0(D_80123FC0->item.types[D_80123FC4->item_type].scripts[1]);
    }
    else
    {
        func_800BF2F0(D_80123FC0->item.alternate_types[D_80123FC4->item_type].scripts[1]);
    }
    func_800BF700();
    g_field_script = saved_script;
    field_write_staged_item();

    switch (D_80123FC4->category)
    {
    case FIELD_ITEM_CATEGORY_WEAPON:
        field_derive_weapon_values(D_80123FC4->record);
        return;
    case FIELD_ITEM_CATEGORY_ARMOR:
        field_derive_armor_values(D_80123FC4->record);
        return;
    }
}

/**
 * @brief Load the item table and copy the staged subtype's entry into the staging block.
 */
void func_800BF158(void)
{
    s32 i;

    D_80123FC0 = func_800C1E40(FIELD_ITEM_TABLE);
    D_80123FC4->unk3A = D_80123FC0->item.subtypes[D_80123FC4->item_subtype].divisor;
    for (i = 0; i < 4; i++)
    {
        D_80123FC4->weights[i] = D_80123FC0->item.subtypes[D_80123FC4->item_subtype].weights[i];
    }
    for (i = 0; i < 4; i++)
    {
        D_80123FC4->multipliers[i] = D_80123FC0->item.subtypes[D_80123FC4->item_subtype].multipliers[i];
    }
    for (i = 0; i < FIELD_STAGING_LEVEL_COUNT; i++)
    {
        D_80123FC4->levels[i].cost = D_80123FC0->item.subtypes[D_80123FC4->item_subtype].costs[i];
        D_80123FC4->pending_levels[i] = 0;
    }

    D_80123FC4->flags.bits.slot_class = 0xF;
    for (i = 1; i < 5; i++)
    {
        if (D_80123FC4->slots[i] < FIELD_SLOT_CLASS_LIMIT)
        {
            D_80123FC4->flags.bits.slot_class = D_80123FC4->slots[i];
        }
    }
}
