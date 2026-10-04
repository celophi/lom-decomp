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

/** @brief Land whose animated display overrides the normal visibility limit. */
extern s32 g_wmap_forced_animated_land_id;

/** @brief Nonzero to hide the auxiliary map labels. */
extern s32 g_wmap_auxiliary_labels_hidden;

/** @brief Nonzero while selecting a map cell for artifact placement. */
extern s32 g_wmap_artifact_placement_active;

/** @brief Keep the preview visible and retain its position outside projected mode. */
extern s32 g_wmap_placement_preview_locked;

/** @brief Maximum land display mode: hidden (0), marker (1), or animated (2). */
extern s32 g_wmap_land_display_limit;

/** @brief Artifact placement frame counter, also used as its downward screen offset. */
extern s32 g_wmap_artifact_placement_frame;

/** @brief Nonzero to draw the artifact carousel shadows. */
extern s32 g_wmap_artifact_shadows_enabled;

/** @brief Last requested effect resource set; -1 forces the next request to dispatch. */
extern s32 g_wmap_last_effect_resource_set;

void wmap_reset_after_transition(void);

#endif
