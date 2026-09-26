/**
 * @file field_record_setup_ops.c
 * @brief Item creation and tempering: fill the staging block for a new or
 *        existing item, run the generation scripts and write the result back.
 */

#include "common.h"
#include "field_calls.h"
#include "field_records.h"
#include "field_script.h"

/** @brief func_800C1E40 id of the weapon and armor generation table. */
#define FIELD_ITEM_TABLE 4

/** @brief func_800C1E40 id of the instrument generation table. */
#define FIELD_ITEM_GRID_TABLE 0xF

/** @brief Script variable that receives the created item's inventory index. */
#define FIELD_ITEM_RESULT_VARIABLE 0x7100

/** @brief Inventory index reported when the inventory is full. */
#define FIELD_ITEM_RESULT_FULL 0xFE

/** @brief Inventory index reported when no gosub result was queued. */
#define FIELD_ITEM_RESULT_NONE 0xFF

/** @brief field_create_item_from_gosub kind that tempers an existing inventory item. */
#define FIELD_CREATE_KIND_TEMPER 3

/** @brief Command selector of a new weapon or armor: no tempering item. */
#define FIELD_ITEM_COMMAND_NONE 0xFF

/** @brief Default stat modifier index written to every staged stat. */
#define FIELD_DEFAULT_STAT_MODIFIER 4

/** @brief Bounds-row nibble of a staged stats byte (the low nibble is the modifier). */
#define FIELD_STAGING_BOUNDS_MASK 0xF0

/** @brief FieldItemStaging slot_class when no slot value is below FIELD_SLOT_CLASS_LIMIT. */
#define FIELD_SLOT_CLASS_NONE 0xF

/** @brief Staged slot values below this set the staging slot class. */
#define FIELD_SLOT_CLASS_LIMIT 0x10

/** @brief Largest row or column of the instrument grid. */
#define FIELD_GRID_MAX 7

extern s32 D_801227F0;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern FieldGameState* g_field_game_state;
extern FieldRuntimeContext* g_field_runtime;
extern FieldItemStaging* D_80123FC4;
extern FieldItemTables* D_80123FC0;

FieldItemRecord* field_find_free_inventory_record(void);
s32* func_800C1EC8(s32* src, s32* dest, s32 size);
void* func_800C1E40(s32 table_id);

static void field_create_instrument_item(FieldItemRecord* record, s32 category, s32 item_type, s32 row, s32 command_index);
static void field_generate_staged_item(void);
static void field_load_staged_subtype(void);

/**
 * @brief Create or temper an item from the queued gosub results.
 *
 * FIELD_ITEM_CATEGORY_INSTRUMENT creates an instrument,
 * FIELD_CREATE_KIND_TEMPER tempers the inventory item named by the first
 * result, any other kind creates a weapon or armor of that category. The
 * inventory index, FIELD_ITEM_RESULT_FULL or FIELD_ITEM_RESULT_NONE is
 * written to script variable FIELD_ITEM_RESULT_VARIABLE.
 *
 * @param kind Item category of a new item, or FIELD_CREATE_KIND_TEMPER.
 */
void field_create_item_from_gosub(s32 kind)
{
    FieldItemRecord* record;

    D_801227F0 = 0;
    func_800C1EC8(NULL, (s32*)D_80123FC4, sizeof(FieldItemStaging));

    if (g_gosub_result_count != 0)
    {
        switch (kind)
        {
        case FIELD_ITEM_CATEGORY_INSTRUMENT:
            record = field_find_free_inventory_record();
            if (record != NULL)
            {
                field_create_instrument_item(record, FIELD_ITEM_CATEGORY_INSTRUMENT, g_gosub_result_values[0], g_gosub_result_values[1],
                                             g_gosub_result_values[2]);
                field_set_script_var(0, FIELD_ITEM_RESULT_VARIABLE, record - g_field_game_state->items);
            }
            else
            {
                field_set_script_var(0, FIELD_ITEM_RESULT_VARIABLE, FIELD_ITEM_RESULT_FULL);
            }
            break;
        case FIELD_CREATE_KIND_TEMPER:
            field_temper_item(&g_field_game_state->items[g_gosub_result_values[0]], g_gosub_result_values[1]);
            field_set_script_var(0, FIELD_ITEM_RESULT_VARIABLE, g_gosub_result_values[0]);
            break;
        default:
            record = field_find_free_inventory_record();
            if (record != NULL)
            {
                field_create_equipment_item(record, kind, g_gosub_result_values[0], g_gosub_result_values[1]);
                field_set_script_var(0, FIELD_ITEM_RESULT_VARIABLE, record - g_field_game_state->items);
            }
            else
            {
                field_set_script_var(0, FIELD_ITEM_RESULT_VARIABLE, FIELD_ITEM_RESULT_FULL);
            }
            break;
        }
    }
    else
    {
        field_set_script_var(0, FIELD_ITEM_RESULT_VARIABLE, FIELD_ITEM_RESULT_NONE);
    }
}

/**
 * @brief Create a new weapon or armor, consuming one of its material item, and generate it.
 * @param record Item record that receives the item.
 * @param category FIELD_ITEM_CATEGORY_WEAPON or FIELD_ITEM_CATEGORY_ARMOR.
 * @param item_type Item type.
 * @param item_subtype Item subtype; also the item kind of the material consumed.
 */
void field_create_equipment_item(FieldItemRecord* record, s32 category, s32 item_type, s32 item_subtype)
{
    s32 i;

    func_800C21C0(item_subtype);

    D_80123FC4->record = record;
    D_80123FC4->category = category;
    D_80123FC4->item_type = item_type;
    D_80123FC4->item_subtype = item_subtype;
    D_80123FC4->command_index = FIELD_ITEM_COMMAND_NONE;

    for (i = 0; i < FIELD_STAGING_STAT_COUNT; i++)
    {
        D_80123FC4->stats.bytes[i] = (D_80123FC4->stats.bytes[i] & FIELD_STAGING_BOUNDS_MASK) | FIELD_DEFAULT_STAT_MODIFIER;
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

    field_generate_staged_item();
}

/**
 * @brief Create a new instrument, consuming its two ingredient items, and store its grid values.
 * @param record Item record that receives the item.
 * @param category Item category (FIELD_ITEM_CATEGORY_INSTRUMENT).
 * @param item_type Item type.
 * @param row Row of the grid table pairs; also the item subtype and the first ingredient's item kind.
 * @param command_index Item kind of the second ingredient; selects the command script.
 */
static void field_create_instrument_item(FieldItemRecord* record, s32 category, s32 item_type, s32 row, s32 command_index)
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
        D_80123FC4->stats.bytes[i] = (D_80123FC4->stats.bytes[i] & FIELD_STAGING_BOUNDS_MASK) | FIELD_DEFAULT_STAT_MODIFIER;
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
 * @brief Temper an existing weapon or armor with one more item and regenerate it.
 * @param record Item record to temper.
 * @param command_index Item kind of the tempering item; selects the command entry.
 */
void field_temper_item(FieldItemRecord* record, s32 command_index)
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
    for (i = 0; i < FIELD_ITEM_SPECIAL_COUNT - 1; i++)
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

    field_generate_staged_item();
}

/**
 * @brief Run the generation scripts of the staged item and write it back.
 *
 * The type, subtype, command and slot scripts run on the event script
 * context; pending levels, flags and stat clamping are applied, the item is
 * written back and its weapon or armor derived values are computed.
 */
static void field_generate_staged_item(void)
{
    FieldScriptContext* saved_script;

    saved_script = g_field_script;
    g_field_script = (FieldScriptContext*)&g_field_runtime->events[0].script;
    field_load_staged_subtype();
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
static void field_load_staged_subtype(void)
{
    s32 i;

    D_80123FC0 = func_800C1E40(FIELD_ITEM_TABLE);
    D_80123FC4->unk3A = D_80123FC0->item.subtypes[D_80123FC4->item_subtype].divisor;
    for (i = 0; i < FIELD_STAGING_FACTOR_COUNT; i++)
    {
        D_80123FC4->weights[i] = D_80123FC0->item.subtypes[D_80123FC4->item_subtype].weights[i];
    }
    for (i = 0; i < FIELD_STAGING_FACTOR_COUNT; i++)
    {
        D_80123FC4->multipliers[i] = D_80123FC0->item.subtypes[D_80123FC4->item_subtype].multipliers[i];
    }
    for (i = 0; i < FIELD_STAGING_LEVEL_COUNT; i++)
    {
        D_80123FC4->levels[i].cost = D_80123FC0->item.subtypes[D_80123FC4->item_subtype].costs[i];
        D_80123FC4->pending_levels[i] = 0;
    }

    /* Slots 0 and 5 never hold a slot value. */
    D_80123FC4->flags.bits.slot_class = FIELD_SLOT_CLASS_NONE;
    for (i = 1; i < FIELD_STAGING_SLOT_COUNT - 1; i++)
    {
        if (D_80123FC4->slots[i] < FIELD_SLOT_CLASS_LIMIT)
        {
            D_80123FC4->flags.bits.slot_class = D_80123FC4->slots[i];
        }
    }
}
