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

#endif
