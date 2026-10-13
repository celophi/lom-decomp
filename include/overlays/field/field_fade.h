#ifndef FIELD_FADE_H
#define FIELD_FADE_H

#include "common.h"
#include "main/field_runtime.h"

/**
 * @file field_fade.h
 * @brief FIELD's screen fade, which the menu overlays use to dim the scene
 *        behind their windows and to restore it when they close.
 */

/** @brief Ordering-table entry the fade packets are linked into. */
#define FIELD_FADE_OT_INDEX 16

/** @brief Frames the standard fades take. */
#define FIELD_FADE_DEFAULT_FRAMES 5

/** @brief Channel level of the dimmed scene behind modal windows and result screens. */
#define FIELD_FADE_MODAL_LEVEL 0xC0

/**
 * @brief Fade color triple plus its remaining step count.
 * @note @c duration is the transition length in frames when set as a target,
 *       and the number of remaining interpolation steps while the fade runs.
 */
typedef struct
{
    s16 red;
    s16 green;
    s16 blue;
    s16 duration;
} FieldFade;

/** @brief Saved fade color with no step/duration field. */
typedef struct
{
    s16 red;
    s16 green;
    s16 blue;
} FieldFadeColor;

/** @brief Fade color shown this frame. */
extern FieldFade g_field_fade_current;

/** @brief Fade color being approached and the frames left to reach it. */
extern FieldFade g_field_fade_target;

/** @brief Fade color restored when a modal window closes. */
extern FieldFadeColor g_field_fade_restore_color;

/**
 * @brief Set both the target and saved restore color for the field fade.
 * @param red Target red intensity.
 * @param green Target green intensity.
 * @param blue Target blue intensity.
 * @param duration Transition duration in frames.
 */
void field_set_fade_target(s16 red, s16 green, s16 blue, s16 duration);

/**
 * @brief Reset the current and target field fade colors.
 */
void field_reset_fade_state(void);

/**
 * @brief Advance the screen fade one step and emit its blend tile + draw mode.
 * @param render_half Render half whose ordering-table entry FIELD_FADE_OT_INDEX receives the packets.
 */
void field_update_and_render_fade(FieldRenderHalf* render_half);

/**
 * @brief Restore the saved field fade color over FIELD_FADE_DEFAULT_FRAMES frames.
 */
void field_restore_fade_target(void);

/**
 * @brief Restore the saved field fade color over the requested duration.
 * @param duration Transition duration in frames.
 */
void field_restore_fade_target_with_duration(s16 duration);

/**
 * @brief Set the default modal-overlay fade target.
 */
void field_set_default_fade_target(void);

#endif
