#include "common.h"
#include "field_calls.h"
#include "field_records.h"

/** @brief Level at and above which the experience threshold is the stored experience. */
#define FIELD_LEVEL_CAP 99

/** @brief Hit points a character is reset to before replaying its level-ups. */
#define FIELD_RESET_HP 50

/** @brief Base value (before growth bits) each stat is reset to. */
#define FIELD_RESET_STAT 20

extern FieldGameState* g_field_game_state;

/**
 * @brief Return the experience needed to advance from @p level.
 * @param level Current level.
 * @return (level - 1) * level * 20 + level * 10.
 */
static inline s32 field_level_threshold(s32 level)
{
    s32 previous = level - 1;
    s32 scaled = level * 5;
    s32 scaled_x4 = scaled * 4;

    return previous * scaled_x4 + scaled * 2;
}

/**
 * @brief Return the experience a party character needs for its next level.
 * @param index Party character index.
 * @return (level - 1) * level * 20 + level * 10 below level 99, otherwise the
 *         character's current experience.
 */
s32 func_800B607C(s32 index)
{
    s32 level = g_field_game_state->characters[index].progress.level;

    if (level < FIELD_LEVEL_CAP)
    {
        return field_level_threshold(level);
    }

    return g_field_game_state->characters[index].progress.word >> 8;
}

/**
 * @brief Reset every party character to level 1 and level it up to @p level.
 * @param level Target level; each character gets the experience threshold of
 *              level - 1 and then advances through func_800C14A4.
 */
void func_800B60DC(s32 level)
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
            g_field_game_state->characters[i].stats[j] = (g_field_game_state->characters[i].stats[j] & 0xFE00) | FIELD_RESET_STAT;
        }

        while (func_800C14A4(i, 1) != 0)
        {
        }
    }
}
