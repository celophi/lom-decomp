#include "common.h"

typedef struct {
    s16 red;
    s16 green;
    s16 blue;
    s16 duration;
} FieldFadeTarget;

typedef struct {
    s16 red;
    s16 green;
    s16 blue;
} FieldFadeRestoreColor;

extern FieldFadeTarget g_field_fade_target;
extern FieldFadeRestoreColor g_field_fade_restore_color;

/**
 * @brief Set both the target and saved restore color for the field fade.
 * @param red Target red intensity.
 * @param green Target green intensity.
 * @param blue Target blue intensity.
 * @param duration Transition duration in frames.
 * @see decomp.me (100%) TODO
 */
void field_set_fade_target(s16 red, s16 green, s16 blue, s16 duration) {
    g_field_fade_target.red = red;
    g_field_fade_restore_color.red = red;
    g_field_fade_target.green = green;
    g_field_fade_restore_color.green = green;
    g_field_fade_target.blue = blue;
    g_field_fade_restore_color.blue = blue;
    g_field_fade_target.duration = duration;
}

/**
 * @brief Set the field fade target used by the CD error overlay.
 * @see decomp.me (100%) TODO
 */
void field_set_cd_error_fade_target(void) {
    g_field_fade_target.red = 0xD0;
    g_field_fade_target.green = 0x100;
    g_field_fade_target.blue = 0x100;
    g_field_fade_target.duration = 5;
}

/**
 * @brief Set the field fade target without changing its saved restore color.
 * @param red Target red intensity.
 * @param green Target green intensity.
 * @param blue Target blue intensity.
 * @param duration Transition duration in frames.
 * @see decomp.me (100%) TODO
 */
void field_set_fade_target_only(s16 red, s16 green, s16 blue, s16 duration) {
    g_field_fade_target.red = red;
    g_field_fade_target.green = green;
    g_field_fade_target.blue = blue;
    g_field_fade_target.duration = duration;
}

/**
 * @brief Restore the saved field fade color over five frames.
 * @see decomp.me (100%) TODO
 */
void field_restore_fade_target(void) {
    g_field_fade_target.duration = 5;
    g_field_fade_target.red = (u16)g_field_fade_restore_color.red;
    g_field_fade_target.green = (u16)g_field_fade_restore_color.green;
    g_field_fade_target.blue = (u16)g_field_fade_restore_color.blue;
}

/**
 * @brief Restore the saved field fade color over the requested duration.
 * @param duration Transition duration in frames.
 * @see decomp.me (100%) TODO
 */
void field_restore_fade_target_with_duration(s16 duration) {
    g_field_fade_target.duration = duration;
    g_field_fade_target.red = (u16)g_field_fade_restore_color.red;
    g_field_fade_target.green = (u16)g_field_fade_restore_color.green;
    g_field_fade_target.blue = (u16)g_field_fade_restore_color.blue;
}

/**
 * @brief Set the default modal-overlay fade target.
 * @see decomp.me (100%) TODO
 */
void field_set_default_fade_target(void) {
    g_field_fade_target.red = 0xC0;
    g_field_fade_target.green = 0xC0;
    g_field_fade_target.blue = 0xC0;
    g_field_fade_target.duration = 5;
}
