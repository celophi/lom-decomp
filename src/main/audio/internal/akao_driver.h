#ifndef _AKAO_DRIVER_H
#define _AKAO_DRIVER_H

#include "common.h"
#include "main/audio/akao.h"

/** @brief Root counter 2 target of one driver tick (akao_irq_handler). */
#define AKAO_TICK_PERIOD 0x44E8

/** @brief Number of SPU bank slots (entries of g_akao_bank_slot_keys). */
#define AKAO_BANK_SLOT_COUNT 6

/** @brief g_akao_driver_flags.upload_flags: an instrument bank is being streamed to the SPU. */
#define AKAO_UPLOAD_STREAMING 0x1

/** @brief Values of g_akao_driver_flags.output_mode. */
#define AKAO_OUTPUT_STEREO 1
#define AKAO_OUTPUT_MONO 2

/** @brief Centre position of g_akao_pan_gain_table; mono output uses its gain for both sides. */
#define AKAO_PAN_CENTER 0x80

/** @brief g_akao_driver_flags.update_flags: the SPU noise clock needs rewriting. */
#define AKAO_NOISE_CLOCK_UPDATE_PENDING 0x10
/** @brief g_akao_driver_flags.update_flags: the SPU reverb depth needs rewriting. */
#define AKAO_REVERB_DEPTH_UPDATE_PENDING 0x80
/** @brief g_akao_driver_flags.update_flags: the reverb/noise/pitch-mod voice masks need rewriting. */
#define AKAO_EFFECT_MASKS_UPDATE_PENDING 0x100

/** @brief g_akao_driver_mode_flags: the primary song is paused (its channels are parked). */
#define AKAO_MODE_SONG_PAUSED 0x1
/** @brief g_akao_driver_mode_flags: SFX channels are paused. */
#define AKAO_MODE_SFX_PAUSED 0x2
/** @brief g_akao_driver_mode_flags: tick the sequencers every frame regardless of tempo; no top-level code sets it. */
#define AKAO_MODE_FORCE_TICK 0x4

/** @brief Driver-wide state words. */
typedef struct
{
    u32 upload_flags; /**< AKAO_UPLOAD_* bits. */
    u32 output_mode;  /**< AKAO_OUTPUT_STEREO or AKAO_OUTPUT_MONO. */
    u32 update_flags; /**< Pending SPU hardware updates, applied by the tick. */
} AkaoDriverFlags;

/**
 * @brief Channel masks and tick rate of the SFX channel set.
 *
 * The masks use the same channel bits as the song masks; SFX channels occupy
 * bits 12-23 (AKAO_SFX_FIRST_CHANNEL_BIT upwards).
 */
typedef struct
{
    u32 active_mask;    /**< SFX channels currently playing. */
    s32 key_on_mask;    /**< Channels whose voice still needs a key-on. */
    u32 note_on_mask;   /**< Channels with a note sounding. */
    u32 key_off_mask;   /**< Channels whose voice still needs a key-off. */
    u32 paused_mask;    /**< Channels parked while SFX playback is paused. */
    u32 tempo;          /**< Q16 tick rate; the high half is added to tempo_acc each driver tick. */
    u32 tempo_acc;      /**< Tick accumulator; a carry out of the low half advances one tick. */
    u32 noise_mask;     /**< Voices enabled in the SPU noise bitmap. */
    u32 reverb_mask;    /**< Voices enabled in the SPU reverb bitmap. */
    u32 pitch_mod_mask; /**< Voices enabled in the SPU pitch-modulation bitmap. */
    u16 noise_freq;     /**< SPU noise clock (6 bits). */
} SfxControl;

/** @brief AkaoXaProgramHeader.flags: the program has separate left and right data. */
#define XA_FLAG_STEREO 0x1
/** @brief AkaoXaProgramHeader.flags: the program loops at loop_offset. */
#define XA_FLAG_LOOP 0x2
/** @brief AkaoXaTracker.flags: the program is fed from a CD ring of blocks. */
#define XA_FLAG_RING_STREAM 0x1000000

/**
 * @brief Header of an XA program (0x40 bytes); the ADPCM data follows it.
 *
 * The same header prefixes a program in a RAM buffer, the copy staged by
 * akao_upload_xa_program, and each block of a CD ring stream.
 */
typedef struct
{
    u32 magic;         /**< "AKAO" */
    u32 key;           /**< AKAO id and length, as one word. */
    u8 _pad08[8];
    u32 sample_size;   /**< Bytes of ADPCM data. */
    u32 loop_offset;   /**< Loop start inside the data, in bytes. */
    u32 flags;         /**< XA_FLAG_* bits. */
    u16 pitch;         /**< SPU pitch register value. */
    u8 _pad1E[2];
    u32 spu_addr;      /**< SPU address the program was uploaded to. */
    u32 right_offset;  /**< Start of the right channel data of a stereo program. */
    s32 fade_in_ticks; /**< Volume fade-in applied when a ring stream starts; 0 for none. */
    u8 _pad2C[0x14];
} AkaoXaProgramHeader;

/** @brief Staged XA program header and the first 16 bytes of its data. */
typedef struct
{
    AkaoXaProgramHeader header;
    u8 sample_prefix[0x10];
} AkaoXaProgramStaging;

/**
 * @brief Playback state of the streamed voice pair (g_akao_xa_tracker).
 *
 * A stream plays through two adjacent SPU voices. Its data is fed from main
 * RAM in blocks, either from one buffer or from a ring of blocks that the CD
 * reader fills (see akao_xa_advance_frame).
 */
typedef struct
{
    u8* data_cursor;       /**< Next block of RAM data to upload. */
    u8* loop_cursor;       /**< Data restart point of a looping program, or 0. */
    u32 flags;             /**< XA_FLAG_* bits. */
    s32 voice_mask;        /**< Voice mask of the stream's voice pair; 0 when idle. */
    s32 first_voice;       /**< First voice of the pair. */
    u32 bytes_remaining;   /**< Data still to upload. */
    s32 unk18;             /**< Copied from AkaoXaProgramHeader.spu_addr; not read. */
    u32 loop_bytes;        /**< bytes_remaining to reload when the program loops. */
    s32 last_ring_block_key; /**< Last uploaded ring block's AKAO id/length word; not read. */
    s32 filled_blocks;     /**< Ring blocks reported by the CD reader. */
    s32 uploaded_blocks;   /**< Ring blocks uploaded to the SPU. */
    union
    {
        s32 spu_addr;      /**< One-shot programs: SPU base address. */
        u8* ring_base;     /**< CD ring streams: first ring block. */
    } source;
    u32 ring_size;         /**< Ring size in bytes. */
    u32 upload_block;      /**< Index of the next ring block to upload. */
    u32 fill_block;        /**< Index of the ring block the CD reader fills next. */
    u32 ring_block_count;  /**< Blocks in the ring. */
    s32 volume;            /**< Q8 volume. */
    s32 volume_step;       /**< Volume fade step per tick. */
    s32 volume_fade_ticks; /**< Ticks left in the volume fade. */
    s32 pan;               /**< Q8 pan. */
    u8 _pad50[8];
    s32 pitch;             /**< SPU pitch of the voice pair. */
} AkaoXaTracker;

extern volatile s32 g_akao_spu_xfer_pending;
extern u8 g_akao_articulation_slots[];
/** @brief The SFX channels (AKAO_SFX_CHANNEL_COUNT entries). */
extern AkaoChannelState g_sfx_channels[];
extern s32 g_akao_driver_mode_flags;
extern s32 g_akao_muted_channel_mask;
extern s32 g_akao_seq_pending_ticks;
extern AkaoXaTracker g_akao_xa_tracker;
extern s16 g_akao_cdvol_fade_ticks;
extern s32 g_akao_masterpan_acc;
extern s16 g_akao_masterpan_fade_ticks;
extern s32 g_akao_mastervol_acc;
extern s16 g_akao_mastervol_fade_ticks;
extern s32 g_akao_cdvol_tick;
extern s32 g_akao_cdvol_acc;
/** @brief Channel table of the secondary song (g_akao_seq_channel1). */
extern AkaoChannelState* g_akao_pending_channels;
extern AkaoSongState* g_akao_seq_channel1;
extern AkaoSongState *g_akao_seq_channel0;
/** @brief Sequence-channel table base recorded during driver initialization. */
extern void* g_akao_seq_channels_base;
/** @brief Song state saved by akao_seq_suspend_song (the first 0x70 bytes are used). */
extern AkaoSongState g_akao_suspended_song;
/** @brief Channel table saved with g_akao_suspended_song. */
extern AkaoChannelState g_akao_suspended_channels[];
/** @brief Q15 pan gain for each of the 256 pan positions; the right gain is table[pan ^ 0xFF]. */
extern s16 g_akao_pan_gain_table[256];
extern SfxControl g_akao_sfx_control;
/** @brief Voice bitmaps for the SPU reverb [0], noise [1] and pitch-modulation [2] enables. */
extern u32 g_akao_effect_voice_masks[3];
extern AkaoDriverFlags g_akao_driver_flags;
/**
 * @brief Keys of the banks loaded into the 6 SPU bank slots.
 *
 * Each entry holds the loaded bank's id/length word (@c ((s32*)bank)[1], the
 * second word of its AkaoHeader) and is used by akao_upload_bank_slot to
 * detect and evict a previously-loaded copy. Zeroed for all 6 slots in
 * akao_driver_init_state.
 */
extern s32 g_akao_bank_slot_keys[6];
/** @brief Reset words: entry 0 is unknown; entry 1 aliases g_akao_song_descriptor_match_value[0]. */
extern u32 D_8003EC30[2];
/** @brief Channel table of the primary song (AKAO_CHANNEL_COUNT entries). */
extern AkaoChannelState g_akao_seq_channels[];
extern AkaoSongState g_akao_seq_master_state;
extern char g_akao_spu_malloc_table[];
extern char g_akao_spu_zero_primer[];
extern s32 g_akao_rcnt2_event;
extern u8* g_akao_bank_prog_base;
extern u8* g_akao_bank_region_b;
extern u8* g_akao_bank_region_c;

void akao_driver_init(void);
void akao_driver_shutdown(void);
void akao_driver_init_state(void);
void akao_set_bank_data_ptrs(u8* base);
void akao_relocate_articulations(AkaoArticulation* src, AkaoArticulation* dst, s32 spu_base, s32 count);
void akao_spu_wait(void);
extern s32 akao_check_magic(AkaoHeader* header);
extern s32 akao_submit_bank(AkaoBankHeader* bank, s32 wait_for_completion);
extern s32 akao_upload_bank(void* bank, s32 wait_for_completion, s32 bank_id, s32 spu_base);
void akao_spu_write(void* source, s32 byte_count);
void akao_spu_read(void* destination, s32 byte_count);
extern long akao_irq_handler(void);

#endif
