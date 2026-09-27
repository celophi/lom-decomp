/**
 * @file field_record_setup_ops.c
 * @brief Forge, workshop and tempering: fill the staging block for a new or
 *        existing item, run the generation scripts and write the result back.
 *
 * The GOSUB selection screens queue their choices in g_gosub_result_values:
 * a weapon or armor is made from an item type and a material, an instrument
 * from an instrument type, a material and a secondary material, and tempering
 * takes an inventory index and a tempering item.
 */

#include "common.h"
#include "field_calls.h"
#include "field_records.h"
#include "field_script.h"

/** @brief field_find_resource id of the weapon and armor generation table. */
#define FIELD_ITEM_TABLE 4

/** @brief field_find_resource id of the instrument generation table. */
#define FIELD_ITEM_GRID_TABLE 0xF

/** @brief Script variable that receives the created item's inventory index. */
#define FIELD_ITEM_RESULT_VARIABLE 0x7100

/** @brief Inventory index reported when the inventory is full. */
#define FIELD_ITEM_RESULT_FULL 0xFE

/** @brief Inventory index reported when the GOSUB screens were cancelled. */
#define FIELD_ITEM_RESULT_NONE 0xFF

/** @brief field_create_item_from_gosub kind that tempers an existing inventory item. */
#define FIELD_CREATE_KIND_TEMPER 3

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

extern s32 g_field_gosub_state;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern FieldGameState* g_field_game_state;
extern FieldRuntimeContext* g_field_runtime;

FieldItemRecord* field_find_free_inventory_record(void);

static void field_create_instrument_item(FieldItemRecord* record, s32 category, s32 item_type, s32 material, s32 secondary_item);
static void field_generate_staged_item(void);
static void field_load_staged_material(void);

/**
 * @brief Create or temper an item from the queued GOSUB results.
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

    g_field_gosub_state = 0;
    field_copy_words(NULL, g_field_item_staging, sizeof(FieldItemStaging));

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
 * @brief Create a new weapon or armor, consuming one of its material, and generate it.
 * @param record Item record that receives the item.
 * @param category FIELD_ITEM_CATEGORY_WEAPON or FIELD_ITEM_CATEGORY_ARMOR.
 * @param item_type Weapon or armor type.
 * @param material Material item kind; one is consumed.
 */
void field_create_equipment_item(FieldItemRecord* record, s32 category, s32 item_type, s32 material)
{
    s32 i;

    field_consume_item(material);

    g_field_item_staging->record = record;
    g_field_item_staging->category = category;
    g_field_item_staging->item_type = item_type;
    g_field_item_staging->material = material;
    g_field_item_staging->secondary_item = FIELD_NO_SECONDARY_ITEM;

    for (i = 0; i < FIELD_STAGING_STAT_COUNT; i++)
    {
        g_field_item_staging->stats.bytes[i] = (g_field_item_staging->stats.bytes[i] & FIELD_STAGING_BOUNDS_MASK) | FIELD_DEFAULT_STAT_MODIFIER;
        g_field_item_staging->base_stats[i] = FIELD_DEFAULT_STAT_MODIFIER;
    }

    for (i = 0; i < FIELD_STAGING_SLOT_COUNT; i++)
    {
        g_field_item_staging->slots[i] = FIELD_STAGING_SLOT_EMPTY;
    }

    g_field_item_staging->properties[0] = g_field_item_staging->item_type << 4;
    g_field_item_staging->properties[1] = (g_field_item_staging->item_type << 4) + 0xC;
    g_field_item_staging->properties[2] = (g_field_item_staging->item_type << 4) + 0xD;
    g_field_item_staging->properties[3] = (g_field_item_staging->item_type << 4) + 0xE;
    g_field_item_staging->properties[4] = (g_field_item_staging->item_type << 4) + 0xF;
    g_field_item_staging->properties[5] = 0xFF;

    field_generate_staged_item();
}

/**
 * @brief Create a new instrument from a material and a secondary material, and pick its spell.
 *
 * The material and instrument type give the start cell and power; the
 * secondary material's script moves the cell, which is clamped to the grid
 * and selects the spell.
 *
 * @param record Item record that receives the item.
 * @param category Item category (FIELD_ITEM_CATEGORY_INSTRUMENT).
 * @param item_type Instrument type.
 * @param material Material item kind; one is consumed.
 * @param secondary_item Secondary material item kind; one is consumed.
 */
static void field_create_instrument_item(FieldItemRecord* record, s32 category, s32 item_type, s32 material, s32 secondary_item)
{
    FieldScriptContext* saved_script;
    s32 column;
    s32 grid_row;
    s32 i;

    field_consume_item(material);
    field_consume_item(secondary_item);
    g_field_item_staging->record = record;
    g_field_item_staging->category = category;
    g_field_item_staging->item_type = item_type;
    g_field_item_staging->material = material;
    g_field_item_staging->secondary_item = secondary_item;

    for (i = 0; i < FIELD_STAGING_STAT_COUNT; i++)
    {
        g_field_item_staging->stats.bytes[i] = (g_field_item_staging->stats.bytes[i] & FIELD_STAGING_BOUNDS_MASK) | FIELD_DEFAULT_STAT_MODIFIER;
    }
    for (i = 0; i < FIELD_STAGING_SLOT_COUNT; i++)
    {
        g_field_item_staging->slots[i] = FIELD_STAGING_SLOT_EMPTY;
    }

    g_field_item_tables = field_find_resource(FIELD_ITEM_GRID_TABLE);
    g_field_item_staging->properties[1] = g_field_item_tables->grid.pairs[material][item_type][0] & 7;
    g_field_item_staging->properties[2] = g_field_item_tables->grid.pairs[material][item_type][0] >> 3;
    g_field_item_staging->properties[3] = g_field_item_tables->grid.pairs[material][item_type][1];

    saved_script = g_field_script;
    g_field_script = (FieldScriptContext*)&g_field_runtime->events[0].script;
    field_run_item_script(g_field_item_tables->grid.secondary_scripts[secondary_item - FIELD_SECONDARY_ITEM_FIRST]);
    g_field_script = saved_script;
    field_write_staged_item();

    record->derived.instrument.spirit = g_field_item_staging->properties[0];

    if ((s8)g_field_item_staging->properties[1] >= 0)
    {
        column = FIELD_GRID_MAX;
        if (g_field_item_staging->properties[1] <= FIELD_GRID_MAX)
        {
            column = g_field_item_staging->properties[1];
        }
    }
    else
    {
        column = 0;
    }

    if ((s8)g_field_item_staging->properties[2] >= 0)
    {
        grid_row = FIELD_GRID_MAX;
        if (g_field_item_staging->properties[2] <= FIELD_GRID_MAX)
        {
            grid_row = g_field_item_staging->properties[2];
        }
    }
    else
    {
        grid_row = 0;
    }

    record->derived.instrument.spell = g_field_item_tables->grid.grid[grid_row][column];
    record->derived.instrument.power = g_field_item_staging->properties[3];
}

/**
 * @brief Temper an existing weapon or armor with one more item and regenerate it.
 * @param record Item record to temper.
 * @param secondary_item Tempering item kind; one is consumed.
 * @note JP changes this function; the JP build takes it from assembly.
 */
#if defined(VERSION_JP)
INCLUDE_ASM("overlays/field/nonmatchings/field_record_setup_ops", field_temper_item);
#else
void field_temper_item(FieldItemRecord* record, s32 secondary_item)
{
    s32 i;

    g_field_item_tables = field_find_resource(FIELD_ITEM_TABLE);
    field_consume_item(secondary_item);

    g_field_item_staging->record = record;
    g_field_item_staging->category = record->info.bits.category;
    g_field_item_staging->item_type = record->info.bits.item_type;
    g_field_item_staging->material = record->info.bits.material;
    g_field_item_staging->secondary_item = secondary_item;
    g_field_item_staging->pool += g_field_item_tables->item.secondary_items[secondary_item - FIELD_SECONDARY_ITEM_FIRST].pool_bonus;

    g_field_item_staging->levels[0].level = record->bonus_nibbles.bits.n0;
    g_field_item_staging->levels[1].level = record->bonus_nibbles.bits.n1;
    g_field_item_staging->levels[2].level = record->bonus_nibbles.bits.n2;
    g_field_item_staging->levels[3].level = record->bonus_nibbles.bits.n3;
    g_field_item_staging->levels[4].level = record->bonus_nibbles.bits.n4;
    g_field_item_staging->levels[5].level = record->bonus_nibbles.bits.n5;
    g_field_item_staging->levels[6].level = record->bonus_nibbles.bits.n6;
    g_field_item_staging->levels[7].level = record->bonus_nibbles.bits.n7;
    g_field_item_staging->effect_index = record->effect_index;

    g_field_item_staging->stats.words[0].modifier0 = record->stat_nibbles.bits.n0;
    g_field_item_staging->stats.words[0].modifier1 = record->stat_nibbles.bits.n1;
    g_field_item_staging->stats.words[0].modifier2 = record->stat_nibbles.bits.n2;
    g_field_item_staging->stats.words[0].modifier3 = record->stat_nibbles.bits.n3;
    g_field_item_staging->stats.words[1].modifier0 = record->stat_nibbles.bits.n4;
    g_field_item_staging->stats.words[1].modifier1 = record->stat_nibbles.bits.n5;
    g_field_item_staging->stats.words[1].modifier2 = record->stat_nibbles.bits.n6;
    g_field_item_staging->stats.words[1].modifier3 = record->stat_nibbles.bits.n7;

    for (i = 0; i < FIELD_STAGING_STAT_COUNT; i++)
    {
        g_field_item_staging->base_stats[i] = FIELD_DEFAULT_STAT_MODIFIER;
    }

    g_field_item_staging->slots[0] = FIELD_STAGING_SLOT_EMPTY;
    g_field_item_staging->slots[1] = record->special_ids[3];
    for (i = 0; i < FIELD_ITEM_SPECIAL_COUNT - 1; i++)
    {
        g_field_item_staging->slots[i + 2] = record->special_ids[i];
    }
    g_field_item_staging->slots[5] = FIELD_STAGING_SLOT_EMPTY;

    switch (g_field_item_staging->category)
    {
    case FIELD_ITEM_CATEGORY_WEAPON:
        for (i = 0; i < FIELD_STAGING_PROPERTY_COUNT; i++)
        {
            g_field_item_staging->properties[i] = record->derived.weapon.stats[i];
        }
        g_field_item_staging->power_flags = 0;
        break;
    case FIELD_ITEM_CATEGORY_ARMOR:
        g_field_item_staging->element_flags = 0;
        g_field_item_staging->immunity_flags = record->status_flags;
        break;
    }

    field_generate_staged_item();
}
#endif

/**
 * @brief Run the generation scripts of the staged item and write it back.
 *
 * The type, material, secondary material and slot scripts run on the event script
 * context; pending levels, flags and stat clamping are applied, the item is
 * written back and its weapon or armor derived values are computed.
 * @note JP applies the pending levels in field_finish_staged_item instead.
 */
static void field_generate_staged_item(void)
{
    FieldScriptContext* saved_script;

    saved_script = g_field_script;
    g_field_script = (FieldScriptContext*)&g_field_runtime->events[0].script;
    field_load_staged_material();
    if (g_field_item_staging->category == FIELD_ITEM_CATEGORY_WEAPON)
    {
        field_run_item_script(g_field_item_tables->item.weapon_types[g_field_item_staging->item_type].scripts[0]);
    }
    else
    {
        field_run_item_script(g_field_item_tables->item.armor_types[g_field_item_staging->item_type].scripts[0]);
    }
    field_run_item_script(g_field_item_tables->item.materials[g_field_item_staging->material].script);
    field_run_item_script(g_field_item_tables->item.secondary_items[g_field_item_staging->secondary_item - FIELD_SECONDARY_ITEM_FIRST].script);
    field_run_slot_scripts();
#if !defined(VERSION_JP)
    field_apply_pending_levels();
#endif
    if (g_field_item_staging->category == FIELD_ITEM_CATEGORY_WEAPON)
    {
        field_run_item_script(g_field_item_tables->item.weapon_types[g_field_item_staging->item_type].scripts[1]);
    }
    else
    {
        field_run_item_script(g_field_item_tables->item.armor_types[g_field_item_staging->item_type].scripts[1]);
    }
    field_finish_staged_item();
    g_field_script = saved_script;
    field_write_staged_item();

    switch (g_field_item_staging->category)
    {
    case FIELD_ITEM_CATEGORY_WEAPON:
        field_derive_weapon_values(g_field_item_staging->record);
        return;
    case FIELD_ITEM_CATEGORY_ARMOR:
        field_derive_armor_values(g_field_item_staging->record);
        return;
    }
}

/**
 * @brief Load the item table and copy the staged material's entry into the staging block.
 */
static void field_load_staged_material(void)
{
    s32 i;

    g_field_item_tables = field_find_resource(FIELD_ITEM_TABLE);
    g_field_item_staging->divisor = g_field_item_tables->item.materials[g_field_item_staging->material].divisor;
    for (i = 0; i < FIELD_STAGING_FACTOR_COUNT; i++)
    {
        g_field_item_staging->weights[i] = g_field_item_tables->item.materials[g_field_item_staging->material].weights[i];
    }
    for (i = 0; i < FIELD_STAGING_FACTOR_COUNT; i++)
    {
        g_field_item_staging->multipliers[i] = g_field_item_tables->item.materials[g_field_item_staging->material].multipliers[i];
    }
    for (i = 0; i < FIELD_STAGING_LEVEL_COUNT; i++)
    {
        g_field_item_staging->levels[i].cost = g_field_item_tables->item.materials[g_field_item_staging->material].costs[i];
        g_field_item_staging->pending_levels[i] = 0;
    }

    /* Slots 0 and 5 never hold a slot value. */
    g_field_item_staging->flags.bits.slot_class = FIELD_SLOT_CLASS_NONE;
    for (i = 1; i < FIELD_STAGING_SLOT_COUNT - 1; i++)
    {
        if (g_field_item_staging->slots[i] < FIELD_SLOT_CLASS_LIMIT)
        {
            g_field_item_staging->flags.bits.slot_class = g_field_item_staging->slots[i];
        }
    }
}
