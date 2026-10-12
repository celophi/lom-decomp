#ifndef FIELD_OVERLAY_HANDOFF_H
#define FIELD_OVERLAY_HANDOFF_H

#include "common.h"

/**
 * @file field_overlay_handoff.h
 * @brief FIELD-resident state passed between FIELD and the menu overlays it
 *        loads into the secondary overlay area (GOSUB, CARDA, NIKI and ADDHERO).
 */

/** @brief Progress of the running menu overlay as FIELD scripts see it: 1 while it runs, 2 once it has finished. */
extern s32 g_field_gosub_state;

/** @brief Number of values the menu overlay left in g_gosub_result_values. */
extern s32 g_gosub_result_count;

/** @brief Values a menu overlay hands back to FIELD: selected indexes and materials, or the transferred pet slot. */
extern s32 g_gosub_result_values[];

/** @brief CARDA overlay mode plus one while CARDA is open, 0 otherwise; CARDA replaces it with its CARDA_RESULT_* code. */
extern s32 g_field_card_overlay_mode;

/** @brief Ranch pet slot CARDA transfers to or from a PocketStation. */
extern s32 g_field_card_pet_slot;

/** @brief NIKI and ADDHERO progress: FIELD sets 1 when it opens them. TODO: later values unknown. */
extern s32 g_field_niki_addhero_state;

#endif
