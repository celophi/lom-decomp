#ifndef LOM_TITLE_SAVE_H
#define LOM_TITLE_SAVE_H

#include "overlays/title/title.h"

/** @brief Initialize the title screen's save-slot picker state and artwork. */
void init_save_slot_menu(void);

/**
 * @brief Draw the save-slot picker and process its input for one frame.
 * @param context Active title display and primitive buffers.
 */
void render_save_slot_menu(TitleMenuContext* context);

/**
 * @brief Replace the saved game with the new-game or the field-start template.
 * @param field_start Zero for a new game, nonzero to start directly in FIELD.
 */
void load_saved_game_template(s32 field_start);

#endif
