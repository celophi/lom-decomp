#ifndef ZUKAN_H
#define ZUKAN_H

#include "common.h"

/** @brief Initialize the encyclopedia overlay state and return the next free work-buffer address. */
s32 zukan_initialize_state(s32 work_buffer, s32 category);

/** @brief Build and render one encyclopedia frame. */
void zukan_update_frame(s32 frame_context);

/** @brief Load the encyclopedia UI resource archive. */
void zukan_load_ui_resource(void);

#endif
