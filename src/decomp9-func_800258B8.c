#include "akao.h"
#include "akao_driver.h"

extern s32 D_8004F76C;
extern s32 D_8004D404;

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
 * @note 96.15% match (gcc280_g4, 232/288 exact rows), not yet 100%.
 *       No structural (control-flow) differences remain. Three shapes are
 *       required to match and were each measured via probe_variants:
 *         - `song_ptr = &channel->w04.song` for the FIRST voice_alloc_low_mask
 *           read at each dealloc site: the target reads that field twice (once
 *           per statement) with the same base register. A plain repeated field
 *           read is CSE-folded to one load; routing the first read through a
 *           sub-struct pointer defeats CSE at cse-time, and combine later folds
 *           the +4 back into the addressing mode, leaving two `lw x,8(base)`
 *           exactly as the target. (`*(&field)` does NOT work - gcc folds it at
 *           tree level.) Worth +14 exact rows across the two sites.
 *         - the mask/D_8004F76C load is combined into one expression so the
 *           constant is read at the point of use, not hoisted to entry.
 *         - the SFX update_flags test is hoisted into `static_voice_mask1`
 *           (dead by then) as a temp; +12 exact rows.
 *       `dealloc_mask1` is reused to hold the constant 0xCC (the flags_ptr
 *       offset to AkaoChannelState::update_flags): the variable offset stops
 *       gcc strength-reducing flags_ptr+0xCC into a redundant giv (a 5th loop
 *       pointer the target lacks). Removing it costs ~11 rows.
 *       Remaining residual is a single coupled saved-register permutation:
 *       target wants dealloc_mask1=s0 / static_voice_mask1=s4 /
 *       dealloc_mask0=s3 / driver_flags=s3, we get them rotated. Root cause:
 *       the 0xCC reuse above extends dealloc_mask1's live range into the SFX
 *       loop, where it conflicts with the loop's saved regs and is forced off
 *       s0 (explain_conflict / simulate_alloc: no single source edit flips it -
 *       "try pairs by hand"). The fade block's base-pointer anchor
 *       (D_8004F834 vs D_8004F830) is a knock-on of the same allocation.
 *       Full iteration history in working/func_800258B8/code.c.
 */
void func_800258B8(void)
{
    u32 voice_mask;
    s32 mask;
    s32 combined_mask;
    s32 dealloc_mask1;
    s32 static_voice_mask1;
    s32 key_on_submask1;
    s32 dealloc_mask0;
    s32 static_voice_mask0;
    s32 key_on_submask0;
    s32 sfx_active_mask;
    s32 bit;
    AkaoChannelState *sfx_channel;
    s32 *flags_ptr;
    typeof(g_akao_seq_channel0->w04.song) *song_ptr;
    AkaoChannelState *old_channel0;
    s32 *fade0;
    s32 *fade1;
    s32 driver_flags;
    s32 lfo_arg;
    u16 key;

    static_voice_mask1 = 0;
    dealloc_mask1 = 0;
    voice_mask = 0;
    mask = (g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10) | D_8004F76C;

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
        func_800257E0(mask, D_8004F76C);
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
                func_80025500((AkaoChannelState *)g_akao_pending_channels,key_on_submask1, static_voice_mask1, &voice_mask);
                song_ptr = &g_akao_seq_channel0->w04.song;
                dealloc_mask1 &= ~song_ptr->voice_alloc_low_mask;
                g_akao_seq_channel0->w04.song.key_on_mask &= ~g_akao_seq_channel0->w04.song.voice_alloc_low_mask;
            }
            g_akao_seq_channel0 = &g_akao_seq_master_state;
        }
    }

    combined_mask = static_voice_mask1 | mask;
    dealloc_mask0 = g_akao_seq_channel0->w04.song.active_mask & g_akao_seq_channel0->note_on_mask & ~(g_akao_seq_channel0->w04.song.static_voice_mask & combined_mask);
    static_voice_mask0 = dealloc_mask0 & g_akao_seq_channel0->w04.song.static_voice_mask & ~combined_mask;
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
        func_80025500((AkaoChannelState *)g_akao_pending_channels,dealloc_mask1, static_voice_mask1 & ~static_voice_mask0, &voice_mask);
        old_channel0 = g_akao_seq_channel0;
        g_akao_seq_channel0 = &g_akao_seq_master_state;
        old_channel0->w04.song.key_on_mask = 0;
    }

    if (dealloc_mask0 != 0)
    {
        func_80025500((AkaoChannelState *)g_akao_seq_channels, dealloc_mask0, static_voice_mask0, &voice_mask);
        g_akao_seq_channel0->w04.song.key_on_mask = 0;
    }

    sfx_active_mask = g_akao_sfx_control.unk0 & g_akao_sfx_control.unk8;
    if (sfx_active_mask != 0)
    {
        bit = 0x1000;
        dealloc_mask1 = 0xCC;
        sfx_channel = (AkaoChannelState *)g_sfx_channels;
        flags_ptr = &sfx_channel->flags;
        voice_mask |= g_akao_sfx_control.unk4;
        do
        {
            if (sfx_active_mask & bit)
            {
                func_80024F60(sfx_channel, bit);
                static_voice_mask1 = *(s32 *)((u8 *)flags_ptr + dealloc_mask1) != 0;
                if (static_voice_mask1)
                {
                    spu_apply_voice_updates(*(u32 *)((u8 *)flags_ptr + 0xC8), (void *)((u8 *)sfx_channel + 0xFC), *flags_ptr);
                }
                sfx_active_mask &= ~bit;
            }
            bit <<= 1;
            flags_ptr = (s32 *)((u8 *)flags_ptr + 0x118);
            sfx_channel = (AkaoChannelState *)((u8 *)sfx_channel + 0x118);
        } while (sfx_active_mask != 0);
        D_8004D404 = 0;
    }

    driver_flags = g_akao_driver_flags.unk8;
    if (driver_flags & 0x80)
    {
        lfo_arg = (s32)(g_akao_seq_channel0->unk48 << 4) >> 16;
        func_8002613C(lfo_arg, lfo_arg);
        g_akao_driver_flags.unk8 &= ~0x80;
    }

    if (driver_flags & 0x10)
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

    if (driver_flags & 0x100)
    {
        fade0 = (s32 *)off(D_8004F830, 4);
        func_80025F48(fade0, g_akao_seq_channel1->reverb_mask, g_akao_seq_channel0->reverb_mask, g_akao_sfx_control.reverb_mask);
        fade1 = (s32 *)off(D_8004F830, 0);
        func_80025F48(fade1, g_akao_seq_channel1->noise_mask, g_akao_seq_channel0->noise_mask, g_akao_sfx_control.noise_mask);
        func_80025F48((s32 *)off(D_8004F830, 8), g_akao_seq_channel1->pitch_mod_mask, g_akao_seq_channel0->pitch_mod_mask, g_akao_sfx_control.pitch_mod_mask);
        spu_set_reverb_enable(*fade1);
        spu_set_noise_enable(*fade0);
        spu_set_pitch_modulation_enable(*(s32 *)((u8 *)fade1 + 8));
        g_akao_driver_flags.unk8 &= ~0x100;
    }

    if (voice_mask != 0)
    {
        spu_set_key_on(voice_mask);
    }
}
