#ifndef _GAME_AUDIO_H
#define _GAME_AUDIO_H

#include "common.h"

/** @brief Music resource selected by each field music index. */
extern u8 g_music_track_table[];
extern s16 g_game_diagnostic_status;

void fade_out_current_song(void);
void load_and_play_song(s32 song_index);
/** @brief record_game_diagnostic status of an error report. */
#define DIAG_ERROR 0x8001

void record_game_diagnostic(s32 status, s32 code, s32 arg0, s32 arg1);

#endif
