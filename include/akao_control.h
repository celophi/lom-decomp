#ifndef _AKAO_CONTROL_H
#define _AKAO_CONTROL_H

#include "akao.h"
#include "akao_driver.h"

/* Public entry points of akao_control.c used by the other driver units. */

void akao_sfx_stop_channels(s32 sfx_id, s32 mode);
void akao_sfx_play(AkaoCommandParam* params, u8* seq_data0, u8* seq_data1, s32 skip_stop);
void akao_seq_flag_volume_update(AkaoChannelState* song, AkaoChannelState* channels);
void akao_apply_reverb_type(s32 reverb_type);

#endif
