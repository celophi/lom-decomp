/*
 * g_akao_seq_master_state and g_akao_seq_channels are declared here as u8
 * objects and reached through casts, so this file does not include
 * akao_driver.h. The *View types mirror the akao_driver.h structures for the
 * fields this file writes.
 */

#include "akao.h"
#include "sdk/libspu.h"

#define AKAO_SFX_FIRST_VOICE 12
#define AKAO_FULL_VOLUME (AKAO_VOLUME_MAX << 8)
#define AKAO_INITIAL_SFX_TEMPO 0x66A80000
#define AKAO_INITIAL_REVERB_DEPTH 0x03FFF000
#define AKAO_REVERB_DEPTH_UPDATE_PENDING 0x80
#define AKAO_DEFAULT_REVERB_TYPE 4

#define SPU_CONTROL_ADDRESS 0x1F801DAA
#define SPU_INITIAL_CONTROL_MASK 0xFFFA
#define SPU_MASTER_VOLUME_LEFT (*(s16*)0x1F801D80)
#define SPU_MASTER_VOLUME_RIGHT (*(s16*)0x1F801D82)
#define SPU_CD_VOLUME_LEFT (*(s16*)0x1F801DB0)
#define SPU_CD_VOLUME_RIGHT (*(s16*)0x1F801DB2)
#define SPU_MAX_MASTER_VOLUME 0x3FFF
#define SPU_MAX_CD_VOLUME 0x7FFF

/** @brief SFX channel control block (mirrors SfxControl in akao_driver.h). */
typedef struct
{
    u32 active_mask;
    s32 key_on_mask;
    u32 note_on_mask;
    u32 key_off_mask;
    u32 paused_mask;
    u32 tempo;
    u32 tempo_acc;
    u32 noise_mask;
    u32 reverb_mask;
    u32 pitch_mod_mask;
} SfxControlView;

/** @brief AKAO driver state flags (mirrors AkaoDriverFlags in akao_driver.h). */
typedef struct
{
    u32 upload_flags;
    u32 output_mode;
    u32 update_flags;
} AkaoDriverFlagsView;

/** @brief Streamed-voice volume fields of g_akao_xa_tracker. */
typedef struct
{
    u8 _pad00[0x40];
    s32 volume;
    s32 volume_step;
    s32 volume_fade_ticks;
} AkaoXaTrackerView;

void akao_apply_reverb_type(s32 reverb_type);
extern u32 D_8003EC30[2];
extern s32 g_akao_bank_slot_keys[6];
extern AkaoDriverFlagsView g_akao_driver_flags;
extern SfxControlView g_akao_sfx_control;
extern u8 g_akao_seq_master_state;
extern AkaoSongState g_akao_suspended_song;
extern AkaoXaTrackerView g_akao_xa_tracker;
extern u32 g_akao_effect_voice_masks[3];
extern u8 g_akao_seq_channels;
extern u8 g_sfx_channels[];
extern AkaoChannelStatePtr g_akao_pending_channels;
extern AkaoSongStatePtr g_akao_seq_channel1;
extern s16 g_akao_mastervol_fade_ticks;
extern s16 g_akao_masterpan_fade_ticks;
extern s32 g_akao_seq_pending_ticks;
extern s16 g_akao_cdvol_fade_ticks;
extern s32 g_akao_cdvol_acc;
extern s32 g_akao_muted_channel_mask;
extern s32 g_akao_cdvol_tick;
extern s32 g_akao_mastervol_acc;
extern s32 g_akao_masterpan_acc;
extern s32 g_akao_driver_mode_flags;
extern void_ptr D_8003EC58;
extern AkaoSongStatePtr g_akao_seq_channel0;

/**
 * @brief Initializes song, sequence-channel, SFX and SPU mixer state.
 *
 * Resets 32 sequence slots to the unassigned SPU voice and assigns the 12 SFX
 * slots to voices 12 through 23. Seeds volume and timing state, configures
 * the SPU mixer, and installs the default reverb type.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/9R0Vj
 */
void akao_driver_init_state(void)
{
    u16* spu_control = (u16*)SPU_CONTROL_ADDRESS;
    u32 unassigned_voice = AKAO_VOICE_COUNT;
    AkaoSongState* song;
    u8* sequence_tick;
    AkaoChannelState* sfx_channel;
    u32 value;

    song = (AkaoSongState*)&g_akao_seq_master_state;
    song = (AkaoSongState*)((u32)song ^ 1);
    song = (AkaoSongState*)((u32)song ^ 1);

    sequence_tick = &g_akao_seq_channels;

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
    song->volume = (AKAO_VOLUME_MAX << 16);

    D_8003EC58 = sequence_tick;
    sequence_tick = (u8*)((u32)sequence_tick ^ 1);
    sequence_tick = (u8*)((u32)sequence_tick ^ 1);
    sequence_tick = (u8*)&((AkaoChannelState*)sequence_tick)->sfx_age;
    g_akao_seq_channel0 = song;
    g_akao_seq_channel1 = 0;
    g_akao_pending_channels = 0;
    g_akao_cdvol_tick = 0;
    song->volume_fade_ticks = 0;
    g_akao_cdvol_acc = (SPU_MAX_CD_VOLUME << 16);
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
    g_akao_xa_tracker.volume = AKAO_FULL_VOLUME;
    g_akao_xa_tracker.volume_fade_ticks = 0;
    g_akao_seq_pending_ticks = 0;
    g_akao_muted_channel_mask = 0;
    g_akao_driver_mode_flags = 0;
    g_akao_effect_voice_masks[2] = 0;
    g_akao_effect_voice_masks[1] = 0;
    g_akao_effect_voice_masks[0] = 0;
    *spu_control = (value & SPU_INITIAL_CONTROL_MASK) | 1;
    value = 0;
    do
    {
        value++;
        *((u32*)(sequence_tick - 0x24)) = 0;
        *((u32*)(sequence_tick + 0xA4)) = unassigned_voice;
        *((u16*)(sequence_tick + 0x0C)) = 0;
        *((u32*)sequence_tick) = 0;
        sequence_tick += 0x100;
        sequence_tick += 0x10;
        sequence_tick += 0x8;
    } while ((value & 0xFFFF) < AKAO_CHANNEL_COUNT);

    sfx_channel = (AkaoChannelState*)g_sfx_channels;
    for (value = AKAO_SFX_FIRST_VOICE; (u16)value < AKAO_VOICE_COUNT; value++, sfx_channel++)
    {
        sfx_channel->flags = 0;
        sfx_channel->voice = (u16)value;
        sfx_channel->is_sfx_channel = 1;
        /* The channel tick counter spans both halfwords at 0x58. */
        sfx_channel->sfx_age = 0;
        sfx_channel->volume_scale = AKAO_FULL_VOLUME;
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

    akao_apply_reverb_type(AKAO_DEFAULT_REVERB_TYPE);
    SpuSetReverb(SPU_ON);
}
