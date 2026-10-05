#ifndef WMAP_MAIN_H
#define WMAP_MAIN_H

#include "overlays/wmap/world_map.h"
#include <libgpu.h>
#include "main/controller_internal.h"

/** @brief Controller ports used for world-map input and vibration commands. */
extern ControllerPortState* g_wmap_controller_ports;

/** @brief Nonzero to defer filling a missing land-image cache entry. */
extern s32 g_wmap_land_image_load_locked;

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

/** @brief Rendering applied after the world-map exit frame is captured. */
enum WmapExitMode
{
    WMAP_EXIT_SCREEN_PANELS = 0,
    WMAP_EXIT_FADE_OVERLAY = 1,
    WMAP_EXIT_DIRECT = 2
};

/** @brief Frames since exit was requested; zero keeps the map running. */
extern s32 g_wmap_exit_frame;
/** @brief Rendering mode used after the exit frame has been captured. */
extern s32 g_wmap_exit_mode;
/** @brief Queued land-event ticks remaining for the event dispatcher. */
extern s32 g_wmap_pending_event_count;
/** @brief Pending vehicle events; the first five slots handle flags 25 and 17-20. */
extern u8 g_wmap_pending_vehicle_events[8];
/** @brief Land-event flag 4-8 selected for special effect 34. */
extern s32 g_wmap_special_land_event;
/** @brief Status-panel mode; value three hides the day and placement sprites. */
extern s32 g_wmap_status_panel_mode;
/** @brief Pending sequence requested by land-event flag 11. */
extern s32 g_wmap_land_event_11_pending;
/** @brief Pending sequence requested by land-event flag 9. */
extern s32 g_wmap_land_event_09_pending;
/** @brief Pending sequence requested by land-event flag 21. */
extern s32 g_wmap_land_event_21_pending;
/** @brief Pending sequence requested by land-event flag 16. */
extern s32 g_wmap_land_event_16_pending;
/** @brief Pending sequence requested by land-event flag 15. */
extern s32 g_wmap_land_event_15_pending;
/** @brief Pending sequence requested by land-event flag 13. */
extern s32 g_wmap_land_event_13_pending;
/** @brief Pending sequence requested by land-event flag 12. */
extern s32 g_wmap_land_event_12_pending;
/** @brief Pending sequence requested by land-event flag 27. */
extern s32 g_wmap_land_event_27_pending;
/** @brief Pending sequence requested by land-event flag 24. */
extern s32 g_wmap_land_event_24_pending;
/** @brief Land-event flag 10 selects the alternate event-00 sequence. */
extern s32 g_wmap_land_event_10_active;

/** @brief Template for the four-corner backdrop gradient. */
extern POLY_G4 g_wmap_backdrop_gradient_template;

/** @brief Backdrop gradient whose vertex colors approach their targets. */
extern POLY_G4 g_wmap_backdrop_gradient;

/** @brief Nonzero to start the special return sequence after party setup. */
extern s32 g_wmap_special_return_pending;

#endif
