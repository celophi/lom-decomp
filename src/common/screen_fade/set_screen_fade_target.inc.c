#include "common/screen_fade.h"

/**
 * @brief Set the target level of each channel and the number of steps to reach it.
 * @param red Target red level.
 * @param green Target green level.
 * @param blue Target blue level.
 * @param steps Number of frames the fade takes.
 */
void set_screen_fade_target(s16 red, s16 green, s16 blue, s16 steps)
{
    g_screen_fade_target.red = red;
    g_screen_fade_target.green = green;
    g_screen_fade_target.blue = blue;
    g_screen_fade_target.steps_remaining = steps;
}
