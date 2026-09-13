#ifndef _TITLE_H
#define _TITLE_H

#include "common.h"

/**
 * @brief Run the title screen and select the next game state.
 * @param menu_context_address Address of the title display buffers.
 * @return Next game-state code selected by the title screen.
 */
s32 run_title(s32 menu_context_address);

#endif
