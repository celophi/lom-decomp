#ifndef _AKAO_CONTROL_H
#define _AKAO_CONTROL_H

#include "main/audio/akao.h"
#include "akao_driver.h"

/* Public entry points of akao_control.c used by the other driver units. */

void akao_sfx_stop_channels(s32 sfx_id, s32 mode);
void akao_sfx_play(AkaoCommandParam* params, u8* first_sequence, u8* second_sequence, s32 skip_stop);
void akao_seq_flag_volume_update(AkaoSongState* song, AkaoChannelState* channels);
void akao_apply_reverb_type(s32 reverb_type);
s32 akao_send_command(u32 opcode);

#endif
