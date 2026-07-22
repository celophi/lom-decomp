#ifndef _GAME_AUDIO_H
#define _GAME_AUDIO_H

#include "common.h"
#include "akao.h"

extern s32 g_current_song_handle;
extern u8 g_music_data_buffer;
extern s16 g_akao_song_cmd_arg0;
extern s16 g_akao_song_cmd_neg_arg0;
extern s16 g_akao_song_cmd_arg1;
extern s16 g_akao_song_cmd_arg2;
extern s16 g_akao_song_cmd_arg3;

void akao_song_cmd_12c(void);
void load_and_play_song(s32 song_index);
void akao_set_song_params(int flags, s16 duration, s16 field_id, s16 sub_id);

#endif
