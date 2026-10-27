#ifndef _GAME_AUDIO_H
#define _GAME_AUDIO_H

#include "common.h"

/** @brief Music resource selected by each field music index. */
extern u8 g_music_track_table[];
/** @brief Resident storage for an AKAO sequence or a section-offset song container. */
extern u8 g_resident_song_buffer[];
/** @brief Section offsets in g_resident_song_buffer while it holds a song container. */
extern s32 g_resident_song_section_offsets[];
extern s16 g_game_diagnostic_status;

void fade_out_current_song(void);
void load_and_play_song(s32 song_index);

#define OVERLAY_SONG_BUFFER_SIZE 0x4000

#define SFX_ERROR 0x78
#define SFX_CURSOR 0x7D
#define SFX_CONFIRM 0x7E
#define SFX_CANCEL 0x7F

/** @brief record_game_diagnostic status of an error report. */
#define DIAG_ERROR 0x8001

void record_game_diagnostic(s32 status, s32 code, s32 arg0, s32 arg1);

#endif
