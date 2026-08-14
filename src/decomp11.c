#include "decomp4.h"
#include "decomp9.h"

extern AkaoChannelState* g_akao_seq_channel0;

/**
 * @see decomp.me (100%)
 */
void func_8002D0DC(AkaoChannelState* channel, s32 channel_mask)
{
    g_akao_seq_channel0->w04.song.voice_alloc_low_mask &= ~channel_mask;
}
