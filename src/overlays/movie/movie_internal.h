#ifndef LOM_MOVIE_INTERNAL_H
#define LOM_MOVIE_INTERNAL_H

#include "movie.h"
#include "cdrom.h"
#include "movie_state.h"
#include "pad.h"
#include "controller.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/libpress.h"
#include "sdk/libcd.h"

/* The block at 0x801ED600 is the merged-controller SCDRegs (see pad.h).
 * Skip-cinematic checks read the merged controller type, held buttons, and
 * newly pressed buttons from SCDRegs. */

/** @brief Integer and pointer views of a GPU transfer result. */
typedef union
{
    s32 sync_status;
    u_long address;
    u_long* ordering_table;
} MovieGpuResult;

/** @brief Unsigned storage and signed arithmetic views of a 16-bit value. */
typedef union
{
    u16 raw;
    s16 signed_value;
} MovieHalfword;

/** @brief MovieState::end_state sentinel values. */
#define END_STATE_RUNNING 0
#define END_STATE_NEAR_END 1
#define END_STATE_DONE 2

/** @brief MDEC output pipeline states. */
#define MDEC_STATE_IDLE 0
#define MDEC_STATE_ACTIVE 1
#define MDEC_STATE_CHAINED 2

/** @brief GPU transfer mode using DrawSync and LoadImage. */
#define MOVIE_GPU_MODE_STANDARD 0

/** @brief VLC decode and XA audio state used by @ref movie_update. */
#define STANDARD_VLC_DECODE_SIZE 0x1000
#define ALTERNATE_VLC_DECODE_SIZE 0x16AA
#define STANDARD_VLC_RETRY_COUNT 3
#define ALTERNATE_VLC_RETRY_COUNT 1
#define AUDIO_STREAM_STATE_IDLE 0
#define AUDIO_STREAM_STATE_SECTOR_READY 1
#define AUDIO_STREAM_STATE_PRIMED 2
#define AKAO_COMMAND_SELECTOR_9E 3
#define AKAO_XA_POSITION_UNAVAILABLE (-1)
#define AUDIO_SECTORS_PER_XA_FRAME 2
#define SIGNED_HALF_ROUNDING(value) ((u32)(value) >> 31)

/** @brief GPU and CD status values used by the MDEC output callback. */
#define CD_STATUS_RECOVERY_PENDING 1
#define DRAW_SYNC_MODE_POLL 1
#define DRAW_SYNC_DEFER_THRESHOLD 2
#define BREAK_DRAW_FAILURE_ADDRESS ((u_long) - 1)

/** @brief Fixed configuration used by @ref movie_play. */
#define MOVIE_DISPLAY_WIDTH 320
#define MOVIE_DISPLAY_HEIGHT 240
#define MOVIE_RGB24_VRAM_WIDTH ((MOVIE_DISPLAY_WIDTH * 3) / 2)
#define STANDARD_DECODE_RECT_WIDTH 24
#define ALTERNATE_DECODE_RECT_WIDTH 16
#define ALTERNATE_RECT_WRAP_THRESHOLD 0x300
#define ALTERNATE_RECT_WRAP_X 0x200
#define MOVIE_UPDATE_POLL_LIMIT 0x2000
#define CONTROLLER_VSYNC_INTERVAL_SINGLE 1
#define MOVIE_FRAME_VSYNC_INTERVAL 4
#define CD_ERROR_STATUS_RETRIES_EXHAUSTED 5

/** @brief Per-stream frame totals used as the playback stop condition. */
#define MOVIE_INTRO_TOTAL_FRAMES 2098
#define MOVIE_ATTRACT_1_TOTAL_FRAMES 2473
#define MOVIE_ATTRACT_2_PART_1_TOTAL_FRAMES 1318
#define MOVIE_ATTRACT_2_PART_2_TOTAL_FRAMES 5368
#define MOVIE_ATTRACT_2_PART_3_TOTAL_FRAMES 898

/**
 * @brief Cinematic indices selected by the main game-state dispatcher.
 *
 * GAME_STATE_ATTRACT_2 plays its three stream segments consecutively.
 */
typedef enum
{
    MOVIE_INDEX_INTRO = 0,
    MOVIE_INDEX_ATTRACT_1 = 1,
    MOVIE_INDEX_ATTRACT_2_PART_1 = 2,
    MOVIE_INDEX_ATTRACT_2_PART_2 = 3,
    MOVIE_INDEX_ATTRACT_2_PART_3 = 4
} MovieIndex;

/** @brief Skip-cinematic gating used by movie_play. */
#define MOVIE_FIRST_UNSKIPPABLE_INDEX MOVIE_INDEX_ATTRACT_2_PART_1
#define MOVIE_INTRO_SKIP_MASK ((u16) ~(PAD_BTN_SQUARE | PAD_BTN_CROSS | PAD_BTN_CIRCLE | PAD_BTN_TRIANGLE))
#define MOVIE_ATTRACT_1_SKIP_MASK (PAD_BTN_R2 | PAD_BTN_R1 | PAD_BTN_DOWN)
#define SCD_VALID_DEVICE_TYPE_COUNT 3 /**< digital, analog joystick, analog controller */

/** @brief Movie resource-table base, index representation, and initialization flag. */
#define MOVIE_RESOURCE_BASE 0x16A0
#define MOVIE_INIT_GPU_MODE_MASK 0x7F
#define MOVIE_INIT_USE_CD_AUDIO 0x80
#define MOVIE_INDEX_MASK 0xFFFF

/**
 * @brief Audio fade-out ramp during a skip-triggered exit.
 *
 * Armed by setting audio_fade_vol = AUDIO_FADE_INITIAL, stepped down by
 * AUDIO_FADE_STEP each outer-loop iteration, exits the loop when it reaches 0.
 */
#define AUDIO_FADE_DISARMED (-1)
#define AUDIO_FADE_INITIAL 0x70
#define AUDIO_FADE_STEP 0x10

/** @brief Movie initialization sentinels and audio parameters. */
#define MOVIE_FRAME_NONE ((u32) - 1)
#define AKAO_CD_VOLUME_MAX 0x7F
#define MOVIE_AKAO_C8_INIT_VALUE 0x7FFF
#define MOVIE_NONSTREAMED_CD_MIX_VOLUME 0xA0

/** @brief Encode a PSX RAM pointer in an AKAO command word. */
#define AKAO_STREAM_ADDRESS(buffer) ((u32)(buffer))

/** @brief sector_type values stamped in the CD-XA subheader. */
#define SECTOR_TYPE_VIDEO 0x8001
#define SECTOR_TYPE_AUDIO 1

/** @brief MovieState::continuation_type values; selects which ring a continuation sector goes into. */
#define CONTINUATION_VIDEO 0
#define CONTINUATION_AUDIO 1

/** @brief Ring capacities for the two movie buffer layouts. */
#define STANDARD_VIDEO_RING_SLOTS 50
#define ALTERNATE_VIDEO_RING_SLOTS 30
#define AUDIO_RING_SLOTS 16

/** @brief CD sector geometry consumed by @ref cd_sector_callback. */
#define CD_SECTOR_BYTES 2048
#define CD_HEADER_WORDS 8
#define CD_HEADER_BYTES (CD_HEADER_WORDS * sizeof(u32))
#define CD_PAYLOAD_BYTES (CD_SECTOR_BYTES - CD_HEADER_BYTES)
#define CD_PAYLOAD_WORDS (CD_PAYLOAD_BYTES / sizeof(u32))

/**
 * @brief Allocation descriptor consulted by @ref movie_init's path B.
 *
 * Only @c alloc_base (the buffer base address) is used here; the leading
 * 0x38 bytes are owned by other subsystems.
 */
typedef struct AlternateMovieDecodeBuffers AlternateMovieDecodeBuffers;

typedef struct
{
    u8 pad[0x38];
    AlternateMovieDecodeBuffers* alloc_base;
} AllocInfo;

/**
 * @brief Header layout shared by video- and audio-ring entries.
 *
 * 12 bytes total. The same layout is the leading prefix of every 32-byte
 * raw CD sector header read by @ref cd_sector_callback. Used for both
 * video (video_table_base, 32-byte stride) and audio (audio_data_base,
 * 2048-byte stride) ring entries.
 */
typedef struct
{
    u16 _unk0;            /**< 0x0 - always zero in the streams we read */
    u16 sector_type;      /**< 0x2 - 0x8001 = video, 1 = audio */
    u16 chunk_sector_idx; /**< 0x4 - sector position within a multi-sector frame */
    u16 sector_count;     /**< 0x6 - sectors comprising this frame chunk */
    u32 frame_number;     /**< 0x8 */
} SectorEntry;

/**
 * @brief One video-ring table entry: 32 bytes copied as 8 u32 words by
 *        @ref cd_sector_callback.
 *
 * The first 12 bytes are the SectorEntry header; the remaining 20 bytes hold
 * sector metadata. The actual VLC payload lives in a parallel buffer
 * (video_data_base, 2016-byte stride).
 */
typedef union VideoSectorEntry
{
    SectorEntry header;
    u32 words[CD_HEADER_WORDS];
} VideoSectorEntry;

/**
 * @brief One PSX CD sector (2048 bytes) of audio ring data.
 *
 * The 32-byte header is followed by a 2016-byte XA payload.
 */
typedef struct AudioSector
{
    VideoSectorEntry header_block;
    u8 payload[CD_PAYLOAD_BYTES];
} AudioSector;

/**
 * @brief One slot of the video VLC payload buffer: 2016 bytes of raw
 *        bitstream data.
 *
 * video_data_base is a parallel array of these, indexed by the same
 * read/write indices as video_table_base.
 */
typedef union VideoVlcPayload
{
    u8 data[CD_PAYLOAD_BYTES];
    u_long words[CD_PAYLOAD_WORDS];
} VideoVlcPayload;

/** @brief Typed views of the shared stream-entry pointer slot. */
typedef union
{
    u8* payload;
    VideoVlcPayload* video_payload;
    VideoSectorEntry* video_header;
    AudioSector* audio_sector;
} MovieStreamEntryPointer;

/** @brief View the storage after a video-header table as VLC payload slots. */
#define VIDEO_PAYLOADS_AFTER_TABLE(table, entry_count) ((VideoVlcPayload*)&(table)[entry_count])

/** @brief Fixed RAM layout used by the standard movie path. */
#define MOVIE_BUFFER_RAM_BASE 0x80147000
#define MOVIE_VLC_TABLE_BYTES 0x11000
#define STANDARD_VLC_INPUT_BYTES 0x14000
#define STANDARD_MDEC_OUTPUT_BYTES 0x2D00

typedef struct
{
    VideoSectorEntry video_table[STANDARD_VIDEO_RING_SLOTS];
    VideoVlcPayload video_data[STANDARD_VIDEO_RING_SLOTS];
    AudioSector audio_data[AUDIO_RING_SLOTS];
    u8 vlc_table[MOVIE_VLC_TABLE_BYTES];
    u8 vlc_input_buf[2][STANDARD_VLC_INPUT_BYTES];
    u_long mdec_output_buf[2][STANDARD_MDEC_OUTPUT_BYTES / sizeof(u_long)];
} StandardMovieBuffers;

#define STANDARD_MOVIE_BUFFERS ((StandardMovieBuffers*)MOVIE_BUFFER_RAM_BASE)

/** @brief Fixed RAM layout used by the alternate movie path. */
#define ALTERNATE_VLC_INPUT_BYTES 0x11000

typedef struct
{
    VideoSectorEntry video_table[ALTERNATE_VIDEO_RING_SLOTS];
    VideoVlcPayload video_data[ALTERNATE_VIDEO_RING_SLOTS];
    AudioSector audio_data[AUDIO_RING_SLOTS];
    u8 vlc_input_buf[2][ALTERNATE_VLC_INPUT_BYTES];
} AlternateMovieBuffers;

#define ALTERNATE_MOVIE_BUFFERS ((AlternateMovieBuffers*)MOVIE_BUFFER_RAM_BASE)

/** @brief Allocator-owned VLC and MDEC buffers used by the alternate path. */
#define ALTERNATE_MDEC_OUTPUT_BYTES 0x1E00

struct AlternateMovieDecodeBuffers
{
    u8 vlc_table[MOVIE_VLC_TABLE_BYTES];
    u_long mdec_output_buf[2][ALTERNATE_MDEC_OUTPUT_BYTES / sizeof(u_long)];
};

extern AllocInfo* g_allocInfo; /* allocation descriptor used by movie_init's alternate buffer layout */
extern u8 g_busy;              /* non-zero while a DMA/GPU operation is in flight (at 0x801ED596) */
extern u8 g_mdecRetryPending;  /* MDEC decode ready but MDEC was busy; retry on next tick (at 0x801ED595) */
extern u8 g_audioStreamState;  /* CD audio state: 0=idle, 1=sector arrived, 2=pipeline primed (at 0x801ED592) */
extern u16 g_sectorsRemaining; /* sectors left to read for the current multi-sector frame (at 0x801ED57E) */

u_int cdrom_process_state(void);
void cdrom_verify_recovery(void);
s32 cdrom_get_error_status(void);
void cdrom_reset(void);
void cdrom_wait_queue_empty(void);
/*
 * Intentionally left without parameter types: the movie callback uses the
 * legacy no-argument, boolean-sentinel ABI rather than CdCommandCallback's
 * buffer-return prototype.
 */
s32 cdrom_queue_command();

/* AKAO XA-streaming helpers (see config/symbols/shared_symbol_addrs.txt). */
void akao_cmd_c8(u32 arg0);                                /* AKAO cmd 0xC8 (raw param) */
void akao_xa_setup_panning(u32 sample_rate);               /* writes panning/sample-rate table */
void akao_cmd_e8_start_xa_stream(u32 addr, u32 len_bytes); /* AKAO cmd 0xE8 */
void akao_cmd_e4_set_cd_volume(s32 vol);                   /* AKAO cmd 0xE4 (vol & 0x7F << 8) */
void akao_xa_advance_frame(void);                          /* increments audio frame counters */
s32 akao_xa_get_position(void);                            /* returns SPU/XA position */
void akao_cmd_98_9a_9c_9e(u32 arg0);

/* These overlay-internal functions retain external linkage because the
 * original MOVIE.BIN exposes each one as a global symbol. */
void movie_init(s32 resource_index, s32 flags, s32 total_frames, s32 init_buffer_idx);
void movie_update(void);
void movie_mdec_out_callback(void);
void movie_schedule_next_decode(void);
s32 cd_sector_callback(void);
s32 get_next_audio_entry(AudioSector** out_entry);
void draw_sync_callback(void);
s32 get_next_video_entry(VideoVlcPayload** out_vlc_data, VideoSectorEntry** out_entry_header);
void advance_audio_read(void);
void advance_video_read(void);

#endif
