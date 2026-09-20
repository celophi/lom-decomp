#ifndef WSEL_H
#define WSEL_H

#include "common.h"

/**
 * @brief Run world selection until the player chooses an exit state.
 * @param render_context Workspace for the world-selection display.
 * @return Next top-level game state.
 */
s32 wsel_main(void* render_context);

#endif
