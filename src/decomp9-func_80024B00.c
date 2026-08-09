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

void func_80024B00(AkaoChannelState* channel)
{
    s32 flags;
    s32 lfo_pitch;
    s32 value;
    s32 scale;
    s32 pan;
    AkaoLfoSample* cursor;

    flags = channel->flags;
    lfo_pitch = (((s16*)&channel->unk48)[1] * (channel->volume >> 8)) >> 7;

    if (flags & 1)
    {
        if (channel->pitch_lfo_delay_ticks == 0)
        {
            channel->pitch_lfo_restart--;
            if (channel->pitch_lfo_restart == 0)
            {
                channel->pitch_lfo_restart = *((u16*)((u8*)channel + 0xA6));
                cursor = (AkaoLfoSample*)channel->unk1C;
                if ((cursor->value == 0) && (cursor->unk2 == 0))
                {
                    channel->unk1C = (s32)(((u8*)cursor) + (cursor->relative_offset * 2));
                }

                cursor = (AkaoLfoSample*)channel->unk1C;
                channel->unk1C = (s32)(((u8*)cursor) + 2);
                value = ((s32)channel->pitch_lfo_depth_scaled * cursor->value) >> 16;
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
    }

    if (flags & 2)
    {
        if (channel->volume_lfo_delay_ticks == 0)
        {
            channel->volume_lfo_restart--;
            if (channel->volume_lfo_restart == 0)
            {
                channel->volume_lfo_restart = *((u16*)((u8*)channel + 0xBA));
                cursor = (AkaoLfoSample*)channel->tempo;
                if ((cursor->value == 0) && (cursor->unk2 == 0))
                {
                    channel->tempo = (u32)(((u8*)cursor) + (cursor->relative_offset * 2));
                }

                cursor = (AkaoLfoSample*)channel->tempo;
                value = (lfo_pitch * (channel->volume_lfo_depth >> 8) << 9) >> 16;
                scale = (value * cursor->value) >> 15;
                channel->tempo = (u32)(((u8*)cursor) + 2);
                if (scale != channel->volume_lfo_value)
                {
                    channel->volume_lfo_value = scale;
                    channel->update_flags |= 3;
                }
            }
        }
    }

    if (flags & 4)
    {
        channel->pan_lfo_restart--;
        if (channel->pan_lfo_restart == 0)
        {
            channel->pan_lfo_restart = channel->pan_lfo_period;
            cursor = (AkaoLfoSample*)channel->tempo_step;
            if ((cursor->value == 0) && (cursor->unk2 == 0))
            {
                channel->tempo_step = (s32)(((u8*)cursor) + (cursor->relative_offset * 2));
            }

            cursor = (AkaoLfoSample*)channel->tempo_step;
            scale = (((channel->pan_lfo_depth >> 8) * cursor->value) >> 15);
            channel->tempo_step = (s32)(((u8*)cursor) + 2);
            if (scale != channel->pan_lfo_value)
            {
                channel->pan_lfo_value = scale;
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
        pan = ((channel->pan >> 8) + channel->pan_lfo_value) & 0xFF;
        lfo_pitch = ((lfo_pitch + channel->volume_lfo_value) * (*((u16*)((u8*)g_akao_seq_channel0 + 0x52)) & 0x7F)) >> 7;
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
        value = *((u16*)((u8*)channel - 0xC)) + channel->pitch_lfo_value + *((s16*)((u8*)channel + 0x32));
        scale = (g_akao_mastervol_acc & 0xFF0000) >> 16;
        if ((g_akao_mastervol_acc & 0xFF0000) != 0)
        {
            if (scale < 0x80)
            {
                value += (value * scale) >> 7;
            }
            else
            {
                value = (value * scale) >> 8;
            }
        }
        channel->spu_pitch = (*((u16*)((u8*)channel + 0x54)) + value) & 0x3FFF;
        channel->update_flags |= 0x10;
        return;
    }

    if (channel->update_flags & 0x10)
    {
        value = channel->pitch + channel->pitch_lfo_value + *((s16*)((u8*)channel + 0x32));
        scale = (g_akao_mastervol_acc & 0xFF0000) >> 16;
        if ((g_akao_mastervol_acc & 0xFF0000) != 0)
        {
            if (scale < 0x80)
            {
                value += (value * scale) >> 7;
            }
            else
            {
                value = (value * scale) >> 8;
            }
        }
        channel->spu_pitch = (*((u16*)((u8*)channel + 0x54)) + value) & 0x3FFF;
    }
}