
#include "akao_driver.h"

typedef struct VoiceAllocEntry {
    s32 unk0;
    s16 unk4;
    s16 unk6;
} VoiceAllocEntry;

extern VoiceAllocEntry D_8004C1A0[];
extern AkaoChannelState* D_8004F7C0[];

void func_80025500(

    AkaoChannelState* channel,
    s32 channel_mask,
    s32 static_voice_mask,
    u32* voice_mask)
{
    s32 bit;
    s32 channel_index;
    s32 voice;
    s32 key_on_mask;
    s32 one;
    s32 inv_bit;

    bit = 1;
    channel_index = 0;
    one = bit;
    key_on_mask =
        channel_mask & g_akao_seq_channel0->w04.song.key_on_mask;

    do {
        if (channel_mask & bit) {
            func_80024B00(channel, bit);

            inv_bit = ~bit;

            if (channel->update_flags != 0) {
                if (D_8003EC6C & bit) {
                    channel->spu_volume_right = 0;
                    channel->spu_volume_left = 0;
                }

                if (key_on_mask & bit) {
                    if (static_voice_mask & bit) {
                        *voice_mask |= one << channel_index;
                        channel->voice = channel_index;
                    } else {
                        s32 use_low;

                        use_low =
                            (g_akao_seq_channel0
                                 ->w04.song.voice_alloc_low_mask &
                             bit) != 0;

                        voice = func_80025498(use_low);

                        if (voice == 0x18) {
                            g_akao_seq_channel0->seq_cursor =
                                (u8*)((u32)g_akao_seq_channel0
                                          ->seq_cursor |
                                      2);

                            voice = func_800253E8(use_low);

                            if (voice == 0x18) {
                                channel->voice = voice;

                                g_akao_seq_channel0->seq_cursor =
                                    (u8*)((u32)g_akao_seq_channel0
                                              ->seq_cursor |
                                          1);
                            } else {
                                *voice_mask |= one << voice;
                                channel->voice = voice;
                                D_8004C1A0[voice].unk4 = 0x7FFF;
                            }
                        } else {
                            *voice_mask |= one << voice;
                            channel->voice = voice;
                            D_8004C1A0[voice].unk4 = 0x7FFF;
                        }
                    }

                    if (channel->voice < 0x18U) {
                        spu_write_voice_params(
                            channel->voice,
                            (void*)&channel->voice,
                            channel->spu_volume_scale);

                        D_8004F7C0[channel->voice] =
                            g_akao_seq_channel0;

                        g_akao_driver_flags.unk8 |= 0x100;
                    }
                } else if (channel->voice < 0x18U) {
                    spu_apply_voice_updates(
                        channel->voice,
                        (void*)&channel->voice,
                        channel->flags);
                }

                inv_bit = ~bit;
            }

            channel_mask &= inv_bit;
        }

        bit <<= one;
        channel++;
        channel_index += one;
    } while (channel_mask != 0);
}
