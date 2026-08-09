#include "akao.h"
#include "akao_driver.h"
#include "decomp4.h"

extern s16 D_8003D37C[];

typedef struct
{
    s16 value;
    s16 unk2;
    s16 relative_offset;
} AkaoLfoSample;

void func_80024F60(AkaoChannelState* channel)
{
    s32 flags;
    s32 lfo_pitch;
    s32 value;
    s32 pan;
    s32 pitch_value;
    s32 master_scale;
    s32 next_flags;

    flags = channel->flags;
    lfo_pitch = (((s16*)&channel->unk48)[1] * (channel->volume >> 8)) >> 7;

    if (flags & 1)
    {
        channel->pitch_lfo_restart--;
        if (channel->pitch_lfo_restart == 0)
        {
                channel->pitch_lfo_restart = *((u16*)((u8*)channel + 0xA6));
                {
                    AkaoLfoSample* cursor;
                    s16* waveform;

                    cursor = (AkaoLfoSample*)channel->unk1C;
                    if ((cursor->value == 0) && (cursor->unk2 == 0))
                    {
                        channel->unk1C = (s32)(((u8*)cursor) + (cursor->relative_offset * 2));
                    }

                    waveform = (s16*)channel->unk1C;
                    value = ((s32)channel->pitch_lfo_depth_scaled * *waveform++) >> 16;
                    channel->unk1C = (s32)waveform;
                }
                if (value != channel->pitch_lfo_value)
                {
                    channel->pitch_lfo_value = value;
                    channel->update_flags |= 0x10;
                    if (value >= 0)
                    {
                        channel->pitch_lfo_value = value << 1;
                    }
            }
        }
    }

    if (flags & 2)
    {
        channel->volume_lfo_restart--;
        if (channel->volume_lfo_restart == 0)
        {
                channel->volume_lfo_restart = *((u16*)((u8*)channel + 0xBA));
                {
                    AkaoLfoSample* cursor;
                    s16* waveform;

                    cursor = (AkaoLfoSample*)channel->tempo;
                    if ((cursor->value == 0) && (cursor->unk2 == 0))
                    {
                        channel->tempo = (u32)(((u8*)cursor) + (cursor->relative_offset * 2));
                    }

                    waveform = (s16*)channel->tempo;
                    value = (lfo_pitch * (channel->volume_lfo_depth >> 8) << 9) >> 16;
                    value = (value * *waveform++) >> 15;
                    channel->tempo = (u32)waveform;
                }
                if (value != channel->volume_lfo_value)
                {
                    channel->volume_lfo_value = value;
                    channel->update_flags |= 3;
            }
        }
    }

    if (flags & 4)
    {
        channel->pan_lfo_restart--;
        if (channel->pan_lfo_restart == 0)
        {
            channel->pan_lfo_restart = channel->pan_lfo_period;
            {
                AkaoLfoSample* cursor;
                s16* waveform;

                cursor = (AkaoLfoSample*)channel->tempo_step;
                if ((cursor->value == 0) && (cursor->unk2 == 0))
                {
                    channel->tempo_step = (s32)(((u8*)cursor) + (cursor->relative_offset * 2));
                }

                waveform = (s16*)channel->tempo_step;
                value = ((channel->pan_lfo_depth >> 8) * *waveform++) >> 15;
                channel->tempo_step = (s32)waveform;
            }
            if (value != channel->pan_lfo_value)
            {
                channel->pan_lfo_value = value;
                channel->update_flags |= 3;
            }
        }
    }

    if (flags & 0x20)
    {
        value = (s16)(*((u16*)((u8*)channel - 0xC)) << 1);
        lfo_pitch = (value * (channel->volume >> 8)) >> 7;
        channel->update_flags |= 3;
    }

    if (channel->update_flags & 3)
    {
        pan = 0x80;
        lfo_pitch += channel->volume_lfo_value;
        if (!(channel->tempo_acc & 0x02000000))
        {
            value = (s8)(channel->volume_scale >> 8);
            lfo_pitch = (lfo_pitch * value) >> 7;
            pan = (((channel->pan + channel->pan_bias) >> 8) + channel->pan_lfo_value + 0x80) & 0xFF;
        }

        if (D_8004F754 == 2)
        {
            value = (lfo_pitch * D_8003D47C) >> 15;
            channel->spu_volume_right = value;
            channel->spu_volume_left = value;
        }
        else
        {
            channel->spu_volume_left = (lfo_pitch * D_8003D37C[pan]) >> 15;
            channel->spu_volume_right = (lfo_pitch * D_8003D37C[pan ^ 0xFF]) >> 15;
        }
    }

    if (flags & 0x10)
    {
        pitch_value = *((u16*)((u8*)channel - 0xC)) + channel->pitch_lfo_value + *((s16*)((u8*)channel + 0x32));
        if (!(channel->tempo_acc & 0x02000000))
        {
            master_scale = (channel->noise_mask & 0xFF00) >> 8;
            if (master_scale != 0)
            {
                if (master_scale < 0x80)
                {
                    pitch_value += (pitch_value * master_scale) >> 7;
                }
                else
                {
                    pitch_value = (pitch_value * master_scale) >> 8;
                }
            }
        }
        channel->spu_pitch = (*((u16*)((u8*)channel + 0x54)) + pitch_value) & 0x3FFF;
        channel->update_flags |= 0x10;
        return;
    }

    if (channel->update_flags & 0x10)
    {
        pitch_value = channel->pitch + channel->pitch_lfo_value + *((s16*)((u8*)channel + 0x32));
        if (!(channel->tempo_acc & 0x02000000))
        {
            master_scale = (channel->noise_mask & 0xFF00) >> 8;
            if (master_scale != 0)
            {
                if (master_scale < 0x80)
                {
                    pitch_value += (pitch_value * master_scale) >> 7;
                }
                else
                {
                    pitch_value = (pitch_value * master_scale) >> 8;
                }
            }
        }
        channel->spu_pitch = (*((u16*)((u8*)channel + 0x54)) + pitch_value) & 0x3FFF;
    }
}