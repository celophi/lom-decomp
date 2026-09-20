#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "common.h"

/**
 * @brief Top-level game state machine values stored in g_game_state.
 *
 * main_game_loop() drives the game through these states in an infinite loop. States
 * 0, 9, and 10 share the field-entry path. State 4 is a transient value that
 * main_game_loop() immediately redirects to GAME_STATE_TITLE.
 */
#define GAME_STATE_FIELD 0           /**< Enter field overlay (attract, demo, or normal). */
#define GAME_STATE_WORLD_MAP 1       /**< Enter world map overlay (WMAP.BIN). */
#define GAME_STATE_TITLE 2           /**< Enter title screen (TITLE.BIN). */
#define GAME_STATE_GNAME 3           /**< Enter GNAME overlay (name entry within field). */
#define GAME_STATE_RETURN_TO_TITLE 4 /**< Redirect to the title screen. */
#define GAME_STATE_WORLD_SELECT 5    /**< Enter world select overlay (WSEL.BIN). */
#define GAME_STATE_MENU_LOAD 7       /**< Enter menu/load overlay (save or continue). */
#define GAME_STATE_INTRO_MOVIE 8     /**< Play intro movie, then transition to title. */
#define GAME_STATE_ATTRACT_1 9       /**< Play attract movie 1, then enter field. */
#define GAME_STATE_ATTRACT_2 10      /**< Play attract movies 2 through 4, then enter field. */
#define GAME_STATE_NONE 0xFF         /**< Sentinel indicating no previous state. */

extern u32 g_game_state;
extern u32 g_previous_game_state;

#endif
