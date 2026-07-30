#include "common.h"

typedef struct {
    s16 red;
    s16 green;
    s16 blue;
} FieldFadeColor;

typedef struct {
    s16 red;
    s16 green;
    s16 blue;
    s16 duration;
} FieldFadeTarget;

extern FieldFadeColor g_field_fade_current;
extern FieldFadeTarget g_field_fade_target;

/**
 * @brief Reset the current and target field fade colors.
 * @see decomp.me (100%) TODO
 */
void field_reset_fade_state(void) {
    g_field_fade_current.red = 0;
    g_field_fade_current.green = 0;
    g_field_fade_current.blue = 0;
    g_field_fade_target.red = 0;
    g_field_fade_target.green = 0;
    g_field_fade_target.blue = 0;
    g_field_fade_target.duration = 0;
}
