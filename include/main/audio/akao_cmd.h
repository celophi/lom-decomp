#ifndef _AKAO_CMD_H
#define _AKAO_CMD_H

#include "common.h"
#include "main/audio/akao.h"

/** @brief Playback groups selected by akao_pause_audio and akao_resume_audio. */
typedef enum AkaoAudioTarget
{
    AKAO_AUDIO_ALL = 0,
    AKAO_AUDIO_SONG = 1,
    AKAO_AUDIO_SFX = 2,
    AKAO_AUDIO_XA = 3
} AkaoAudioTarget;

/*
 * Game-facing AKAO driver interface. Each wrapper stores its arguments in
 * g_akao_cmd_params and queues one driver command. akao_register_bank,
 * akao_play_song, akao_set_song_volume and akao_upload_bank_blocking are
 * declared in akao.h.
 */

s32 akao_init(void);
s32 akao_shutdown(void);

/* Songs */
void akao_stop_song(s32 stop_mode);
void akao_suspend_song(void);
s32 akao_start_song_channels(void* sequence, s32 channel_mask, s32 unused);
s32 akao_switch_song(void* sequence, s32 volume);
void akao_play_song_with_ticks(s32 sequence, s32 ticks);
s32 akao_fade_song_volume(s32 song_handle, s32 ticks, s32 volume);
s32 akao_fade_song_volume_from(s32 song_handle, s32 ticks, s32 start_volume, s32 volume);
void akao_mute_song_channels(s32 channel_mask);
void akao_set_song_condition(s32 value);
s32 akao_stop_all_songs(void);

/* Sound effects */
void akao_play_sfx(s32 sound_id, s32 tag, s32 pan, s32 volume);
uintptr_t akao_play_sfx_from_buffer(AkaoHeader* buffer, s32 tag, s32 pan, s32 volume);
void akao_play_sound(s32 sound_id);
void akao_stop_sfx(s32 sound_id, s32 tag_mask);
s32 akao_get_active_sfx_ids(void);
s32 akao_is_sfx_playing(s32 sound_id);
void akao_set_sfx_volume(s32 sound_id, s32 tag_mask, s32 volume);
void akao_fade_sfx_volume(s32 sound_id, s32 tag_mask, s32 ticks, s32 volume);
void akao_set_sfx_pan(s32 sound_id, s32 tag_mask, s32 pan);
void akao_fade_sfx_pan(s32 sound_id, s32 tag_mask, s32 ticks, s32 pan);
s32 akao_set_sfx_pitch_bend(s32 sound_id, s32 tag_mask, s32 bend);
s32 akao_fade_sfx_pitch_bend(s32 sound_id, s32 tag_mask, s32 ticks, s32 bend);
s32 akao_set_all_sfx_volume(s32 volume);
void akao_fade_all_sfx_volume(s32 ticks, s32 volume);
void akao_set_all_sfx_pan(s32 pan);
void akao_fade_all_sfx_pan(s32 ticks, s32 pan);
void akao_set_all_sfx_pitch_bend(s32 bend);
void akao_fade_all_sfx_pitch_bend(s32 ticks, s32 bend);
s32 akao_release_all_sfx(void);

/* Output, pause and master controls */
void akao_set_mono_output(s32 mono);
void akao_pause_audio(u32 target);
void akao_resume_audio(u32 target);
s32 akao_set_cd_volume(s32 volume);
s32 akao_fade_cd_volume(s32 ticks, s32 volume);
s32 akao_fade_cd_volume_from(s32 ticks, s32 start_volume, s32 volume);
s32 akao_set_master_pan(s32 pan);
s32 akao_fade_master_pan(s32 ticks, s32 pan);
s32 akao_fade_master_pan_from(s32 ticks, s32 start_pan, s32 pan);
s32 akao_set_master_volume(s32 volume);
s32 akao_fade_master_volume(s32 ticks, s32 volume);
void akao_fade_master_volume_from(s32 ticks, s32 start_volume, s32 volume);
s32 akao_set_master_pan_and_volume(s32 value);
s32 akao_fade_master_pan_and_volume(s32 ticks, s32 value);
s32 akao_fade_master_pan_and_volume_from(s32 ticks, s32 start_value, s32 value);
s32 akao_set_cd_mix(s32 volume);

/* Instrument banks */
s32 akao_get_xfer_state(void);
s32 akao_reset_xfer_state(void);
s32 akao_streaming_upload_tick(u8* source, u32 avail, s32 wait_for_spu);
s32 akao_load_bank(AkaoBankHeader* bank, s32 wait_for_completion);
s32 akao_upload_bank_slot(void* bank, s32 slot, s32 wait_for_completion);
s32 akao_load_bank_slot(void* bank, s32 slot, s32 wait_for_completion);
s32 akao_load_upper_bank_slot(void* bank, s32 slot, s32 wait_for_completion);

/* Streamed (XA program) voices */
void akao_play_xa_buffer(AkaoHeader* buffer, s32 pan, s32 use_reverb);
s32 akao_stop_xa(void);
s32 akao_set_xa_volume(s32 volume);
s32 akao_fade_xa_volume(s32 ticks, s32 volume);
s32 akao_set_xa_pan(s32 pan);
s32 akao_upload_xa_program(void* buffer, s32 upper_slot);
s32 akao_play_staged_xa(s32 pan, s32 use_reverb);
void akao_play_xa_one_shot(void* buf, s32 pan, s32 upper_slot, s32 use_reverb);
s32 akao_start_xa_stream(void* ring_base, u32 byte_count);
s32 akao_xa_advance_frame(void);
s32 akao_xa_get_position(void);

#endif
