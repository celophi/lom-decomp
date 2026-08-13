#include "akao.h"
#include "akao_driver.h"

/*
 * Declared as arrays on purpose: with -G4, a plain `extern s32` is treated as
 * sdata-cheap, so gcc folds `&sym +/- 4` into new symbolic constants (extra
 * lui pairs) and emits assembler-macro loads/stores (`lui at`). The unknown-
 * size array form keeps the register arithmetic and the compiler-split
 * `lui v0` forms the target has (measured: +21 exact rows).
 */
extern s32 D_8004F76C[];
extern s32 D_8004D404[];
extern s32 D_8004F834[];

extern void func_800257E0(s32 mask, s32 arg1);
extern void func_80025500(AkaoChannelState *channel, s32 channel_mask, s32 static_voice_mask, u32 *voice_mask);
extern void func_80024F60(AkaoChannelState *channel, s32 channel_bit);
extern void func_80025F48(s32 *dest, s32 target, s32 current, s32 step);
extern void func_8002613C(s32 arg0, s32 arg1);
extern void func_800260CC(u16 arg0);
extern void spu_apply_voice_updates(u32 voice, void *params, s32 flags);
extern void spu_set_reverb_enable(u32 voice_mask);
extern void spu_set_noise_enable(u32 voice_mask);
extern void spu_set_pitch_modulation_enable(u32 voice_mask);
extern void spu_set_key_on(u32 voice_mask);

/**
 * @brief Per-tick AKAO note-off / voice-deallocation pass: releases voices
 *        for notes no longer sounding on the song channel(s) and the SFX
 *        channel array, then applies pending SPU hardware updates (LFO
 *        recompute, key-off frequency, reverb/noise/pitch-mod fades, and
 *        key-on) gated by g_akao_driver_flags.unk8.
 * @note 99.97% match (gcc280_g4, 286/288 exact rows, +0 insns). The only
 *       residual is 2 argdiff rows: the master-state addiu/sw in the
 *       dealloc_mask1 block use s1 (sfx_channel) where the target uses v0.
 *       Shapes required to match, each measured via probe_variants:
 *       - array-form extern declarations above (+21 exact);
 *       - a SINGLE loop pointer with plain field accesses in the SFX loop:
 *         gcc's loop.c combines the flags/voice/update_flags mem givs into
 *         one reduced giv anchored at flags (channel+0x34), reproducing the
 *         target's second walker exactly. A hand-built second pointer makes
 *         the giv family anchor at +0xC8 and adds a third walker instead;
 *       - ONE variable (dealloc_mask0) reused for the channel-0 dealloc
 *         mask, the SFX active mask, and the driver update flags (+20): all
 *         three land in s3 like the target. Separate variables give each a
 *         different callee-saved reg;
 *       - `(static_voice_mask1 | mask)` inlined in both dealloc_mask0 and
 *         static_voice_mask0 (no combined_mask local, +10);
 *       - `dealloc_mask1 = 0` initialized before `static_voice_mask1 = 0`
 *         (+2); `key` as s32 not u16 (+3);
 *       - routing the master-state store through sfx_channel in the
 *         dealloc_mask1 block (+29): the early def raises sfx_channel's
 *         priority above bit (refs 12 vs 11, so it wins s1) and creates the
 *         sfx_channel x static_voice_mask0 conflict that forces
 *         static_voice_mask0 into s2 and static_voice_mask1 into s4. The
 *         target emits that store from v0 (no sfx_channel def there), so the
 *         original produced the same allocation by some other means - that
 *         is the open 2-row residual.
 *       - fade block anchored at &D_8004F834 with `fade1 = fade0 - 1` and
 *         relative loads, matching the target's addiu s2, s0, -4 shape.
 */
void func_800258B8(void)
{
    u32 voice_mask;
    s32 mask;
    s32 dealloc_mask1;
    s32 static_voice_mask1;
    s32 key_on_submask1;
    s32 dealloc_mask0;
    s32 static_voice_mask0;
    s32 key_on_submask0;
    s32 bit;
    AkaoChannelState *sfx_channel;
    typeof(g_akao_seq_channel0->w04.song) *song_ptr;
    AkaoChannelState *old_channel0;
    s32 *fade0;
    s32 *fade1;
    s32 lfo_arg;
    s32 key;

    dealloc_mask1 = 0;
    static_voice_mask1 = 0;
    voice_mask = 0;
    mask = (g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10) | D_8004F76C[0];

    if (!(g_akao_seq_channel0->w04.song.active_mask & g_akao_seq_channel0->w04.song.key_on_mask))
    {
        if (g_akao_seq_channel1 != NULL)
        {
            if (g_akao_seq_channel1->w04.song.active_mask & g_akao_seq_channel1->w04.song.key_on_mask)
            {
                goto call_note_off;
            }
            goto dealloc_channel1;
        }
    }
    else
    {
    call_note_off:
        func_800257E0(mask, D_8004F76C[0]);
    dealloc_channel1:
        if (g_akao_seq_channel1 != NULL)
        {
            g_akao_seq_channel0 = g_akao_seq_channel1;
            dealloc_mask1 = g_akao_seq_channel1->w04.song.active_mask & g_akao_seq_channel1->note_on_mask & ~(g_akao_seq_channel1->w04.song.static_voice_mask & mask);
            static_voice_mask1 = g_akao_seq_channel1->w04.song.static_voice_mask;
            key_on_submask1 = dealloc_mask1 & g_akao_seq_channel1->w04.song.voice_alloc_low_mask;
            static_voice_mask1 = dealloc_mask1 & static_voice_mask1 & ~mask;
            if (key_on_submask1 != 0)
            {
                func_80025500((AkaoChannelState *)g_akao_pending_channels, key_on_submask1, static_voice_mask1, &voice_mask);
                song_ptr = &g_akao_seq_channel0->w04.song;
                dealloc_mask1 &= ~song_ptr->voice_alloc_low_mask;
                g_akao_seq_channel0->w04.song.key_on_mask &= ~g_akao_seq_channel0->w04.song.voice_alloc_low_mask;
            }
            g_akao_seq_channel0 = &g_akao_seq_master_state;
        }
    }

    dealloc_mask0 = g_akao_seq_channel0->w04.song.active_mask & g_akao_seq_channel0->note_on_mask & ~(g_akao_seq_channel0->w04.song.static_voice_mask & (static_voice_mask1 | mask));
    static_voice_mask0 = dealloc_mask0 & g_akao_seq_channel0->w04.song.static_voice_mask & ~(static_voice_mask1 | mask);
    key_on_submask0 = dealloc_mask0 & g_akao_seq_channel0->w04.song.voice_alloc_low_mask;
    if (key_on_submask0 != 0)
    {
        func_80025500((AkaoChannelState *)g_akao_seq_channels, key_on_submask0, static_voice_mask0, &voice_mask);
        song_ptr = &g_akao_seq_channel0->w04.song;
        dealloc_mask0 &= ~song_ptr->voice_alloc_low_mask;
        g_akao_seq_channel0->w04.song.key_on_mask &= ~g_akao_seq_channel0->w04.song.voice_alloc_low_mask;
    }

    if ((g_akao_seq_channel1 != NULL) && (dealloc_mask1 != 0))
    {
        g_akao_seq_channel0 = g_akao_seq_channel1;
        func_80025500((AkaoChannelState *)g_akao_pending_channels, dealloc_mask1, static_voice_mask1 & ~static_voice_mask0, &voice_mask);
        old_channel0 = g_akao_seq_channel0;
        sfx_channel = &g_akao_seq_master_state;
        g_akao_seq_channel0 = sfx_channel;
        old_channel0->w04.song.key_on_mask = 0;
    }

    if (dealloc_mask0 != 0)
    {
        func_80025500((AkaoChannelState *)g_akao_seq_channels, dealloc_mask0, static_voice_mask0, &voice_mask);
        g_akao_seq_channel0->w04.song.key_on_mask = 0;
    }

    dealloc_mask0 = g_akao_sfx_control.unk0 & g_akao_sfx_control.unk8;
    if (dealloc_mask0 != 0)
    {
        bit = 0x1000;
        sfx_channel = (AkaoChannelState *)g_sfx_channels;
        voice_mask |= g_akao_sfx_control.unk4;
        do
        {
            if (dealloc_mask0 & bit)
            {
                func_80024F60(sfx_channel, bit);
                if (sfx_channel->update_flags != 0)
                {
                    spu_apply_voice_updates(sfx_channel->voice, &sfx_channel->voice, sfx_channel->flags);
                }
                dealloc_mask0 &= ~bit;
            }
            bit <<= 1;
            sfx_channel++;
        } while (dealloc_mask0 != 0);
        D_8004D404[0] = 0;
    }

    dealloc_mask0 = g_akao_driver_flags.unk8;
    if (dealloc_mask0 & 0x80)
    {
        lfo_arg = (s32)(g_akao_seq_channel0->unk48 << 4) >> 16;
        func_8002613C(lfo_arg, lfo_arg);
        g_akao_driver_flags.unk8 &= ~0x80;
    }

    if (dealloc_mask0 & 0x10)
    {
        if (g_akao_sfx_control.unk0 != 0)
        {
            key = g_akao_sfx_control.unk28;
        }
        else
        {
            key = g_akao_seq_channel0->noise_freq;
        }
        func_800260CC(key);
        g_akao_driver_flags.unk8 &= ~0x10;
    }

    if (dealloc_mask0 & 0x100)
    {
        fade0 = D_8004F834;
        func_80025F48(fade0, g_akao_seq_channel1->reverb_mask, g_akao_seq_channel0->reverb_mask, g_akao_sfx_control.reverb_mask);
        fade1 = fade0 - 1;
        func_80025F48(fade1, g_akao_seq_channel1->noise_mask, g_akao_seq_channel0->noise_mask, g_akao_sfx_control.noise_mask);
        func_80025F48(fade0 + 1, g_akao_seq_channel1->pitch_mod_mask, g_akao_seq_channel0->pitch_mod_mask, g_akao_sfx_control.pitch_mod_mask);
        spu_set_reverb_enable(fade0[-1]);
        spu_set_noise_enable(fade1[1]);
        spu_set_pitch_modulation_enable(fade1[2]);
        g_akao_driver_flags.unk8 &= ~0x100;
    }

    if (voice_mask != 0)
    {
        spu_set_key_on(voice_mask);
    }
}
