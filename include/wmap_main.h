#ifndef WMAP_MAIN_H
#define WMAP_MAIN_H

#include "world_map.h"

/** @brief Shared input, menu, and fade state managed by the world-map loop. */
extern s32 g_wmap_buttons_repeat;
extern s32 g_wmap_buttons_held;
extern s32 g_wmap_map_button_mask;
extern s32 g_wmap_input_locked;
extern s32 g_wmap_backdrop_target_level;
extern s32 g_wmap_screen_fade_mode;

void wmap_reset_after_transition(void);

#endif
