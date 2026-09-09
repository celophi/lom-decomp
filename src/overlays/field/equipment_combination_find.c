#include "common.h"

extern s32 equipment_combination_variant(s32 *);
extern s32 equipment_combination_quantity(s32 *);
extern u8 g_equipment_combination_quantity_scale[];
extern s32 (*g_equipment_combination_rule_table[])(s32 *);

/**
 * @brief Find the first equipment rule matching the supplied record pair.
 * @param record_indices Two equipment-record indices tested by each rule.
 * @param quantity Receives the scaled quantity when a rule matches.
 * @param variant Receives the matching variant clamped to the range 0 through 10.
 * @return Matching rule index, or 0 if no rule matched.
 * @note Preserve both variant stores and the indexed rule-table call.
 * @note GCC 2.8.0 G0: 100% match, 58 instructions (232 bytes).
 */
s32 equipment_combination_find(s32 *record_indices, s32 *quantity, s32 *variant)
{
    s32 index;
    s32 value;
    s32 clamped;

    index = 0;
    do
    {
        if (g_equipment_combination_rule_table[index](record_indices) != 0)
        {
            value = equipment_combination_variant(record_indices);
            *variant = value;
            if (value >= 0)
            {
                clamped = 10;
                if (value < 11)
                {
                    clamped = value;
                }
            }
            else
            {
                clamped = 0;
            }
            *variant = clamped;
            *quantity = equipment_combination_quantity(record_indices) * g_equipment_combination_quantity_scale[index];
            return index;
        }
        index++;
    } while (index < 64);
    return 0;
}
