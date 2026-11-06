#ifndef OVERLAYS_ADDHERO_H
#define OVERLAYS_ADDHERO_H

#include "common.h"

/**
 * @file addhero.h
 * @brief ADDHERO's entry points, called by FIELD once the overlay is loaded.
 */

struct FieldRenderHalf;

/**
 * @brief Start ADDHERO.
 * @param work Work buffer FIELD passes.
 * @param mode Mode FIELD opened ADDHERO in.
 */
void addhero_init(void* work, s32 mode);

/**
 * @brief Update and draw one ADDHERO frame.
 * @param draw_state FIELD's render half for the frame.
 * @return Nonzero once ADDHERO has finished.
 */
s32 addhero_state_step(struct FieldRenderHalf* draw_state);

#endif
