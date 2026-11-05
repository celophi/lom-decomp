/**
 * @file field_layout_slot_state.c
 * @brief Place a land on the world map and reset all lands for a new game.
 */

#include "common.h"

#include "../internal/field_calls.h"
#include "../internal/field_records.h"
#include "common/saved_game.h"

/** @brief g_saved_game viewed as FIELD's game state. */
#define FIELD_SAVED_GAME ((SavedGameLayout*)&g_saved_game)

/**
 * @brief Give the player a land's artifact without any checks and record its artifact order.
 * @param land_index Land index, below FIELD_LAND_COUNT.
 * @see field_try_award_land_artifact for the checked version.
 */
void field_award_land_artifact(s32 land_index)
{
    SavedGameLayout* game = FIELD_SAVED_GAME;

    game->control.fields.artifact_count++;
    game->lands[land_index].flags |= FIELD_LAND_ARTIFACT_OBTAINED;
    game->lands[land_index].artifact_order = game->control.fields.artifact_count;
}

/**
 * @brief Take every land off the map, take back every artifact and clear the lands' levels, as for a new game.
 *
 * Also clears the artifact and placed land counts. Afterwards every land has
 * FIELD_LAND_FLAG_04 set except land 24.
 */
void field_reset_lands(void)
{
    SavedGameLayout* game;
    s32 i;

    FIELD_SAVED_GAME->control.fields.artifact_count = 0;
    FIELD_SAVED_GAME->control.fields.placed_land_count = 0;

    for (i = 0; i < FIELD_LAND_COUNT; i++)
    {
        FIELD_SAVED_GAME->lands[i].x = FIELD_LAND_CELL_NONE;
        FIELD_SAVED_GAME->lands[i].z = FIELD_LAND_CELL_NONE;
        FIELD_SAVED_GAME->lands[i].placement_order = 0;
        FIELD_SAVED_GAME->lands[i].artifact_order = 0;
        FIELD_SAVED_GAME->lands[i].levels[0] = 0;
        FIELD_SAVED_GAME->lands[i].levels[1] = 0;
        FIELD_SAVED_GAME->lands[i].levels[2] = 0;
        FIELD_SAVED_GAME->lands[i].levels[3] = 0;
        FIELD_SAVED_GAME->lands[i].levels[4] = 0;
        FIELD_SAVED_GAME->lands[i].levels[5] = 0;
        FIELD_SAVED_GAME->lands[i].levels[6] = 0;
        FIELD_SAVED_GAME->lands[i].levels[7] = 0;
        FIELD_SAVED_GAME->lands[i].flags &= ~(FIELD_LAND_ARTIFACT_OBTAINED | FIELD_LAND_PLACED | FIELD_LAND_FLAG_04);
    }

    for (i = 0; i < FIELD_LAND_COUNT; i++)
    {
        FIELD_SAVED_GAME->lands[i].flags |= FIELD_LAND_FLAG_04;
    }

    game = FIELD_SAVED_GAME;
    ((FieldLandWords*)game->lands)[24].word &= ~FIELD_LAND_FLAG_04;
}
