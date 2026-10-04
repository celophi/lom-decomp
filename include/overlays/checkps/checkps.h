#ifndef _CHECKPS_H
#define _CHECKPS_H

#include "common.h"

typedef struct CheckPSRenderState CheckPSRenderState;

/**
 * @brief Run the startup CHECKPS screen and hardware checks.
 * @param render_state Double-buffered renderer workspace supplied by the caller.
 * @return Top-level game state to enter after CHECKPS completes.
 */
s32 run_checkps(CheckPSRenderState* render_state);

#endif
