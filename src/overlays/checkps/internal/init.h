#ifndef CHECKPS_INIT_H
#define CHECKPS_INIT_H

#include "overlays/checkps/checkps.h"

void load_checkps_song_from_disc(s32 song_index);
void stop_checkps_song(void);
void play_loaded_checkps_song(void);
void play_checkps_sfx(u32 sound_id, u32 volume, u32 pan);
s32 poll_input_device(void);

#endif
