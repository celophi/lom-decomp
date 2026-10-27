#ifndef OVERLAY_AUDIO_H
#define OVERLAY_AUDIO_H

#include "common.h"

#include "main/audio/game_audio.h"

extern u8 g_overlay_song_buffer[OVERLAY_SONG_BUFFER_SIZE];

void load_song_from_disc(s32 song_index);
void stop_song(void);
void play_loaded_song(void);
void play_sfx(s32 sound_id, s32 pan);

#endif
