#ifndef _AKAO_CMD_H
#define _AKAO_CMD_H

#include "common.h"

s32 akao_init(void);
s32 akao_shutdown(void);
s32 akao_streaming_upload_tick(u8* source, u32 avail, s32 wait_for_spu);

s32 akao_cmd_f0(void);
s32 akao_cmd_f1(void);
s32 akao_cmd_a8(s32 arg0);

void akao_stop_song(s32 stop_mode);
void akao_play_sfx(s32 sound_id, s32 parameter, s32 pan, s32 volume);
void akao_cmd_21(s32 value0, s32 value1);
void akao_cmd_a1(s32 value0, s32 value1, s32 value2, s32 value3);
void akao_cmd_a3(s32 value0, s32 value1, s32 value2, s32 value3);
void akao_cmd_a9(s32 value0, s32 value1);

/**
 * @brief Dispatch one of the four AKAO playback control commands.
 * @param mode Selects command 0x98, 0x9A, 0x9C or 0x9E (0..3).
 */
void akao_cmd_98_9a_9c_9e(u32 mode);
s32 akao_cmd_c8(s32 value0);
s32 akao_xa_setup_panning(s32 volume);
s32 akao_cmd_e4_set_cd_volume(s32 value0);
s32 akao_cmd_e8_start_xa_stream(s32 stream_id, u32 byte_count);
s32 akao_xa_advance_frame(void);
s32 akao_xa_get_position(void);

#endif
