#ifndef SCREEN_FADE_WORDS_H
#define SCREEN_FADE_WORDS_H

#include "common.h"

#include "main/display.h"

extern ScreenFadeWords g_fade_target;
extern ScreenFadeWords g_fade_current;

void reset_fade_state(void);
void update_and_draw_fade(ScreenFrame* frame);
void set_fade_target(s32 red, s32 green, s32 blue, s32 steps);

#endif
