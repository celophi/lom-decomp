#ifndef WMAP_EFFECT_BACKDROP_H
#define WMAP_EFFECT_BACKDROP_H

#include "common.h"

/** @brief Number of frames left in the mesh transition. */
extern s32 g_wmap_mesh_transition_frames;
/** @brief Signed mesh motion multiplier; zero when the transition is idle. */
extern s32 g_wmap_mesh_transition_direction;
/** @brief Current mesh transition selection. */
extern s32 g_wmap_mesh_transition_selection;
/** @brief Mesh selection replaced by the current transition. */
extern s32 g_wmap_mesh_previous_selection;

void wmap_init_transition_mesh(void);
void wmap_draw_transition_mesh(void);
void wmap_start_mesh_transition(s32 selection);
void wmap_select_mesh_motion();

#endif
