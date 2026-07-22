#ifndef _GAME_AUDIO_H
#define _GAME_AUDIO_H

#include "common.h"

void akao_song_cmd_12c(void);
void load_and_play_song(s32 song_index);
void akao_set_song_params(int flags, s16 duration, s16 field_id, s16 sub_id);

#endif
