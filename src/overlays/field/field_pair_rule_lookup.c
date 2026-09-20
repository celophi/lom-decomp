/** @file field_pair_rule_lookup.c
 * @brief Find an unordered pair in the shared packed rule table.
 */

#include "field_ability_progression.h"

/**
 * @brief Looks up the result byte for an unordered pair (@p first_ability, @p second_ability).
 *
 * Scans 18 table entries for one whose prerequisite abilities hold @p first_ability and @p second_ability in
 * either order.
 * @param first_ability First prerequisite ability.
 * @param second_ability Second prerequisite ability.
 * @return The resulting ability, or 0xFF when no rule matches.
 */
s32 func_800AD7DC(s32 first_ability, s32 second_ability)
{
    FieldAbilityUnlockRule* rule = g_field_ability_unlock_rules;
    s32 rule_index;

    rule_index = 0;
    while (rule_index < FIELD_ABILITY_UNLOCK_RULE_COUNT)
    {
        if ((rule->prerequisites[0].ability == first_ability && rule->prerequisites[1].ability == second_ability) ||
            (rule->prerequisites[0].ability == second_ability && rule->prerequisites[1].ability == first_ability))
        {
            return rule->result;
        }
        rule_index++;
        rule++;
    }
    return 0xFF;
}
