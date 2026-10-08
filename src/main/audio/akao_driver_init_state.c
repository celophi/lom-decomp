#define AKAO_DRIVER_INIT_STATE
#include "internal/akao_control.h"
#include "internal/akao_voice.h"
#include <libspu.h>

#define AKAO_INITIAL_SFX_TEMPO 0x66A80000
#define AKAO_INITIAL_REVERB_DEPTH 0x03FFF000

#define SPU_CONTROL_ADDRESS 0x1F801DAA
#define SPU_INITIAL_CONTROL_MASK 0xFFFA
#define SPU_CONTROL_CD_ENABLE 0x1
#define SPU_MASTER_VOLUME_LEFT (*(s16*)0x1F801D80)
#define SPU_MASTER_VOLUME_RIGHT (*(s16*)0x1F801D82)
#define SPU_MAX_MASTER_VOLUME 0x3FFF
#define SPU_MAX_CD_VOLUME 0x7FFF

/**
 * @brief Recover a channel from the address of its age counter.
 * @param age Address of the channel's sfx_age field.
 * @return Channel containing the age counter.
 */
#define AKAO_CHANNEL_FROM_AGE(age) ((AkaoChannelState*)((u8*)(age) - OFFSETOF(AkaoChannelState, sfx_age)))

/**
 * @brief Initializes song, sequence-channel, SFX and SPU mixer state.
 *
 * Resets 32 sequence slots to the unassigned SPU voice and assigns the 12 SFX
 * slots to voices 12 through 23. Seeds volume and timing state, configures
 * the SPU mixer for CD input, and selects the Studio C reverb preset.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/9R0Vj
 */
void akao_driver_init_state(void)
{
    u16* spu_control = (u16*)SPU_CONTROL_ADDRESS;
    u32 unassigned_voice = AKAO_VOICE_COUNT;
    AkaoSongState* song;
    u8* sequence_age;
    AkaoChannelState* sfx_channel;
    u32 value;

    song = (AkaoSongState*)&g_akao_seq_master_state;
    song = (AkaoSongState*)((uintptr_t)song ^ 1);
    song = (AkaoSongState*)((uintptr_t)song ^ 1);

    sequence_age = &g_akao_seq_channels;

    D_8003EC30[1] = 0;
    D_8003EC30[0] = 0;
    g_akao_bank_slot_keys[5] = 0;
    g_akao_bank_slot_keys[4] = 0;
    g_akao_bank_slot_keys[3] = 0;
    g_akao_bank_slot_keys[2] = 0;
    g_akao_bank_slot_keys[1] = 0;
    g_akao_bank_slot_keys[0] = 0;
    g_akao_driver_flags.upload_flags = 0;
    g_akao_driver_flags.output_mode = 1;

    g_akao_sfx_control.active_mask = 0;
    song->masks.active_mask = 0;
    song->masks.voice_alloc_low_mask = 0;
    song->song_id = 0;
    g_akao_sfx_control.paused_mask = 0;
    song->parked_mask = 0;
    g_akao_suspended_song.song_id = 0;
    g_akao_suspended_song.masks.active_mask = 0;
    song->volume = (AKAO_VOLUME_MAX << AKAO_Q16_SHIFT);

    g_akao_seq_channels_base = sequence_age;
    sequence_age = (u8*)((uintptr_t)sequence_age ^ 1);
    sequence_age = (u8*)((uintptr_t)sequence_age ^ 1);
    sequence_age = (u8*)&((AkaoChannelState*)sequence_age)->sfx_age;
    g_akao_seq_channel0 = song;
    g_akao_seq_channel1 = NULL;
    g_akao_pending_channels = NULL;
    g_akao_cdvol_tick = 0;
    song->volume_fade_ticks = 0;
    g_akao_cdvol_acc = (SPU_MAX_CD_VOLUME << AKAO_Q16_SHIFT);
    g_akao_mastervol_fade_ticks = 0;
    g_akao_mastervol_acc = 0;
    g_akao_masterpan_fade_ticks = 0;
    g_akao_masterpan_acc = 0;
    g_akao_cdvol_fade_ticks = 0;
    g_akao_sfx_control.noise_mask = 0;
    song->noise_mask = 0;
    g_akao_sfx_control.reverb_mask = 0;

    value = *spu_control;
    song->reverb_mask = 0;
    SPU_MASTER_VOLUME_LEFT = SPU_MAX_MASTER_VOLUME;
    SPU_MASTER_VOLUME_RIGHT = SPU_MAX_MASTER_VOLUME;
    SPU_CD_VOLUME_LEFT = SPU_MAX_CD_VOLUME;
    SPU_CD_VOLUME_RIGHT = SPU_MAX_CD_VOLUME;
    g_akao_sfx_control.pitch_mod_mask = 0;
    song->pitch_mod_mask = 0;
    song->ticks_per_beat = 0;
    song->beat = 0;
    song->beats_per_measure = 0;
    song->measure = 0;
    g_akao_xa_tracker.volume = AKAO_CHANNEL_VOLUME_MASK;
    g_akao_xa_tracker.volume_fade_ticks = 0;
    g_akao_seq_pending_ticks = 0;
    g_akao_muted_channel_mask = 0;
    g_akao_driver_mode_flags = 0;
    g_akao_effect_voice_masks[2] = 0;
    g_akao_effect_voice_masks[1] = 0;
    g_akao_effect_voice_masks[0] = 0;
    *spu_control = (value & SPU_INITIAL_CONTROL_MASK) | SPU_CONTROL_CD_ENABLE;
    value = 0;
    do
    {
        value++;
        AKAO_CHANNEL_FROM_AGE(sequence_age)->flags = 0;
        AKAO_CHANNEL_FROM_AGE(sequence_age)->voice = unassigned_voice;
        AKAO_CHANNEL_FROM_AGE(sequence_age)->is_sfx_channel = 0;
        AKAO_CHANNEL_FROM_AGE(sequence_age)->sfx_age = 0;
        sequence_age += 0x100;
        sequence_age += 0x10;
        sequence_age += 0x8;
    } while ((u16)value < AKAO_CHANNEL_COUNT);

    sfx_channel = (AkaoChannelState*)g_sfx_channels;
    for (value = AKAO_SFX_FIRST_VOICE; (u16)value < AKAO_VOICE_COUNT; value++, sfx_channel++)
    {
        sfx_channel->flags = 0;
        sfx_channel->voice = (u16)value;
        sfx_channel->is_sfx_channel = 1;
        sfx_channel->sfx_age = 0;
        sfx_channel->volume_scale = AKAO_CHANNEL_VOLUME_MASK;
        sfx_channel->volume_scale_fade_ticks = 0;
        sfx_channel->sfx_pitch_bend_fade_ticks = 0;
        sfx_channel->sfx_pitch_bend = 0;
        sfx_channel->note_expression_ticks = 0;
    }

    g_akao_seq_channel0->key_off_mask = 0;
    g_akao_seq_channel0->note_on_mask = 0;
    g_akao_seq_channel0->masks.key_on_mask = 0;
    g_akao_sfx_control.tempo_acc = 1;
    g_akao_sfx_control.tempo = AKAO_INITIAL_SFX_TEMPO;
    g_akao_sfx_control.key_off_mask = 0;
    g_akao_sfx_control.note_on_mask = 0;
    g_akao_sfx_control.key_on_mask = 0;
    g_akao_seq_channel0->reverb_depth = AKAO_INITIAL_REVERB_DEPTH;
    g_akao_seq_channel0->reverb_depth_step = 0;
    g_akao_seq_channel0->reverb_depth_fade_ticks = 0;
    g_akao_driver_flags.update_flags |= AKAO_REVERB_DEPTH_UPDATE_PENDING;

    akao_apply_reverb_type(SPU_REV_MODE_STUDIO_C);
    SpuSetReverb(SPU_ON);
}
