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
 * @see func_800C3518 for the checked version.
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
    FieldGameState *view;
    s32 i;

    i = 0;
    FIELD_SAVED_GAME->control.fields.placed_land_count = 0;
    FIELD_SAVED_GAME->control.fields.hero_level = 0;
    /* view walks g_saved_game one land record per pass, so view->lands[0] is lands[i].
     * A structured loop lets loop.c hoist the FIELD_LAND_POSITION_NONE constant out of it. */
    view = FIELD_SAVED_GAME;
reset_land:
    i++;
    view->lands[0].position = FIELD_LAND_POSITION_NONE;
    view->lands[0].unk2 = 0;
    view->lands[0].count = 0;
    view->lands[0].levels[0] = 0;
    view->lands[0].levels[1] = 0;
    view->lands[0].levels[2] = 0;
    view->lands[0].levels[3] = 0;
    view->lands[0].levels[4] = 0;
    view->lands[0].levels[5] = 0;
    view->lands[0].levels[6] = 0;
    view->lands[0].levels[7] = 0;
    view->lands[0].flags &= ~(FIELD_LAND_PLACED | FIELD_LAND_FLAG_02 | FIELD_LAND_FLAG_04);
    view = (FieldGameState *)((FieldLandRecord *)view + 1);
    if (i < FIELD_LAND_COUNT)
    {
        goto reset_land;
    }

    for (i = 0; i < FIELD_LAND_COUNT; i++)
    {
        FIELD_SAVED_GAME->lands[i].flags |= FIELD_LAND_FLAG_04;
    }
    /* Through a local pointer; the direct form folds the offset into the symbol address. */
    game = FIELD_SAVED_GAME;
    ((FieldLandWords *)game->lands)[24].word &= ~FIELD_LAND_FLAG_04;
}
