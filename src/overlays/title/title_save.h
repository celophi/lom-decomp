#ifndef LOM_TITLE_SAVE_H
#define LOM_TITLE_SAVE_H

#include "title.h"

/** @brief Initialize the title screen's save-slot picker state and artwork. */
void init_save_slot_menu(void);

/**
 * @brief Draw the save-slot picker and process its input for one frame.
 * @param context Active title display and primitive buffers.
 */
void render_save_slot_menu(TitleMenuContext* context);

/**
 * @brief Copy a new-game or alternate state template into the active save data.
 * @param use_alt Zero for a new game, nonzero for the alternate template.
 */
void load_menu_layout(s32 use_alt);

#endif
