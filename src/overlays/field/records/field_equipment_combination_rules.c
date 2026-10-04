#include "common.h"
#include "common/saved_game.h"
#include "common/golem_shape.h"
#include "overlays/field/field_equipment_combination_rules.h"
#include "../internal/field_records.h"

/**
 * @file field_equipment_combination_rules.c
 * @brief Golem logic-block recipes made from two inventory equipment records.
 *
 * The first matching recipe supplies the logic-block id. Equipment power
 * determines its level step, scaled by the recipe, while the sum of the
 * equipment materials selects its shape. GOSUB previews the block and
 * consumes both equipment records when the player confirms the recipe.
 *
 * Recipe classes combine an equipment category and its item type: weapons
 * use types 0-10, armor adds 11, and instruments add 23.
 */

/* Equipment class ids: weapons 0-10, armor 11-22, instruments 23 and up. */
#define EQUIP_CLASS_ARMOR_BASE 11
#define EQUIP_CLASS_INSTRUMENT_BASE 23
#define EQUIP_CLASS_WEAPON(item_type) (item_type)
#define EQUIP_CLASS_ARMOR(item_type) ((item_type) + EQUIP_CLASS_ARMOR_BASE)
#define EQUIP_CLASS_INSTRUMENT(item_type) ((item_type) + EQUIP_CLASS_INSTRUMENT_BASE)

/** @brief Number of inventory records a combination takes. */
#define EQUIPMENT_PAIR_SIZE 2
/** @brief Number of entries in g_golem_logic_block_recipe_table. */
#define GOLEM_LOGIC_BLOCK_RECIPE_COUNT 64
/** @brief Largest logic-block level step the equipment pair can reach. */
#define GOLEM_LOGIC_BLOCK_LEVEL_STEP_MAX 9
/** @brief Equipment power required for each logic-block level step. */
#define GOLEM_LOGIC_BLOCK_POWER_PER_LEVEL 17
/** @brief Number of armor defense values contributing to a combination. */
#define EQUIPMENT_COMBINATION_DEFENSE_COUNT 4

/** @brief Recipe level scale: zero hides the level, one keeps the equipment power step. */
extern u8 g_golem_logic_block_level_scale[];
/** @brief One test per golem logic-block recipe; the index is the block id. */
extern s32 (*g_golem_logic_block_recipe_table[])(s32*);

static s32 golem_equipment_pair_has_classes(s32 class_a, s32 class_b, s32* record_indices);
static s32 golem_logic_block_shape_from_equipment(s32* record_indices);
static s32 golem_logic_block_level_step(s32* record_indices);

/**
 * @brief Calculate the equipment power step for a golem logic block.
 *
 * Adds up the derived values of both records (a weapon's power, all four
 * armor defense values, or an instrument's power), divides by 17
 * and clamps the result to 0 through GOLEM_LOGIC_BLOCK_LEVEL_STEP_MAX.
 *
 * @param record_indices Two indices into the inventory.
 * @return Level step from 0 through 9, before the recipe scale is applied.
 */
static s32 golem_logic_block_level_step(s32* record_indices)
{
    s32* record_end;
    s32 total_power;
    SavedGameLayout* state;
    FieldItemRecord* items;
    s32 item_index;
    s16 category;
    u32 value;
    s32 defense_index;
    s32 level;
    s32 armor;

    total_power = 0;
    state = &g_saved_game.layout;
    items = state->items;
    armor = FIELD_ITEM_CATEGORY_ARMOR;
    record_end = record_indices + EQUIPMENT_PAIR_SIZE;
    do
    {
        item_index = *record_indices;
        value = (state->items + item_index)->info.word;
        value >>= 8;
        category = value & 3;
        if (category == FIELD_ITEM_CATEGORY_WEAPON)
        {
            total_power += items[item_index].derived.weapon.power;
        }
        else if (category == armor)
        {
            for (defense_index = 0; defense_index < EQUIPMENT_COMBINATION_DEFENSE_COUNT; defense_index++)
            {
                total_power += items[item_index].derived.values[defense_index];
            }
        }
        else
        {
            value = FIELD_ITEM_CATEGORY_INSTRUMENT;
            if (category == value)
            {
                total_power += items[item_index].derived.instrument.power;
            }
        }
        record_indices++;
    } while ((intptr_t)record_indices < (intptr_t)record_end);

    total_power /= GOLEM_LOGIC_BLOCK_POWER_PER_LEVEL;
    if (total_power >= 0)
    {
        level = GOLEM_LOGIC_BLOCK_LEVEL_STEP_MAX;
        if (total_power <= GOLEM_LOGIC_BLOCK_LEVEL_STEP_MAX)
        {
            level = total_power;
        }
    }
    else
    {
        level = 0;
    }

    return level;
}

/**
 * @brief Test whether two inventory records supply one item of each requested class.
 * @param class_a Class required of one record.
 * @param class_b Class required of the other record.
 * @param record_indices Two indices into the inventory.
 * @return 1 when distinct pair positions supply the requested classes, otherwise 0.
 * @note The inventory indices themselves are not checked for equality.
 */
static s32 golem_equipment_pair_has_classes(s32 class_a, s32 class_b, s32* record_indices)
{
    SavedGameLayout* state;
    s32 classes[EQUIPMENT_PAIR_SIZE];
    s32 item_type;
    s32 i;
    s32 j;

    for (i = 0; i < EQUIPMENT_PAIR_SIZE; i++)
    {
        state = &g_saved_game.layout;
        item_type = FIELD_ITEM_TYPE((state->items + record_indices[i])->info.word);
        classes[i] = item_type;
        if (FIELD_ITEM_CATEGORY((state->items + record_indices[i])->info.word) == FIELD_ITEM_CATEGORY_ARMOR)
        {
            classes[i] = EQUIP_CLASS_ARMOR(item_type);
        }
        if (FIELD_ITEM_CATEGORY((state->items + record_indices[i])->info.word) == FIELD_ITEM_CATEGORY_INSTRUMENT)
        {
            classes[i] += EQUIP_CLASS_INSTRUMENT_BASE;
        }
    }
    for (i = 0; i < EQUIPMENT_PAIR_SIZE; i++)
    {
        if (classes[i] == class_a)
        {
            for (j = 0; j < EQUIPMENT_PAIR_SIZE; j++)
            {
                if (classes[j] == class_b && j != i)
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/**
 * @brief Find the first golem logic-block recipe the equipment pair satisfies.
 * @param record_indices Two indices into the inventory, tested by each rule.
 * @param level Receives the block level, scaled by the recipe, when a recipe matches.
 * @param shape Receives the block shape index, clamped to 0 through 10, when a recipe matches.
 * @return Logic-block id of the matching recipe, or 0 when no recipe matches.
 */
s32 golem_find_logic_block_recipe(s32* record_indices, s32* level, s32* shape)
{
    s32 rule_index;
    s32 shape_index;
    s32 clamped_shape;

    for (rule_index = 0; rule_index < GOLEM_LOGIC_BLOCK_RECIPE_COUNT; rule_index++)
    {
        if (g_golem_logic_block_recipe_table[rule_index](record_indices) != 0)
        {
            shape_index = golem_logic_block_shape_from_equipment(record_indices);
            *shape = shape_index;
            if (shape_index >= 0)
            {
                clamped_shape = GOLEM_SHAPE_COUNT - 1;
                if (shape_index < GOLEM_SHAPE_COUNT)
                {
                    clamped_shape = shape_index;
                }
            }
            else
            {
                clamped_shape = 0;
            }
            *shape = clamped_shape;
            *level = golem_logic_block_level_step(record_indices) * g_golem_logic_block_level_scale[rule_index];
            return rule_index;
        }
    }
    return 0;
}

/**
 * @brief Select a golem logic-block shape from two equipment materials.
 * @param record_indices Two indices into the inventory.
 * @return Sum of the material indices modulo GOLEM_SHAPE_COUNT (0 through 10).
 */
static s32 golem_logic_block_shape_from_equipment(s32* record_indices)
{
    SavedGameLayout* state;
    FieldItemRecord* first;
    FieldItemRecord* second;

    state = &g_saved_game.layout;
    first = &state->items[record_indices[0]];
    second = &state->items[record_indices[1]];
    return ((first->info.halves[1] & FIELD_ITEM_MATERIAL_MASK) + (second->info.halves[1] & FIELD_ITEM_MATERIAL_MASK)) %
           GOLEM_SHAPE_COUNT;
}

/**
 * @brief Combination rule 0: no pair matches this rule.
 * @return Always 0.
 */
s32 equipment_combination_rule_00(void)
{
    return 0;
}

/**
 * @brief Combination rule 1: no pair matches this rule.
 * @return Always 0.
 */
s32 equipment_combination_rule_01(void)
{
    return 0;
}

/**
 * @brief Combination rule 2: no pair matches this rule.
 * @return Always 0.
 */
s32 equipment_combination_rule_02(void)
{
    return 0;
}

/**
 * @brief Combination rule 3: no pair matches this rule.
 * @return Always 0.
 */
s32 equipment_combination_rule_03(void)
{
    return 0;
}

/**
 * @brief Combination rule 4: armor 8 with armor 7.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_04(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(8), EQUIP_CLASS_ARMOR(7), record_indices) > 0;
}

/**
 * @brief Combination rule 5: no pair matches this rule.
 * @return Always 0.
 */
s32 equipment_combination_rule_05(void)
{
    return 0;
}

/**
 * @brief Combination rule 6: armor 2 with armor 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_06(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(2), EQUIP_CLASS_ARMOR(2), record_indices) > 0;
}

/**
 * @brief Combination rule 7: armor 2 with armor 1.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_07(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(2), EQUIP_CLASS_ARMOR(1), record_indices) > 0;
}

/**
 * @brief Combination rule 8: any of instrument 0 with armor 7, instrument 0 with armor 8.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_08(s32* record_indices)
{
    return (golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_ARMOR(7), record_indices) +
            golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_ARMOR(8), record_indices)) > 0;
}

/**
 * @brief Combination rule 9: any of armor 4 with armor 4, armor 10 with armor 4, armor 10 with armor 10.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_09(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(4), EQUIP_CLASS_ARMOR(4), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_ARMOR(4), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_ARMOR(10), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 10: any of armor 7 with armor 4, armor 8 with armor 4, armor 10 with armor 7, armor 10 with armor 8.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_10(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(7), EQUIP_CLASS_ARMOR(4), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(8), EQUIP_CLASS_ARMOR(4), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_ARMOR(7), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_ARMOR(8), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 11: any of weapon 1 with weapon 1, weapon 2 with weapon 1, weapon 2 with weapon 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_11(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(1), EQUIP_CLASS_WEAPON(1), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(2), EQUIP_CLASS_WEAPON(1), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(2), EQUIP_CLASS_WEAPON(2), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 12: any of weapon 1 with weapon 0, weapon 2 with weapon 0, weapon 6 with weapon 1, weapon 7 with weapon 1, weapon 6 with weapon 2,
 * weapon 7 with weapon 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_12(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(1), EQUIP_CLASS_WEAPON(0), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(2), EQUIP_CLASS_WEAPON(0), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(6), EQUIP_CLASS_WEAPON(1), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(7), EQUIP_CLASS_WEAPON(1), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(6), EQUIP_CLASS_WEAPON(2), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(7), EQUIP_CLASS_WEAPON(2), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 13: any of instrument 3 with weapon 1, instrument 3 with weapon 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_13(s32* record_indices)
{
    return (golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_WEAPON(1), record_indices) +
            golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_WEAPON(2), record_indices)) > 0;
}

/**
 * @brief Combination rule 14: any of weapon 3 with weapon 3, weapon 4 with weapon 3, weapon 4 with weapon 4.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_14(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(3), EQUIP_CLASS_WEAPON(3), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(4), EQUIP_CLASS_WEAPON(3), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(4), EQUIP_CLASS_WEAPON(4), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 15: any of weapon 3 with weapon 1, weapon 4 with weapon 1, weapon 3 with weapon 2, weapon 4 with weapon 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_15(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(3), EQUIP_CLASS_WEAPON(1), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(4), EQUIP_CLASS_WEAPON(1), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(3), EQUIP_CLASS_WEAPON(2), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(4), EQUIP_CLASS_WEAPON(2), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 16: any of armor 4 with weapon 3, armor 10 with weapon 3, armor 4 with weapon 4, armor 10 with weapon 4.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_16(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(4), EQUIP_CLASS_WEAPON(3), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_WEAPON(3), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(4), EQUIP_CLASS_WEAPON(4), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_WEAPON(4), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 17: any of weapon 5 with weapon 5, weapon 9 with weapon 5, weapon 9 with weapon 9.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_17(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(5), EQUIP_CLASS_WEAPON(5), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(9), EQUIP_CLASS_WEAPON(5), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(9), EQUIP_CLASS_WEAPON(9), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 18: any of weapon 8 with weapon 5, weapon 9 with weapon 8.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_18(s32* record_indices)
{
    return (golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(8), EQUIP_CLASS_WEAPON(5), record_indices) +
            golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(9), EQUIP_CLASS_WEAPON(8), record_indices)) > 0;
}

/**
 * @brief Combination rule 19: any of armor 1 with weapon 5, armor 2 with weapon 5, armor 1 with weapon 9, armor 2 with weapon 9.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_19(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(1), EQUIP_CLASS_WEAPON(5), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(2), EQUIP_CLASS_WEAPON(5), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(1), EQUIP_CLASS_WEAPON(9), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(2), EQUIP_CLASS_WEAPON(9), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 20: any of armor 4 with weapon 5, armor 10 with weapon 5, armor 4 with weapon 9, armor 10 with weapon 9.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_20(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(4), EQUIP_CLASS_WEAPON(5), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_WEAPON(5), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(4), EQUIP_CLASS_WEAPON(9), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_WEAPON(9), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 21: any of weapon 0 with weapon 0, weapon 6 with weapon 6, weapon 7 with weapon 6, weapon 7 with weapon 7.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_21(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(0), EQUIP_CLASS_WEAPON(0), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(6), EQUIP_CLASS_WEAPON(6), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(7), EQUIP_CLASS_WEAPON(6), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(7), EQUIP_CLASS_WEAPON(7), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 22: any of weapon 8 with weapon 0, weapon 8 with weapon 6, weapon 8 with weapon 7.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_22(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(8), EQUIP_CLASS_WEAPON(0), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(8), EQUIP_CLASS_WEAPON(6), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(8), EQUIP_CLASS_WEAPON(7), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 23: any of instrument 0 with weapon 0, instrument 0 with weapon 6, instrument 0 with weapon 7.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_23(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_WEAPON(0), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_WEAPON(6), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_WEAPON(7), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 24: weapon 8 with weapon 8.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_24(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(8), EQUIP_CLASS_WEAPON(8), record_indices) > 0;
}

/**
 * @brief Combination rule 25: any of armor 3 with weapon 8, armor 9 with weapon 8.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_25(s32* record_indices)
{
    return (golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(3), EQUIP_CLASS_WEAPON(8), record_indices) +
            golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_WEAPON(8), record_indices)) > 0;
}

/**
 * @brief Combination rule 26: instrument 0 with weapon 8.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_26(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_WEAPON(8), record_indices) > 0;
}

/**
 * @brief Combination rule 27: weapon 10 with weapon 10.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_27(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_WEAPON(10), EQUIP_CLASS_WEAPON(10), record_indices) > 0;
}

/**
 * @brief Combination rule 28: instrument 3 with weapon 10.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_28(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_WEAPON(10), record_indices) > 0;
}

/**
 * @brief Combination rule 29: any of armor 0 with weapon 10, armor 5 with weapon 10.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_29(s32* record_indices)
{
    return (golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(0), EQUIP_CLASS_WEAPON(10), record_indices) +
            golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(5), EQUIP_CLASS_WEAPON(10), record_indices)) > 0;
}

/**
 * @brief Combination rule 30: any of armor 3 with armor 3, armor 9 with armor 3, armor 9 with armor 9.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_30(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(3), EQUIP_CLASS_ARMOR(3), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_ARMOR(3), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_ARMOR(9), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 31: any of instrument 3 with armor 3, instrument 3 with armor 9.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_31(s32* record_indices)
{
    return (golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_ARMOR(3), record_indices) +
            golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_ARMOR(9), record_indices)) > 0;
}

/**
 * @brief Combination rule 32: any of armor 3 with armor 1, armor 9 with armor 1, armor 3 with armor 2, armor 9 with armor 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_32(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(3), EQUIP_CLASS_ARMOR(1), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_ARMOR(1), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(3), EQUIP_CLASS_ARMOR(2), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_ARMOR(2), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 33: any of armor 6 with weapon 5, armor 11 with weapon 5, armor 6 with weapon 9, armor 11 with weapon 9.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_33(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(6), EQUIP_CLASS_WEAPON(5), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(11), EQUIP_CLASS_WEAPON(5), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(6), EQUIP_CLASS_WEAPON(9), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(11), EQUIP_CLASS_WEAPON(9), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 34: never matches.
 * @return Always 0.
 */
s32 equipment_combination_rule_34(void)
{
    return 0;
}

/**
 * @brief Combination rule 35: instrument 0 with instrument 0.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_35(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_INSTRUMENT(0), record_indices) > 0;
}

/**
 * @brief Combination rule 36: instrument 1 with instrument 0.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_36(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(1), EQUIP_CLASS_INSTRUMENT(0), record_indices) > 0;
}

/**
 * @brief Combination rule 37: any of instrument 0 with armor 1, instrument 0 with armor 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_37(s32* record_indices)
{
    return (golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_ARMOR(1), record_indices) +
            golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_ARMOR(2), record_indices)) > 0;
}

/**
 * @brief Combination rule 38: instrument 2 with instrument 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_38(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(2), EQUIP_CLASS_INSTRUMENT(2), record_indices) > 0;
}

/**
 * @brief Combination rule 39: instrument 2 with weapon 10.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_39(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(2), EQUIP_CLASS_WEAPON(10), record_indices) > 0;
}

/**
 * @brief Combination rule 40: any of instrument 3 with armor 4, instrument 2 with instrument 1.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_40(s32* record_indices)
{
    return (golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_ARMOR(4), record_indices) +
            golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(2), EQUIP_CLASS_INSTRUMENT(1), record_indices)) > 0;
}

/**
 * @brief Combination rule 41: any of armor 7 with armor 3, armor 8 with armor 3, armor 9 with armor 7, armor 9 with armor 8.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_41(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(7), EQUIP_CLASS_ARMOR(3), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(8), EQUIP_CLASS_ARMOR(3), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_ARMOR(7), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_ARMOR(8), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 42: any of instrument 1 with armor 1, instrument 1 with armor 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_42(s32* record_indices)
{
    return (golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(1), EQUIP_CLASS_ARMOR(1), record_indices) +
            golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(1), EQUIP_CLASS_ARMOR(2), record_indices)) > 0;
}

/**
 * @brief Combination rule 43: instrument 3 with armor 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_43(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_ARMOR(2), record_indices) > 0;
}

/**
 * @brief Combination rule 44: instrument 3 with instrument 3.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_44(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_INSTRUMENT(3), record_indices) > 0;
}

/**
 * @brief Combination rule 45: instrument 1 with instrument 1.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_45(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(1), EQUIP_CLASS_INSTRUMENT(1), record_indices) > 0;
}

/**
 * @brief Combination rule 46: instrument 3 with instrument 1.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_46(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_INSTRUMENT(1), record_indices) > 0;
}

/**
 * @brief Combination rule 47: any of instrument 2 with armor 1, instrument 2 with armor 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_47(s32* record_indices)
{
    return (golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(2), EQUIP_CLASS_ARMOR(1), record_indices) +
            golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(2), EQUIP_CLASS_ARMOR(2), record_indices)) > 0;
}

/**
 * @brief Combination rule 48: instrument 1 with armor 9.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_48(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(1), EQUIP_CLASS_ARMOR(9), record_indices) > 0;
}

/**
 * @brief Combination rule 49: instrument 2 with armor 9.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_49(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(2), EQUIP_CLASS_ARMOR(9), record_indices) > 0;
}

/**
 * @brief Combination rule 50: armor 1 with armor 1.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_50(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(1), EQUIP_CLASS_ARMOR(1), record_indices) > 0;
}

/**
 * @brief Combination rule 51: any of armor 1 with armor 0, armor 2 with armor 0, armor 5 with armor 0, armor 5 with armor 1, armor 5 with armor 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_51(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(1), EQUIP_CLASS_ARMOR(0), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(2), EQUIP_CLASS_ARMOR(0), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(5), EQUIP_CLASS_ARMOR(0), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(5), EQUIP_CLASS_ARMOR(1), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(5), EQUIP_CLASS_ARMOR(2), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 52: any of instrument 1 with armor 0, instrument 1 with armor 5.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_52(s32* record_indices)
{
    return (golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(1), EQUIP_CLASS_ARMOR(0), record_indices) +
            golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(1), EQUIP_CLASS_ARMOR(5), record_indices)) > 0;
}

/**
 * @brief Combination rule 53: instrument 3 with instrument 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_53(s32* record_indices)
{
    return golem_equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_INSTRUMENT(2), record_indices) > 0;
}

/**
 * @brief Combination rule 54: any of armor 3 with armor 0, armor 9 with armor 0, armor 5 with armor 3, armor 9 with armor 5.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_54(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(3), EQUIP_CLASS_ARMOR(0), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_ARMOR(0), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(5), EQUIP_CLASS_ARMOR(3), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_ARMOR(5), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 55: any of armor 6 with armor 6, armor 11 with armor 6, armor 11 with armor 11.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_55(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(6), EQUIP_CLASS_ARMOR(6), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(11), EQUIP_CLASS_ARMOR(6), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(11), EQUIP_CLASS_ARMOR(11), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 56: any of armor 6 with weapon 10, armor 11 with weapon 10.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_56(s32* record_indices)
{
    return (golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(6), EQUIP_CLASS_WEAPON(10), record_indices) +
            golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(11), EQUIP_CLASS_WEAPON(10), record_indices)) > 0;
}

/**
 * @brief Combination rule 57: any of armor 6 with armor 4, armor 11 with armor 4, armor 10 with armor 6, armor 11 with armor 10.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_57(s32* record_indices)
{
    s32 sum;

    sum = 0;
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(6), EQUIP_CLASS_ARMOR(4), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(11), EQUIP_CLASS_ARMOR(4), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_ARMOR(6), record_indices);
    sum += golem_equipment_pair_has_classes(EQUIP_CLASS_ARMOR(11), EQUIP_CLASS_ARMOR(10), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 58: no pair matches this rule.
 * @return Always 0.
 */
s32 equipment_combination_rule_58(void)
{
    return 0;
}

/**
 * @brief Combination rule 59: no pair matches this rule.
 * @return Always 0.
 */
s32 equipment_combination_rule_59(void)
{
    return 0;
}

/**
 * @brief Combination rule 60: no pair matches this rule.
 * @return Always 0.
 */
s32 equipment_combination_rule_60(void)
{
    return 0;
}

/**
 * @brief Combination rule 61: no pair matches this rule.
 * @return Always 0.
 */
s32 equipment_combination_rule_61(void)
{
    return 0;
}

/**
 * @brief Combination rule 62: no pair matches this rule.
 * @return Always 0.
 */
s32 equipment_combination_rule_62(void)
{
    return 0;
}

/**
 * @brief Combination rule 63: no pair matches this rule.
 * @return Always 0.
 */
s32 equipment_combination_rule_63(void)
{
    return 0;
}
