/** @file field_ability_progression.c
 * @brief Advance party proficiency and evaluate ability and technique unlocks.
 */

#include "field_ability_progression.h"
#include "saved_game.h"

#define FIELD_TECHNIQUE_UNLOCK_RULE_COUNT 227
#define FIELD_ABILITY_PROFICIENCY_COUNT 88
#define FIELD_PROGRESSION_PARTY_SIZE 3
#define FIELD_PROGRESSION_ACTIVE_FLAG 1
#define FIELD_PROGRESSION_CHARACTER_MASK 0x7F
#define FIELD_PROGRESSION_PLAYER_COUNT 2U
#define FIELD_PROFICIENCY_MAX 100U
#define FIELD_PROFICIENCY_BONUS 4
#define FIELD_WEAPON_CATEGORY_COUNT 11
#define FIELD_EQUIPPED_ABILITY_COUNT 2
#define FIELD_WEAPON_CATEGORY_SHIFT 10
#define FIELD_WEAPON_CATEGORY_MASK 0x3F
#define FIELD_TECHNIQUE_GROUP_SHIFT 4
#define FIELD_TECHNIQUE_WEAPON_MASK 0x0F
#define FIELD_TECHNIQUE_INDEX_MASK 0x7F
#define FIELD_TECHNIQUE_SILENT_FLAG 0x80
#define FIELD_TECHNIQUES_PER_GROUP 24
#define FIELD_TECHNIQUE_UNLOCK_FLAG 0x8000
#define FIELD_UNLOCK_BITS_PER_WORD 32

/** @brief Four ability prerequisites and a packed technique/weapon selection. */
typedef struct
{
    FieldAbilityPrerequisite prerequisites[4];
    u8 weapon;
    u8 result;
    u8 weapon_proficiency;
} FieldTechniqueUnlockRule;

/** @brief Saved character fields used to identify equipped abilities and weapons. */
typedef struct
{
    u8 pad_0[0x18];
    u8 character;
    u8 unknown_0x19;
    u8 abilities[FIELD_EQUIPPED_ABILITY_COUNT];
    u8 pad_1c[0x64 - 0x1C];
    u32 equipment;
    u8 pad_68[SAVED_CHARACTER_SIZE - 0x68];
} FieldProgressionCharacter;

/** @brief Persistent unlock masks, proficiency counters, and party equipment. */
typedef struct
{
    u8 pad_0[0x34];
    u32 techniques[FIELD_WEAPON_CATEGORY_COUNT];
    u32 abilities[3];
    u8 ability_proficiency[FIELD_ABILITY_PROFICIENCY_COUNT];
    u8 weapon_proficiency[FIELD_WEAPON_CATEGORY_COUNT];
    u8 pad_cf[0x5F0 - 0xCF];
    FieldProgressionCharacter characters[FIELD_PROGRESSION_PARTY_SIZE];
} FieldProgressionContext;

/** @brief Presence flags at the start of each runtime party record. */
typedef struct
{
    u8 flags;
    u8 pad_1[0x268 - 1];
} FieldProgressionPartyRecord;

extern FieldTechniqueUnlockRule g_field_technique_unlock_rules[];
extern FieldProgressionPartyRecord g_field_player_records[];
extern FieldProgressionContext* g_pad_ctx;
/* Scene image setup also uses this flag; its broader purpose is unresolved. */
extern s32 D_80115890;

/**
 * @brief Advance active player proficiency and queue newly learned abilities and techniques.
 * @note Proficiency saturates at 100, advancing by four when D_80115890 is set, or one otherwise.
 * @note A technique rule can unlock silently, without adding a dialog entry.
 */
void field_advance_ability_progression(void)
{
    FieldAbilityUnlockRule* ability_base;
    FieldTechniqueUnlockRule* technique_base;
    FieldTechniqueUnlockRule* active_rule;
    FieldProgressionContext* context;
    FieldAbilityUnlockRule* ability_rule;
    FieldTechniqueUnlockRule* technique_rule;
    s32 technique_mask;
    s32 unlocked_technique;
    s32 technique;
    s32 ability_word;
    s32 unlocked_ability_word;
    s32 equipped_weapon;
    s32 equipped_index;
    s32 party_index;
    s32 technique_party_index;
    u32 weapon_category;
    u32 technique_group;
    FieldProgressionPartyRecord* party_record;
    FieldProgressionPartyRecord* technique_party_record;
    u8 ability_proficiency;
    u8 weapon_requirement;
    s32 ability;
    s32 first_ability;
    s32 second_ability;
    s32 unlocked_ability;
    s32 first_required;
    s32 second_required;
    s32 third_required;
    s32 fourth_required;
    u8 weapon_proficiency;
    u8* weapon_proficiency_ptr;
    FieldProgressionCharacter* character;
    u8* weapon_clamp;
    u8* ability_proficiency_ptr;
    u8* ability_clamp;
    FieldProgressionCharacter* party_character;

    /* Advance the equipped weapon and both abilities for each active player. */
    party_record = g_field_player_records;
    for (party_index = 0; party_index < FIELD_PROGRESSION_PARTY_SIZE; party_index++)
    {
        if (party_record->flags & FIELD_PROGRESSION_ACTIVE_FLAG)
        {
            character = &g_pad_ctx->characters[party_index];
            if ((u32)(character->character & FIELD_PROGRESSION_CHARACTER_MASK) < FIELD_PROGRESSION_PLAYER_COUNT)
            {
                weapon_category = (character->equipment >> FIELD_WEAPON_CATEGORY_SHIFT) & FIELD_WEAPON_CATEGORY_MASK;
                if (weapon_category < FIELD_WEAPON_CATEGORY_COUNT)
                {
                    weapon_proficiency_ptr = &g_pad_ctx->weapon_proficiency[weapon_category];
                    weapon_proficiency = *weapon_proficiency_ptr;
                    if (weapon_proficiency < FIELD_PROFICIENCY_MAX)
                    {
                        if (D_80115890 != 0)
                        {
                            *weapon_proficiency_ptr = (u8)(weapon_proficiency + FIELD_PROFICIENCY_BONUS);
                            weapon_clamp = &g_pad_ctx->weapon_proficiency[(((u32)g_pad_ctx->characters[party_index].equipment >> FIELD_WEAPON_CATEGORY_SHIFT) &
                                                                           FIELD_WEAPON_CATEGORY_MASK)];
                            if ((u8)*weapon_clamp > FIELD_PROFICIENCY_MAX)
                            {
                                *weapon_clamp = FIELD_PROFICIENCY_MAX;
                            }
                        }
                        else
                        {
                            *weapon_proficiency_ptr = (u8)(weapon_proficiency + 1);
                        }
                    }
                }
                for (equipped_index = 0; equipped_index < FIELD_EQUIPPED_ABILITY_COUNT; equipped_index++)
                {
                    ability_proficiency_ptr = &g_pad_ctx->ability_proficiency[g_pad_ctx->characters[party_index].abilities[equipped_index]];
                    ability_proficiency = *ability_proficiency_ptr;
                    if (ability_proficiency < FIELD_PROFICIENCY_MAX)
                    {
                        if (D_80115890 != 0)
                        {
                            *ability_proficiency_ptr = (u8)(ability_proficiency + FIELD_PROFICIENCY_BONUS);
                            ability_clamp = &g_pad_ctx->ability_proficiency[g_pad_ctx->characters[party_index].abilities[equipped_index]];
                            if ((u8)*ability_clamp > FIELD_PROFICIENCY_MAX)
                            {
                                *ability_clamp = FIELD_PROFICIENCY_MAX;
                            }
                        }
                        else
                        {
                            *ability_proficiency_ptr = (u8)(ability_proficiency + 1);
                        }
                    }
                }
            }
        }
        party_record++;
    }
    context = g_pad_ctx;
    /* Ability rules require each nonempty prerequisite to be learned and trained. */
    ability_base = g_field_ability_unlock_rules;
    ability_rule = ability_base;
    g_field_progression_unlock_count = 0;
    do
    {
        ability = ability_rule->result;
        ability_word = ability / FIELD_UNLOCK_BITS_PER_WORD;
        if (!(context->abilities[ability_word] & (1 << (ability % FIELD_UNLOCK_BITS_PER_WORD))))
        {
            first_ability = ability_rule->prerequisites[0].ability;
            if (first_ability == FIELD_ABILITY_PREREQUISITE_NONE ||
                ((context->abilities[first_ability / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (first_ability % FIELD_UNLOCK_BITS_PER_WORD))) &&
                 (context->ability_proficiency[first_ability] >= ability_rule->prerequisites[0].proficiency)))
            {
                second_ability = ability_rule->prerequisites[1].ability;
                if (second_ability == FIELD_ABILITY_PREREQUISITE_NONE ||
                    ((context->abilities[second_ability / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (second_ability % FIELD_UNLOCK_BITS_PER_WORD))) &&
                     (context->ability_proficiency[second_ability] >= ability_rule->prerequisites[1].proficiency)))
                {
                    unlocked_ability = ability_rule->result;
                    unlocked_ability_word = unlocked_ability / FIELD_UNLOCK_BITS_PER_WORD;
                    context->abilities[unlocked_ability_word] =
                        (s32)(context->abilities[unlocked_ability_word] | (1 << (unlocked_ability % FIELD_UNLOCK_BITS_PER_WORD)));
                    g_field_progression_unlocks[g_field_progression_unlock_count] = (s16)unlocked_ability;
                    g_field_progression_unlock_count += 1;
                }
            }
        }
        ability_rule++;
    } while (ability_rule < ability_base + FIELD_ABILITY_UNLOCK_RULE_COUNT);
    context = g_pad_ctx;
    /* Techniques additionally require an active player with the matching weapon. */
    technique_base = g_field_technique_unlock_rules;
    technique_rule = technique_base;
    do
    {
        technique_group = technique_rule->weapon >> FIELD_TECHNIQUE_GROUP_SHIFT;
        technique = technique_rule->result & FIELD_TECHNIQUE_INDEX_MASK;
        if (!(context->techniques[technique_group] & (1 << (technique % FIELD_UNLOCK_BITS_PER_WORD))))
        {
            first_required = technique_rule->prerequisites[0].ability;
            if (first_required == FIELD_ABILITY_PREREQUISITE_NONE ||
                ((context->abilities[first_required / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (first_required % FIELD_UNLOCK_BITS_PER_WORD))) &&
                 (context->ability_proficiency[first_required] >= technique_rule->prerequisites[0].proficiency)))
            {
                second_required = technique_rule->prerequisites[1].ability;
                if (second_required == FIELD_ABILITY_PREREQUISITE_NONE ||
                    ((context->abilities[second_required / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (second_required % FIELD_UNLOCK_BITS_PER_WORD))) &&
                     (context->ability_proficiency[second_required] >= technique_rule->prerequisites[1].proficiency)))
                {
                    third_required = technique_rule->prerequisites[2].ability;
                    if (third_required == FIELD_ABILITY_PREREQUISITE_NONE ||
                        ((context->abilities[third_required / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (third_required % FIELD_UNLOCK_BITS_PER_WORD))) &&
                         (context->ability_proficiency[third_required] >= technique_rule->prerequisites[2].proficiency)))
                    {
                        fourth_required = technique_rule->prerequisites[3].ability;
                        if (fourth_required == FIELD_ABILITY_PREREQUISITE_NONE ||
                            ((context->abilities[fourth_required / FIELD_UNLOCK_BITS_PER_WORD] & (1 << (fourth_required % FIELD_UNLOCK_BITS_PER_WORD))) &&
                             (context->ability_proficiency[fourth_required] >= technique_rule->prerequisites[3].proficiency)))
                        {
                            active_rule = technique_rule;
                            technique_party_index = 0;
                            party_character = context->characters;
                            technique_party_record = g_field_player_records;
                            unlocked_technique = technique_rule->result & FIELD_TECHNIQUE_INDEX_MASK;
                            do
                            {
                                if ((technique_party_record->flags & FIELD_PROGRESSION_ACTIVE_FLAG) &&
                                    ((u32)(party_character->character & FIELD_PROGRESSION_CHARACTER_MASK) < FIELD_PROGRESSION_PLAYER_COUNT))
                                {
                                    weapon_requirement = active_rule->weapon;
                                    equipped_weapon = (party_character->equipment >> FIELD_WEAPON_CATEGORY_SHIFT) & FIELD_WEAPON_CATEGORY_MASK;
                                    if (equipped_weapon == (weapon_requirement & FIELD_TECHNIQUE_WEAPON_MASK))
                                    {
                                        if (context->weapon_proficiency[equipped_weapon] >= active_rule->weapon_proficiency)
                                        {
                                            technique_mask = 1 << (unlocked_technique % FIELD_UNLOCK_BITS_PER_WORD);
                                            if (!(context->techniques[technique_group] & technique_mask))
                                            {
                                                technique_group = weapon_requirement >> FIELD_TECHNIQUE_GROUP_SHIFT;
                                                context->techniques[technique_group] = (s32)(context->techniques[technique_group] | technique_mask);
                                                if (!(active_rule->result & FIELD_TECHNIQUE_SILENT_FLAG))
                                                {
                                                    g_field_progression_unlocks[g_field_progression_unlock_count] =
                                                        ((technique_group * FIELD_TECHNIQUES_PER_GROUP) + unlocked_technique) | FIELD_TECHNIQUE_UNLOCK_FLAG;
                                                    g_field_progression_unlock_count += 1;
                                                }
                                            }
                                        }
                                    }
                                }
                                party_character++;
                                technique_party_index += 1;
                                technique_party_record++;
                            } while (technique_party_index < FIELD_PROGRESSION_PARTY_SIZE);
                        }
                    }
                }
            }
        }
        technique_rule++;
    } while (technique_rule < technique_base + FIELD_TECHNIQUE_UNLOCK_RULE_COUNT);
}
