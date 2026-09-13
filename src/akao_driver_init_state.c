/* The two entry symbols use scalar address declarations here. Their aggregate
 * declarations in akao_driver.h require this translation unit to stay separate. */

#include "akao.h"
#include "sdk/libspu.h"

#define AKAO_SEQUENCE_CHANNEL_COUNT 32
#define AKAO_SFX_FIRST_VOICE 12
#define AKAO_SPU_VOICE_COUNT 24
#define AKAO_FULL_VOLUME (AKAO_VOLUME_MAX << 8)
#define AKAO_INITIAL_SFX_TEMPO 0x66A80000
#define AKAO_INITIAL_MASTER_VOLUME 0x03FFF000
#define AKAO_REVERB_UPDATE_PENDING 0x80
#define AKAO_DEFAULT_REVERB_TYPE 4

#define SPU_CONTROL_ADDRESS 0x1F801DAA
#define SPU_INITIAL_CONTROL_MASK 0xFFFA
#define SPU_MASTER_VOLUME_LEFT (*(s16*)0x1F801D80)
#define SPU_MASTER_VOLUME_RIGHT (*(s16*)0x1F801D82)
#define SPU_CD_VOLUME_LEFT (*(s16*)0x1F801DB0)
#define SPU_CD_VOLUME_RIGHT (*(s16*)0x1F801DB2)
#define SPU_MAX_MASTER_VOLUME 0x3FFF
#define SPU_MAX_CD_VOLUME 0x7FFF

void akao_apply_reverb_type(s32 reverb_type);
extern u32 D_8003EC30[2];
extern s32 g_akao_bank_slot_keys[6];
extern u8 g_akao_driver_flags[];
extern u8 g_akao_sfx_control[];
extern u8 g_akao_seq_master_state;
extern u8 D_8004C2D0[];
extern u8 g_akao_xa_tracker[];
extern u32 D_8004F830[3];
extern u8 g_akao_seq_channels;
extern u8 g_sfx_channels[];
extern s32 g_akao_pending_channels;
extern AkaoChannelState* g_akao_seq_channel1;
extern s16 g_akao_mastervol_fade_ticks;
extern s16 g_akao_masterpan_fade_ticks;
extern s32 g_akao_seq_pending_ticks;
extern s16 g_akao_cdvol_fade_ticks;
extern s32 g_akao_cdvol_acc;
extern s32 D_8003EC6C;
extern s32 g_akao_cdvol_tick;
extern s32 g_akao_mastervol_acc;
extern s32 g_akao_masterpan_acc;
extern s32 g_akao_driver_mode_flags;
extern void* D_8003EC58;
extern AkaoChannelState* g_akao_seq_channel0;

/**
 * @brief Returns a byte address within a driver state block.
 * @param base Start of the state block.
 * @param offset Byte offset within the block.
 * @return Address of the selected field.
 */
static inline u8* akao_state_offset(u8* base, s32 offset)
{
    return base + offset;
}

/**
 * @brief Initializes song, sequence-channel, SFX and SPU mixer state.
 *
 * Resets 32 sequence slots to the unassigned SPU voice and assigns the 12 SFX
 * slots to voices 12 through 23. Seeds volume and timing state, configures
 * the SPU mixer, and installs the default reverb type.
 *
 * @see https://decomp.me/scratch/9R0Vj (100%)
 */
void akao_driver_init_state(void)
{
    u16* spu_control = (u16*)SPU_CONTROL_ADDRESS;
    u32 unassigned_voice = AKAO_SPU_VOICE_COUNT;
    AkaoChannelState* song;
    u8* sequence_tick;
    u32 master_volume;
    u32 value;

    song = (AkaoChannelState*)&g_akao_seq_master_state;
    song = (AkaoChannelState*)((u32)song ^ 1);
    song = (AkaoChannelState*)((u32)song ^ 1);

    sequence_tick = &g_akao_seq_channels;

    D_8003EC30[1] = 0;
    D_8003EC30[0] = 0;
    g_akao_bank_slot_keys[5] = 0;
    g_akao_bank_slot_keys[4] = 0;
    g_akao_bank_slot_keys[3] = 0;
    g_akao_bank_slot_keys[2] = 0;
    g_akao_bank_slot_keys[1] = 0;
    g_akao_bank_slot_keys[0] = 0;
    *((u32*)akao_state_offset(g_akao_driver_flags, 0x00)) = 0;
    *((u32*)akao_state_offset(g_akao_driver_flags, 0x04)) = 1;

    *((u32*)akao_state_offset(g_akao_sfx_control, 0x00)) = 0;
    song->w04.song.active_mask = 0;
    song->w04.song.voice_alloc_low_mask = 0;
    song->unk5E = 0;
    *((u32*)akao_state_offset(g_akao_sfx_control, 0x10)) = 0;
    song->unk1C = 0;
    ((AkaoChannelState*)D_8004C2D0)->unk5E = 0;
    ((AkaoChannelState*)D_8004C2D0)->w04.song.active_mask = 0;
    song->pitch_slide_step = (AKAO_VOLUME_MAX << 16);

    D_8003EC58 = sequence_tick;
    sequence_tick = (u8*)((u32)sequence_tick ^ 1);
    sequence_tick = (u8*)((u32)sequence_tick ^ 1);
    sequence_tick = (u8*)&((AkaoChannelState*)sequence_tick)->unk58;
    g_akao_seq_channel0 = song;
    g_akao_seq_channel1 = 0;
    g_akao_pending_channels = 0;
    g_akao_cdvol_tick = 0;
    song->unk58 = 0;
    g_akao_cdvol_acc = (SPU_MAX_CD_VOLUME << 16);
    g_akao_mastervol_fade_ticks = 0;
    g_akao_mastervol_acc = 0;
    g_akao_masterpan_fade_ticks = 0;
    g_akao_masterpan_acc = 0;
    g_akao_cdvol_fade_ticks = 0;
    *((u32*)akao_state_offset(g_akao_sfx_control, 0x1C)) = 0;
    song->reverb_mask = 0;
    *((u32*)akao_state_offset(g_akao_sfx_control, 0x20)) = 0;

    value = *spu_control;
    song->noise_mask = 0;
    SPU_MASTER_VOLUME_LEFT = SPU_MAX_MASTER_VOLUME;
    SPU_MASTER_VOLUME_RIGHT = SPU_MAX_MASTER_VOLUME;
    SPU_CD_VOLUME_LEFT = SPU_MAX_CD_VOLUME;
    SPU_CD_VOLUME_RIGHT = SPU_MAX_CD_VOLUME;
    *((u32*)akao_state_offset(g_akao_sfx_control, 0x24)) = 0;
    song->pitch_mod_mask = 0;
    song->unk68 = 0;
    song->unk66 = 0;
    song->is_sfx_channel = 0;
    song->measure = 0;
    *((u32*)akao_state_offset(g_akao_xa_tracker, 0x40)) = AKAO_FULL_VOLUME;
    *((u32*)akao_state_offset(g_akao_xa_tracker, 0x48)) = 0;
    g_akao_seq_pending_ticks = 0;
    D_8003EC6C = 0;
    g_akao_driver_mode_flags = 0;
    D_8004F830[2] = 0;
    D_8004F830[1] = 0;
    D_8004F830[0] = 0;
    *spu_control = (value & SPU_INITIAL_CONTROL_MASK) | 1;
    value = 0;
    do
    {
        value++;
        *((volatile u32*)(sequence_tick - 0x24)) = 0;
        *((volatile u32*)(sequence_tick + 0xA4)) = unassigned_voice;
        *((volatile u16*)(sequence_tick + 0x0C)) = 0;
        *((volatile u32*)sequence_tick) = 0;
        sequence_tick += 0x100;
        sequence_tick += 0x10;
        sequence_tick += 0x8;
    } while ((value & 0xFFFF) < AKAO_SEQUENCE_CHANNEL_COUNT);

    {
        AkaoChannelState* sfx_channel = (AkaoChannelState*)g_sfx_channels;
        for (value = AKAO_SFX_FIRST_VOICE; (u16)value < AKAO_SPU_VOICE_COUNT; value++, sfx_channel++)
        {
            sfx_channel->flags = 0;
            sfx_channel->voice = (u16)value;
            sfx_channel->is_sfx_channel = 1;
            /* The channel tick counter spans both halfwords at 0x58. */
            *(u32*)&sfx_channel->unk58 = 0;
            sfx_channel->volume_scale = AKAO_FULL_VOLUME;
            sfx_channel->unk8E = 0;
            sfx_channel->unk88 = 0;
            sfx_channel->noise_mask = 0;
            sfx_channel->note_expression_ticks = 0;
        }
    }

    {
        u8* active_song = (u8*)g_akao_seq_channel0;
        u8* sfx_control = g_akao_sfx_control;
        u8* driver_flags = g_akao_driver_flags;
        *((u32*)akao_state_offset(active_song, 0x18)) = 0;
        *((u32*)akao_state_offset(active_song, 0x14)) = 0;
        *((u32*)akao_state_offset(active_song, 0x10)) = 0;
        *((u32*)akao_state_offset(sfx_control, 0x18)) = 1;
        *((u32*)akao_state_offset(sfx_control, 0x14)) = AKAO_INITIAL_SFX_TEMPO;
        *((u32*)akao_state_offset(sfx_control, 0x0C)) = 0;
        *((u32*)akao_state_offset(sfx_control, 0x08)) = 0;
        *((u32*)akao_state_offset(sfx_control, 0x04)) = 0;
        master_volume = AKAO_INITIAL_MASTER_VOLUME;
        *((u32*)akao_state_offset(active_song, 0x48)) = master_volume;
        *((u32*)akao_state_offset(active_song, 0x4C)) = 0;
        *((u16*)akao_state_offset(active_song, 0x5A)) = 0;
        *((u32*)akao_state_offset(driver_flags, 0x08)) = (*((u32*)akao_state_offset(driver_flags, 0x08))) | AKAO_REVERB_UPDATE_PENDING;
    }

    akao_apply_reverb_type(AKAO_DEFAULT_REVERB_TYPE);
    SpuSetReverb(1);
}
