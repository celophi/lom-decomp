/**
 * @file field_layout_slot_state.c
 * @brief Place a land on the world map and reset all lands for a new game.
 */

#include "saved_game.h"
#include "common.h"
#include "field_calls.h"
#include "field_records.h"

/** @brief g_saved_game viewed as FIELD's game state. */
#define FIELD_SAVED_GAME ((FieldGameState *)&g_saved_game)

/**
 * @brief Place a land on the map without any checks and record its placement order.
 * @param land_index Land index, below FIELD_LAND_COUNT.
 * @see field_try_place_land for the checked version.
 */
void field_place_land(s32 land_index)
{
    FieldGameState *game = FIELD_SAVED_GAME;

    game->control.fields.placed_land_count++;
    game->lands[land_index].flags |= FIELD_LAND_PLACED;
    game->lands[land_index].count = game->control.fields.placed_land_count;
}

/**
 * @brief Take every land off the map and clear its levels, as for a new game.
 *
 * Also clears the hero level. Afterwards every land has FIELD_LAND_FLAG_04
 * set except land 24.
 */
void field_reset_lands(void)
{
    FieldGameState *game;
    s32 i;

    FIELD_SAVED_GAME->control.fields.placed_land_count = 0;
    FIELD_SAVED_GAME->control.fields.hero_level = 0;
    for (i = 0; i < FIELD_LAND_COUNT; i++)
    {
        FIELD_SAVED_GAME->lands[i].x = FIELD_LAND_CELL_NONE;
        FIELD_SAVED_GAME->lands[i].z = FIELD_LAND_CELL_NONE;
        FIELD_SAVED_GAME->lands[i].unk2 = 0;
        FIELD_SAVED_GAME->lands[i].count = 0;
        FIELD_SAVED_GAME->lands[i].levels[0] = 0;
        FIELD_SAVED_GAME->lands[i].levels[1] = 0;
        FIELD_SAVED_GAME->lands[i].levels[2] = 0;
        FIELD_SAVED_GAME->lands[i].levels[3] = 0;
        FIELD_SAVED_GAME->lands[i].levels[4] = 0;
        FIELD_SAVED_GAME->lands[i].levels[5] = 0;
        FIELD_SAVED_GAME->lands[i].levels[6] = 0;
        FIELD_SAVED_GAME->lands[i].levels[7] = 0;
        FIELD_SAVED_GAME->lands[i].flags &= ~(FIELD_LAND_PLACED | FIELD_LAND_FLAG_02 | FIELD_LAND_FLAG_04);
    }

    for (i = 0; i < FIELD_LAND_COUNT; i++)
    {
        FIELD_SAVED_GAME->lands[i].flags |= FIELD_LAND_FLAG_04;
    }
    /* Through a local pointer; the direct form folds the offset into the symbol address. */
    game = FIELD_SAVED_GAME;
    ((FieldLandWords *)game->lands)[24].word &= ~FIELD_LAND_FLAG_04;
}
