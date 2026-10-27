#include "common/screen_fade_words.h"

/**
 * @brief Start a fade toward the given levels.
 * @param red Target red level (FADE_NEUTRAL leaves the channel unchanged).
 * @param green Target green level.
 * @param blue Target blue level.
 * @param steps Number of frames over which to interpolate.
 * @see decomp.me (100%, TITLE copy) https://decomp.me/scratch/zxqdP
 * @see decomp.me (100%, GNAME copy) https://decomp.me/scratch/jq3uD
 */
void set_fade_target(s32 red, s32 green, s32 blue, s32 steps)
{
    g_fade_target.red = red;
    g_fade_target.green = green;
    g_fade_target.blue = blue;
    g_fade_target.steps_remaining = steps;
}
