#ifndef FIELD_ABILITY_PROGRESSION_H
#define FIELD_ABILITY_PROGRESSION_H

#include "common.h"

#define FIELD_ABILITY_UNLOCK_RULE_COUNT 18
#define FIELD_ABILITY_PREREQUISITE_NONE 0xFF
/** @brief field_find_combined_ability result when no rule matches. */
#define FIELD_ABILITY_NONE 0xFF

/** @brief An ability and the proficiency required to satisfy an unlock rule. */
typedef struct
{
    u8 ability;
    u8 proficiency;
} FieldAbilityPrerequisite;

/** @brief Two ability prerequisites and the ability they unlock. */
typedef struct
{
    FieldAbilityPrerequisite prerequisites[2];
    u8 result;
} FieldAbilityUnlockRule;

extern FieldAbilityUnlockRule g_field_ability_unlock_rules[];
extern u16 g_field_progression_unlocks[];
extern u16 g_field_progression_unlock_count;

void field_advance_ability_progression(void);
s32 field_find_combined_ability(s32 first_ability, s32 second_ability);

#endif
