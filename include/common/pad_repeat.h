#ifndef PAD_REPEAT_H
#define PAD_REPEAT_H

#include "common.h"

#include "common/pad.h"

extern s32 g_debounced_input;
extern s32 g_last_input_state;
extern s32 g_input_repeat_timer;

s32 read_pad_buttons(void);
void init_pad_repeat(void);

#endif
