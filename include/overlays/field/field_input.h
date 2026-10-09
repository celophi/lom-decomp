#ifndef FIELD_INPUT_H
#define FIELD_INPUT_H

#include "common.h"

/**
 * @file field_input.h
 * @brief FIELD's controller input with key repeat, which the menu overlays read through g_pad_input.
 */

/**
 * @brief Read both controllers and apply the initial delay and key repeat to their buttons.
 * @note A new button fires at once, waits FIELD_PAD_REPEAT_DELAY frames, then repeats every third frame.
 * @note While a modal screen is open, held directions repeat on their own without the other buttons.
 */
void field_update_input_repeat(void);

/**
 * @brief Clear delivered input and restart both controllers' held-button delays.
 */
void field_reset_input_repeat(void);

#endif
