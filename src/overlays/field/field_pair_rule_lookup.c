/** @file field_pair_rule_lookup.c
 * @brief Find the ability that two abilities combine into.
 */

#include "field_ability_progression.h"

/**
 * @brief Look up the ability unlocked by an unordered pair of abilities.
 *
 * Scans g_field_ability_unlock_rules for a rule whose two prerequisite
 * abilities are @p first_ability and @p second_ability, in either order.
 *
 * @param first_ability First ability of the pair.
 * @param second_ability Second ability of the pair.
 * @return The rule's resulting ability, or FIELD_ABILITY_NONE when no rule matches.
 */
s32 field_find_combined_ability(s32 first_ability, s32 second_ability)
{
    FieldAbilityUnlockRule* rule;
    s32 i;

    rule = g_field_ability_unlock_rules;
    for (i = 0; i < FIELD_ABILITY_UNLOCK_RULE_COUNT; i++, rule++)
    {
        if ((rule->prerequisites[0].ability == first_ability && rule->prerequisites[1].ability == second_ability) ||
            (rule->prerequisites[0].ability == second_ability && rule->prerequisites[1].ability == first_ability))
        {
            return rule->result;
        }
    }
    return FIELD_ABILITY_NONE;
}
