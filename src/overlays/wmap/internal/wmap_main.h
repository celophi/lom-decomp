#ifndef WMAP_MAIN_H
#define WMAP_MAIN_H

#include "overlays/wmap/world_map.h"

/** @brief Shared input, menu, and fade state managed by the world-map loop. */
extern s32 g_wmap_buttons_repeat;
extern s32 g_wmap_buttons_held;
extern s32 g_wmap_map_button_mask;
extern s32 g_wmap_input_locked;
extern s32 g_wmap_backdrop_target_level;
extern s32 g_wmap_screen_fade_mode;

/**
 * @brief Confirm and cancel buttons of the world-map prompts (sdk/libetc.h bits).
 * @note US confirms with cross and cancels with circle; JP swaps them.
 */
#if defined(VERSION_JP)
#define WMAP_PAD_CONFIRM PADRright
#define WMAP_PAD_CANCEL PADRdown
#else
#define WMAP_PAD_CONFIRM PADRdown
#define WMAP_PAD_CANCEL PADRright
#endif

void wmap_reset_after_transition(void);

#endif
