#include "common.h"

/*
 * Equipment combination rules.
 *
 * equipment_combination_variant scores a selected pair; every other function
 * here is one entry of the 64-entry g_equipment_combination_rule_table. equipment_combination_find walks that
 * table with the pair of equipment records the player selected and stops at
 * the first rule that returns nonzero; the rule's index is the id of the
 * item the pair combines into.
 *
 * A rule asks equipment_pair_has_classes whether the two records hold one
 * item of class A and one item of class B. A class folds an equipment kind
 * (weapon, armor, instrument) and its category into a single id so a rule
 * can name any item type with one number.
 */

/* Equipment class ids: weapons 0-10, armor 11-22, instruments 23 and up. */
#define EQUIP_CLASS_ARMOR_BASE 11
#define EQUIP_CLASS_INSTRUMENT_BASE 23
#define EQUIP_CLASS_WEAPON(category) (category)
#define EQUIP_CLASS_ARMOR(category) ((category) + EQUIP_CLASS_ARMOR_BASE)
#define EQUIP_CLASS_INSTRUMENT(category) ((category) + EQUIP_CLASS_INSTRUMENT_BASE)

extern u8 g_menuLayoutBuffer[];

/** @brief View of one save-data equipment record exposing its material halfword. */
typedef struct
{
    u8 pad[0xCF6];
    u16 material;
} EquipmentMaterialView;

/**
 * @brief Test whether a pair of equipment records holds one item of each class.
 * @param class_a Equipment class required of one record.
 * @param class_b Equipment class required of the other record.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when one record is class_a and the other is class_b, otherwise 0.
 */
s32 equipment_pair_has_classes(s32 class_a, s32 class_b, s32* record_indices);

/**
 * @brief Derive the variant of the item two equipment records combine into.
 *
 * Sums the low six bits of each record's material field and reduces the
 * total modulo 11.
 *
 * @param record_indices Two indices into the save-data equipment table.
 * @return Variant index in the range 0 through 10.
 */
s32 equipment_combination_variant(s32* record_indices)
{
    u8* base;
    EquipmentMaterialView* first;
    EquipmentMaterialView* second;

    base = g_menuLayoutBuffer;
    first = (EquipmentMaterialView*)&base[record_indices[0] << 6];
    second = (EquipmentMaterialView*)&base[record_indices[1] << 6];
    return ((first->material & 0x3F) + (second->material & 0x3F)) % 11;
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
    return equipment_pair_has_classes(EQUIP_CLASS_ARMOR(8), EQUIP_CLASS_ARMOR(7), record_indices) > 0;
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
    s32 equip_class = EQUIP_CLASS_ARMOR(2);

    return equipment_pair_has_classes(equip_class, equip_class, record_indices) > 0;
}

/**
 * @brief Combination rule 7: armor 2 with armor 1.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_07(s32* record_indices)
{
    return equipment_pair_has_classes(EQUIP_CLASS_ARMOR(2), EQUIP_CLASS_ARMOR(1), record_indices) > 0;
}

/**
 * @brief Combination rule 8: any of instrument 0 with armor 7, instrument 0 with armor 8.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_08(s32* record_indices)
{
    return (equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_ARMOR(7), record_indices) +
            equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_ARMOR(8), record_indices)) > 0;
}

/**
 * @brief Combination rule 9: any of armor 4 with armor 4, armor 10 with armor 4, armor 10 with armor 10.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_09(s32* record_indices)
{
    s32 result;
    s32 first;

    first = equipment_pair_has_classes(EQUIP_CLASS_ARMOR(4), EQUIP_CLASS_ARMOR(4), record_indices);
    result = first + equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_ARMOR(4), record_indices);
    result += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_ARMOR(10), record_indices);
    return result > 0;
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
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(7), EQUIP_CLASS_ARMOR(4), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(8), EQUIP_CLASS_ARMOR(4), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_ARMOR(7), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_ARMOR(8), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 11: any of weapon 1 with weapon 1, weapon 2 with weapon 1, weapon 2 with weapon 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_11(s32* record_indices)
{
    s32 result;
    s32 first;

    first = equipment_pair_has_classes(EQUIP_CLASS_WEAPON(1), EQUIP_CLASS_WEAPON(1), record_indices);
    result = first + equipment_pair_has_classes(EQUIP_CLASS_WEAPON(2), EQUIP_CLASS_WEAPON(1), record_indices);
    result += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(2), EQUIP_CLASS_WEAPON(2), record_indices);
    return result > 0;
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
    sum += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(1), EQUIP_CLASS_WEAPON(0), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(2), EQUIP_CLASS_WEAPON(0), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(6), EQUIP_CLASS_WEAPON(1), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(7), EQUIP_CLASS_WEAPON(1), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(6), EQUIP_CLASS_WEAPON(2), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(7), EQUIP_CLASS_WEAPON(2), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 13: any of instrument 3 with weapon 1, instrument 3 with weapon 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_13(s32* record_indices)
{
    return (equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_WEAPON(1), record_indices) +
            equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_WEAPON(2), record_indices)) > 0;
}

/**
 * @brief Combination rule 14: any of weapon 3 with weapon 3, weapon 4 with weapon 3, weapon 4 with weapon 4.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_14(s32* record_indices)
{
    s32 result;
    s32 first;

    first = equipment_pair_has_classes(EQUIP_CLASS_WEAPON(3), EQUIP_CLASS_WEAPON(3), record_indices);
    result = first + equipment_pair_has_classes(EQUIP_CLASS_WEAPON(4), EQUIP_CLASS_WEAPON(3), record_indices);
    result += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(4), EQUIP_CLASS_WEAPON(4), record_indices);
    return result > 0;
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
    sum += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(3), EQUIP_CLASS_WEAPON(1), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(4), EQUIP_CLASS_WEAPON(1), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(3), EQUIP_CLASS_WEAPON(2), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(4), EQUIP_CLASS_WEAPON(2), record_indices);
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
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(4), EQUIP_CLASS_WEAPON(3), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_WEAPON(3), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(4), EQUIP_CLASS_WEAPON(4), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_WEAPON(4), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 17: any of weapon 5 with weapon 5, weapon 9 with weapon 5, weapon 9 with weapon 9.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_17(s32* record_indices)
{
    s32 result;
    s32 first;

    first = equipment_pair_has_classes(EQUIP_CLASS_WEAPON(5), EQUIP_CLASS_WEAPON(5), record_indices);
    result = first + equipment_pair_has_classes(EQUIP_CLASS_WEAPON(9), EQUIP_CLASS_WEAPON(5), record_indices);
    result += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(9), EQUIP_CLASS_WEAPON(9), record_indices);
    return result > 0;
}

/**
 * @brief Combination rule 18: any of weapon 8 with weapon 5, weapon 9 with weapon 8.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_18(s32* record_indices)
{
    return (equipment_pair_has_classes(EQUIP_CLASS_WEAPON(8), EQUIP_CLASS_WEAPON(5), record_indices) +
            equipment_pair_has_classes(EQUIP_CLASS_WEAPON(9), EQUIP_CLASS_WEAPON(8), record_indices)) > 0;
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
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(1), EQUIP_CLASS_WEAPON(5), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(2), EQUIP_CLASS_WEAPON(5), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(1), EQUIP_CLASS_WEAPON(9), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(2), EQUIP_CLASS_WEAPON(9), record_indices);
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
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(4), EQUIP_CLASS_WEAPON(5), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_WEAPON(5), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(4), EQUIP_CLASS_WEAPON(9), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_WEAPON(9), record_indices);
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
    sum += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(0), EQUIP_CLASS_WEAPON(0), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(6), EQUIP_CLASS_WEAPON(6), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(7), EQUIP_CLASS_WEAPON(6), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(7), EQUIP_CLASS_WEAPON(7), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 22: any of weapon 8 with weapon 0, weapon 8 with weapon 6, weapon 8 with weapon 7.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_22(s32* record_indices)
{
    s32 result;
    s32 first;

    first = equipment_pair_has_classes(EQUIP_CLASS_WEAPON(8), EQUIP_CLASS_WEAPON(0), record_indices);
    result = first + equipment_pair_has_classes(EQUIP_CLASS_WEAPON(8), EQUIP_CLASS_WEAPON(6), record_indices);
    result += equipment_pair_has_classes(EQUIP_CLASS_WEAPON(8), EQUIP_CLASS_WEAPON(7), record_indices);
    return result > 0;
}

/**
 * @brief Combination rule 23: any of instrument 0 with weapon 0, instrument 0 with weapon 6, instrument 0 with weapon 7.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_23(s32* record_indices)
{
    s32 result;
    s32 first;

    first = equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_WEAPON(0), record_indices);
    result = first + equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_WEAPON(6), record_indices);
    result += equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_WEAPON(7), record_indices);
    return result > 0;
}

/**
 * @brief Combination rule 24: weapon 8 with weapon 8.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_24(s32* record_indices)
{
    return equipment_pair_has_classes(EQUIP_CLASS_WEAPON(8), EQUIP_CLASS_WEAPON(8), record_indices) > 0;
}

/**
 * @brief Combination rule 25: any of armor 3 with weapon 8, armor 9 with weapon 8.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_25(s32* record_indices)
{
    return (equipment_pair_has_classes(EQUIP_CLASS_ARMOR(3), EQUIP_CLASS_WEAPON(8), record_indices) +
            equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_WEAPON(8), record_indices)) > 0;
}

/**
 * @brief Combination rule 26: instrument 0 with weapon 8.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_26(s32* record_indices)
{
    return equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_WEAPON(8), record_indices) > 0;
}

/**
 * @brief Combination rule 27: weapon 10 with weapon 10.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_27(s32* record_indices)
{
    s32 equip_class = EQUIP_CLASS_WEAPON(10);

    return equipment_pair_has_classes(equip_class, equip_class, record_indices) > 0;
}

/**
 * @brief Combination rule 28: instrument 3 with weapon 10.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_28(s32* record_indices)
{
    return equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_WEAPON(10), record_indices) > 0;
}

/**
 * @brief Combination rule 29: any of armor 0 with weapon 10, armor 5 with weapon 10.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_29(s32* record_indices)
{
    return (equipment_pair_has_classes(EQUIP_CLASS_ARMOR(0), EQUIP_CLASS_WEAPON(10), record_indices) +
            equipment_pair_has_classes(EQUIP_CLASS_ARMOR(5), EQUIP_CLASS_WEAPON(10), record_indices)) > 0;
}

/**
 * @brief Combination rule 30: any of armor 3 with armor 3, armor 9 with armor 3, armor 9 with armor 9.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_30(s32* record_indices)
{
    s32 result;
    s32 first;

    first = equipment_pair_has_classes(EQUIP_CLASS_ARMOR(3), EQUIP_CLASS_ARMOR(3), record_indices);
    result = first + equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_ARMOR(3), record_indices);
    result += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_ARMOR(9), record_indices);
    return result > 0;
}

/**
 * @brief Combination rule 31: any of instrument 3 with armor 3, instrument 3 with armor 9.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_31(s32* record_indices)
{
    return (equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_ARMOR(3), record_indices) +
            equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_ARMOR(9), record_indices)) > 0;
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
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(3), EQUIP_CLASS_ARMOR(1), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_ARMOR(1), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(3), EQUIP_CLASS_ARMOR(2), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_ARMOR(2), record_indices);
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
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(6), EQUIP_CLASS_WEAPON(5), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(11), EQUIP_CLASS_WEAPON(5), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(6), EQUIP_CLASS_WEAPON(9), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(11), EQUIP_CLASS_WEAPON(9), record_indices);
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
    return equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_INSTRUMENT(0), record_indices) > 0;
}

/**
 * @brief Combination rule 36: instrument 1 with instrument 0.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_36(s32* record_indices)
{
    return equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(1), EQUIP_CLASS_INSTRUMENT(0), record_indices) > 0;
}

/**
 * @brief Combination rule 37: any of instrument 0 with armor 1, instrument 0 with armor 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_37(s32* record_indices)
{
    return (equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_ARMOR(1), record_indices) +
            equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(0), EQUIP_CLASS_ARMOR(2), record_indices)) > 0;
}

/**
 * @brief Combination rule 38: instrument 2 with instrument 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_38(s32* record_indices)
{
    s32 equip_class = EQUIP_CLASS_INSTRUMENT(2);

    return equipment_pair_has_classes(equip_class, equip_class, record_indices) > 0;
}

/**
 * @brief Combination rule 39: instrument 2 with weapon 10.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_39(s32* record_indices)
{
    return equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(2), EQUIP_CLASS_WEAPON(10), record_indices) > 0;
}

/**
 * @brief Combination rule 40: any of instrument 3 with armor 4, instrument 2 with instrument 1.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_40(s32* record_indices)
{
    return (equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_ARMOR(4), record_indices) +
            equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(2), EQUIP_CLASS_INSTRUMENT(1), record_indices)) > 0;
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
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(7), EQUIP_CLASS_ARMOR(3), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(8), EQUIP_CLASS_ARMOR(3), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_ARMOR(7), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_ARMOR(8), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 42: any of instrument 1 with armor 1, instrument 1 with armor 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_42(s32* record_indices)
{
    return (equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(1), EQUIP_CLASS_ARMOR(1), record_indices) +
            equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(1), EQUIP_CLASS_ARMOR(2), record_indices)) > 0;
}

/**
 * @brief Combination rule 43: instrument 3 with armor 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_43(s32* record_indices)
{
    return equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_ARMOR(2), record_indices) > 0;
}

/**
 * @brief Combination rule 44: instrument 3 with instrument 3.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_44(s32* record_indices)
{
    s32 equip_class = EQUIP_CLASS_INSTRUMENT(3);

    return equipment_pair_has_classes(equip_class, equip_class, record_indices) > 0;
}

/**
 * @brief Combination rule 45: instrument 1 with instrument 1.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_45(s32* record_indices)
{
    return equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(1), EQUIP_CLASS_INSTRUMENT(1), record_indices) > 0;
}

/**
 * @brief Combination rule 46: instrument 3 with instrument 1.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_46(s32* record_indices)
{
    return equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_INSTRUMENT(1), record_indices) > 0;
}

/**
 * @brief Combination rule 47: any of instrument 2 with armor 1, instrument 2 with armor 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_47(s32* record_indices)
{
    return (equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(2), EQUIP_CLASS_ARMOR(1), record_indices) +
            equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(2), EQUIP_CLASS_ARMOR(2), record_indices)) > 0;
}

/**
 * @brief Combination rule 48: instrument 1 with armor 9.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_48(s32* record_indices)
{
    return equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(1), EQUIP_CLASS_ARMOR(9), record_indices) > 0;
}

/**
 * @brief Combination rule 49: instrument 2 with armor 9.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_49(s32* record_indices)
{
    return equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(2), EQUIP_CLASS_ARMOR(9), record_indices) > 0;
}

/**
 * @brief Combination rule 50: armor 1 with armor 1.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_50(s32* record_indices)
{
    return equipment_pair_has_classes(EQUIP_CLASS_ARMOR(1), EQUIP_CLASS_ARMOR(1), record_indices) > 0;
}

/**
 * @brief Combination rule 51: any of armor 1 with armor 0, armor 2 with armor 0, armor 5 with armor 0, armor 5 with armor 1, armor 5 with armor 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_51(s32* record_indices)
{
    s32 sum;

    sum = equipment_pair_has_classes(EQUIP_CLASS_ARMOR(1), EQUIP_CLASS_ARMOR(0), record_indices) +
          equipment_pair_has_classes(EQUIP_CLASS_ARMOR(2), EQUIP_CLASS_ARMOR(0), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(5), EQUIP_CLASS_ARMOR(0), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(5), EQUIP_CLASS_ARMOR(1), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(5), EQUIP_CLASS_ARMOR(2), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 52: any of instrument 1 with armor 0, instrument 1 with armor 5.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_52(s32* record_indices)
{
    return (equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(1), EQUIP_CLASS_ARMOR(0), record_indices) +
            equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(1), EQUIP_CLASS_ARMOR(5), record_indices)) > 0;
}

/**
 * @brief Combination rule 53: instrument 3 with instrument 2.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_53(s32* record_indices)
{
    return equipment_pair_has_classes(EQUIP_CLASS_INSTRUMENT(3), EQUIP_CLASS_INSTRUMENT(2), record_indices) > 0;
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
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(3), EQUIP_CLASS_ARMOR(0), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_ARMOR(0), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(5), EQUIP_CLASS_ARMOR(3), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(9), EQUIP_CLASS_ARMOR(5), record_indices);
    return sum > 0;
}

/**
 * @brief Combination rule 55: any of armor 6 with armor 6, armor 11 with armor 6, armor 11 with armor 11.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_55(s32* record_indices)
{
    s32 result;
    s32 first;

    first = equipment_pair_has_classes(EQUIP_CLASS_ARMOR(6), EQUIP_CLASS_ARMOR(6), record_indices);
    result = first + equipment_pair_has_classes(EQUIP_CLASS_ARMOR(11), EQUIP_CLASS_ARMOR(6), record_indices);
    result += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(11), EQUIP_CLASS_ARMOR(11), record_indices);
    return result > 0;
}

/**
 * @brief Combination rule 56: any of armor 6 with weapon 10, armor 11 with weapon 10.
 * @param record_indices Two indices into the save-data equipment table.
 * @return 1 when the pair satisfies the rule, otherwise 0.
 */
s32 equipment_combination_rule_56(s32* record_indices)
{
    return (equipment_pair_has_classes(EQUIP_CLASS_ARMOR(6), EQUIP_CLASS_WEAPON(10), record_indices) +
            equipment_pair_has_classes(EQUIP_CLASS_ARMOR(11), EQUIP_CLASS_WEAPON(10), record_indices)) > 0;
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
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(6), EQUIP_CLASS_ARMOR(4), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(11), EQUIP_CLASS_ARMOR(4), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(10), EQUIP_CLASS_ARMOR(6), record_indices);
    sum += equipment_pair_has_classes(EQUIP_CLASS_ARMOR(11), EQUIP_CLASS_ARMOR(10), record_indices);
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
