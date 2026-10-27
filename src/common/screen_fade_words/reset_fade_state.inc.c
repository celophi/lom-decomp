#include "common/screen_fade_words.h"

/**
 * @brief Reset the current and target fade levels to black with no fade in progress.
 * @see decomp.me (100%, TITLE copy) https://decomp.me/scratch/m80gj
 * @see decomp.me (100%, GNAME copy) https://decomp.me/scratch/ld2aW
 */
void reset_fade_state(void)
{
    g_fade_current.red = 0;
    g_fade_current.green = 0;
    g_fade_current.blue = 0;

    g_fade_target.red = 0;
    g_fade_target.green = 0;
    g_fade_target.blue = 0;
    g_fade_target.steps_remaining = 0;
}
