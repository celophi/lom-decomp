#ifndef _TITLE_H
#define _TITLE_H

#include "common.h"

typedef struct TitleMenuContext TitleMenuContext;

/**
 * @brief Run the title screen and select the next game state.
 * @param menu_context Title display buffers.
 * @return Next game-state code selected by the title screen.
 */
s32 run_title(TitleMenuContext* menu_context);

#endif
