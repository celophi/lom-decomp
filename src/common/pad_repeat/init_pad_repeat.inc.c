#include "common/pad_repeat.h"

/**
 * @brief Seed the key-repeat state from the current controller sample.
 * @see decomp.me (100%, TITLE copy) https://decomp.me/scratch/1dQbp
 */
void init_pad_repeat(void)
{
    g_debounced_input = 0;
    g_last_input_state = sample_pad_buttons();
    g_input_repeat_timer = PAD_REPEAT_INITIAL_DELAY;
}
