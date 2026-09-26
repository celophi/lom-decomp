/**
 * @file field_progression_ops.c
 * @brief Party level queries: experience to the next level and replaying level-ups.
 */

#include "common.h"
#include "field_calls.h"
#include "field_records.h"

/** @brief Hit points a character is reset to before replaying its level-ups. */
#define FIELD_RESET_HP 50

/** @brief Base value (quarter units) each stat is reset to before replaying its level-ups. */
#define FIELD_RESET_STAT 20

extern FieldGameState* g_field_game_state;

/**
 * @brief Return the experience a party character needs for its next level.
 * @param index Party character index.
 * @return field_level_threshold() of the character's level below FIELD_LEVEL_MAX, otherwise
 *         the character's current experience.
 */
s32 field_get_next_level_experience(s32 index)
{
    s32 level = g_field_game_state->characters[index].progress.level;

    if (level < FIELD_LEVEL_MAX)
    {
        return field_level_threshold(level);
    }

    return g_field_game_state->characters[index].progress.word >> 8;
}

/**
 * @brief Reset every party character to level 1 and replay its level-ups up to @p level.
 * @param level Target level; each character gets the experience threshold of level - 1
 *              and then advances through field_try_character_level_up.
 */
void field_reset_party_to_level(s32 level)
{
    s32 i;
    s32 j;
    s32 experience;

    level--;
    experience = field_level_threshold(level) << 8;
    for (i = 0; i < FIELD_PARTY_SIZE; i++)
    {
        g_field_game_state->characters[i].progress.level = 1;
        g_field_game_state->characters[i].hp = FIELD_RESET_HP;
        g_field_game_state->characters[i].progress.word = g_field_game_state->characters[i].progress.level | experience;

        for (j = 0; j < FIELD_CHARACTER_STAT_COUNT; j++)
        {
            g_field_game_state->characters[i].stats[j] = (g_field_game_state->characters[i].stats[j] & FIELD_STAT_EFFECTIVE_MASK) | FIELD_RESET_STAT;
        }

        while (field_try_character_level_up(i, 1) != 0)
        {
        }
    }
}
