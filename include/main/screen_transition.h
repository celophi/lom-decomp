#ifndef _SCREEN_TRANSITION_H
#define _SCREEN_TRANSITION_H

#include "common.h"

/**
 * @brief Fade out the screen while servicing controllers and CD reads.
 * @param skip_fade Nonzero waits without fading or disabling the display.
 */
void screen_transition(s32 skip_fade);

#endif
