#ifndef _AKAO_CMD_H
#define _AKAO_CMD_H

#include "common.h"

s32 akao_streaming_upload_tick(u8* source, u32 avail, s32 wait_for_spu);

s32 akao_cmd_f0(void);
s32 akao_cmd_f1(void);
s32 akao_cmd_a8(s32 arg0);

#endif
