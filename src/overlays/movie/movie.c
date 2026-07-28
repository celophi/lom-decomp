#include "movie.h"
#include "pad.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libpress.h"
#include "psyq/libcd.h"

/* The block at 0x801ED600 is the merged-controller SCDRegs (see pad.h).
 * Skip-cinematic checks read the merged controller type, held buttons, and
 * newly pressed buttons from SCDRegs. */

/** @brief Saved callback represented as either an SDK return value or handler. */
typedef union
{
    u32 address;
    void (*handler)(void);
} MovieCallback;

/**
 * @brief Movie playback control block; lives at fixed RAM address 0x801ED500.
 *
 * Aliases the AudioSystem block defined in cdrom.c. The CD subsystem uses that
 * block to save the DecDCT and DrawSync callbacks that were active before XA
 * audio playback began so `cdrom_reset` can restore them. Movie playback
 * temporarily re-uses the same memory as scratch, which is why the
 * `dec_dct_out_callback` / `draw_sync_callback` fields (offsets 0x38 / 0x3C)
 * here hold the *previous* handlers - `movie_init` captures them via
 * `DecDCToutCallback(&movie_mdec_out_callback)` etc.
 */
typedef struct
{
    // ---- stream buffers ----
    struct VideoSectorEntry* video_table_base; // array of 32-byte video sector headers
    struct VideoVlcPayload* video_data_base;   // parallel array of 2016-byte VLC payloads
    struct AudioSector* audio_data_base;       // array of 2048-byte CD sectors
    void* vlc_table;                           // opaque VLC decode table
    void* vlc_input_buf[2];
    u_long* mdec_output_buf[2];

    // ---- VRAM destination rectangles ----
    // rects[0] : frame A, rects[1] : frame B, rects[2] : decode rect
    RECT rects[3]; // offsets 32..55
    // The third rectangle's width/height also serve as frame dimensions:
    // s16 frame_width  = rects[2].w;   (offset 52)
    // s16 frame_height = rects[2].h;   (offset 54)

    // ---- callback handles ----
    MovieCallback dec_dct_out_callback;
    MovieCallback draw_sync_callback;

    u8 pad_40[4]; // unreferenced bytes retained for the fixed layout

    // ---- stream metadata ----
    u32 resource_index;      // CD resource index
    u32 current_frame;       // current frame counter (starts at 0)
    u32 total_frames;        // total frames in movie stream
    s32 video_ring_capacity; // 0x50 - video ring buffer capacity
    s32 audio_ring_capacity; // 0x54 - audio ring buffer capacity

    // ---- ring buffer indices ----
    s32 video_write_idx;      // 0x58 - next slot to write into video ring
    s32 video_read_idx;       // 0x5C - next slot to read from video ring
    s32 video_ring_size;      // 0x60 - video ring wrap point (set to old write_idx on ring wrap)
    s32 audio_write_idx;      // 0x64
    s32 audio_read_idx;       // 0x68
    s32 audio_ring_size;      // 0x6C - audio ring wrap point
    u32 audio_buffered_count; // 0x70 - cumulative sector count queued but not yet consumed
    u32 frame_number;         // 0x74 - frame number of sector currently being read
    u32 continuation_type;    // 0x78 - 0=video continuation, non-zero=audio continuation

    // ---- chunk-sector tracking (offsets 0x7C..0x7F) ----
    u16 chunk_sector_idx;          // 0x7C - sector index within current multi-sector frame
    u16 sectors_remaining;         // 0x7E - sectors left to read for the current frame chunk
    u32 last_video_frame;          // 0x80 - frame number of last video sector written
    u32 last_consumed_video_frame; // 0x84

    // ---- audio frame tracking (both structs) ----
    u32 last_audio_frame;          // offset 136..139
    u32 last_consumed_audio_frame; // offset 140..143

    // ---- status bytes (offsets 0x90..0x9F) ----
    // These four s8 declarations are match-sensitive: changing them to u8
    // changes GCC 2.8.0's integer-promotion shape in movie_update.
    u8 gpu_mode;            // 0 = DrawSync/LoadImage, non-zero = BreakDraw/LoadImage2 path
    u8 use_cd_audio;        // bit 7 of movie_init flags; selects XA streaming when set
    u8 audio_stream_state;  // shared alias of g_audioStreamState
    u8 input_buf_idx;       // which vlc_input_buf[] holds current VLC-decoded input (toggled each frame)
    s8 vlc_retry_count;     // countdown for DecDCTvlc2 retries
    s8 mdec_retry_pending;  // MDEC was busy; retry on the next tick
    u8 busy;                // non-zero while DMA/GPU operation is in flight
    u8 draw_sync_target;    // 0x97 - DrawSync target value (set by mdec_out_callback / service_video_ops)
    u8 chunk_idx;           // initial active chunk index (0 or 1)
    u8 out_buf_idx;         // which mdec_output_buf[] receives the next DecDCTout output (0 or 1)
    u8 pending_vram_upload; // 0x9A - decoded frame is ready, needs LoadImage to VRAM
    u8 pending_mdec_decode; // 0x9B - bitstream staged, needs DecDCTout kicked
    s8 mdec_busy;           // non-zero while MDEC/DMA is in flight
    s8 frame_ready;         // a complete frame can be displayed
    u8 end_of_stream; // 0x9E - set when frame_number >= total_frames
    u8 end_state;     // 1 = near end, 2 = stream fully ended (END_STATE_*)
} MovieState;

/** @brief MovieState::end_state sentinel values. */
#define END_STATE_RUNNING 0
#define END_STATE_NEAR_END 1
#define END_STATE_DONE 2

/** @brief Skip-cinematic gating used by movie_play. */
#define MOVIE_SKIPPABLE_MAX 2   /**< only movies with idx < this are skippable */
#define MOVIE0_SKIP_MASK 0xFF0F /**< movie 0 (intro/logo): broad - any non-bit-4..7 button */
#define MOVIE1_SKIP_MASK 0x400A /**< movie 1: narrow specific combination */
#define SCD_DEVICE_STATE_OK 3   /**< device_type < this means the controller is usable */

/** @brief Movie resource-table base and initialization flag. */
#define MOVIE_RESOURCE_BASE 0x16A0
#define MOVIE_INIT_USE_CD_AUDIO 0x80

/**
 * @brief Audio fade-out ramp during a skip-triggered exit.
 *
 * Armed by setting audio_fade_vol = AUDIO_FADE_INITIAL, stepped down by
 * AUDIO_FADE_STEP each outer-loop iteration, exits the loop when it reaches 0.
 */
#define AUDIO_FADE_DISARMED (-1)
#define AUDIO_FADE_INITIAL 0x70
#define AUDIO_FADE_STEP 0x10

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
#define CD_HEADER_WORDS         8       /**< 32-byte sector header read with CdGetSector. */
#define CD_PAYLOAD_WORDS        0x1F8   /**< 504 u32 = 2016-byte sector payload. */

/**
 * @brief Accessors for the fixed-address MovieState block at 0x801ED500.
 *
 * Use the @ref VOL_MOVIE_STATE form when a volatile access is required;
 * wrapping that cast in MOVIE_STATE would silently drop the volatile
 * qualifier.
 */
#define MOVIE_STATE ((MovieState*)0x801ED500)
#define VOL_MOVIE_STATE ((volatile MovieState*)0x801ED500)

/**
 * @brief Allocation descriptor consulted by @ref movie_init's path B.
 *
 * Only @c alloc_base (the buffer base address) is used here; the leading
 * 0x38 bytes are owned by other subsystems.
 */
typedef struct
{
    u8 pad[0x38];
    u8* alloc_base;
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
    s32 frame_number;     /**< 0x8 */
} SectorEntry;

/**
 * @brief One PSX CD sector (2048 bytes) of audio ring data.
 *
 * Layout:
 *   - bytes 0x00..0x0B: SectorEntry header (sector_count, frame_number).
 *   - bytes 0x0C..0x1F: remaining 20 bytes of the CD-XA sector header
 *     (copied verbatim from the CD by cd_sector_callback).
 *   - bytes 0x20..0x7FF: XA audio payload (2016 bytes).
 */
typedef struct AudioSector
{
    SectorEntry header;           /**< 12 bytes */
    u8 _hdr_remainder[0x20 - 12]; /**< 20 bytes - rest of the 32-byte CD header */
    u8 payload[2048 - 0x20];      /**< 2016 bytes XA */
} AudioSector;

/**
 * @brief One video-ring table entry: 32 bytes copied as 8 u32 words by
 *        @ref cd_sector_callback.
 *
 * The first 12 bytes are the SectorEntry header; the remaining 20 bytes hold
 * sector metadata. The actual VLC payload lives in a parallel buffer
 * (video_data_base, 2016-byte stride).
 */
typedef struct VideoSectorEntry
{
    SectorEntry header;
    u8 _rest[32 - 12];
} VideoSectorEntry;

/**
 * @brief One slot of the video VLC payload buffer: 2016 bytes of raw
 *        bitstream data.
 *
 * video_data_base is a parallel array of these, indexed by the same
 * read/write indices as video_table_base.
 */
typedef struct VideoVlcPayload
{
    u8 data[2016];
} VideoVlcPayload;

/* g_cdAudioReady / g_cdStatusByte3 are also declared in cd.h; the MOVIE.BIN
 * overlay references them directly so we redeclare here to keep movie.c
 * self-contained without pulling in the full CD header. */
extern u_char g_cdAudioReady;
extern s8 g_cdStatusByte3;

extern AllocInfo* g_allocInfo; /* allocation descriptor used by movie_init's alternate buffer layout */
extern u8 g_gpuMode;           /* 0=DrawSync/LoadImage path; non-zero=BreakDraw/LoadImage2 path (at 0x801ED590) */
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
void update_controllers(void);
void set_controller_vsync_interval(u32 interval);
void reset_controller_vsync_state(void);

/* AKAO XA-streaming helpers (see config/symbols/shared_symbol_addrs.txt). */
void akao_cmd_c8(u32 arg0);                                /* AKAO cmd 0xC8 (raw param) */
void akao_xa_setup_panning(u32 sample_rate);               /* writes panning/sample-rate table */
void akao_cmd_e8_start_xa_stream(u32 addr, u32 len_bytes); /* AKAO cmd 0xE8 */
void akao_cmd_e4_set_cd_volume(s32 vol);                   /* AKAO cmd 0xE4 (vol & 0x7F << 8) */
void akao_xa_advance_frame(void);                          /* increments audio frame counters */
s32 akao_xa_get_position(void);                            /* returns SPU/XA position */
void akao_cmd_98_9a_9c_9e(u32 arg0);

const s32 g_movieOverlayId = 14;

/* These overlay-internal functions retain external linkage because the
 * original MOVIE.BIN exposes each one as a global symbol. */
void movie_init(s32 resource_index, s32 flags, s32 total_frames, s32 init_buffer_idx);
void movie_update(void);
void movie_mdec_out_callback(void);
void movie_schedule_next_decode(void);
void movie_service_video_ops(void);
s32 cd_sector_callback(void);
s32 get_next_audio_entry(AudioSector** out_entry);
void draw_sync_callback(void);
s32 get_next_video_entry(VideoVlcPayload** out_vlc_data, VideoSectorEntry** out_entry_header);
void advance_audio_read(void);
void advance_video_read(void);

/**
 * @brief Play one of five MDEC cinematics, selected by index.
 *
 * Drives the full playback loop: configures the dual-buffered DISPENV pair,
 * resolves the per-movie frame count, calls @ref movie_init to stage the
 * stream, then services @ref movie_update / @ref movie_service_video_ops
 * until the stream end-state is reached. Includes a CD error-recovery
 * inner loop and an optional audio fade-out.
 *
 * @param movie_index Cinematic to play (0..4). Indices outside this range
 *                   fall through to the case-4 default (898 frames).
 *
 * @see https://decomp.me/scratch/gkEWm (100%)
 */
void movie_play(s32 movie_index)
{
    DISPENV env[2];
    DISPENV* p_disp_env;
    volatile MovieState* state;
    s32 audio_fade_vol;
    s32 retry_limit;
    s8 end_state_match;
    s32 error_status;
    s32 timeout;
    u16 movie_idx_u16;
    u16 buttons;
    s32 frame_count;
    u32 idx;
    s32 resource_idx;
    s32 init_flags;

    VSync(0);
    update_controllers();
    set_controller_vsync_interval(1);
    VSync(0);
    update_controllers();
    cdrom_process_state();
    /* Pre-playback skip gate: if user is already holding a skip button on
     * movie 0 when we get here, bail before staging the stream. */
    if ((((movie_index & 0xFFFF) == 0) && ((SCD_REGS)->device_type < SCD_DEVICE_STATE_OK)) &&
        (((SCD_REGS)->held_buttons & MOVIE0_SKIP_MASK) != 0))
    {
        return;
    }

    reset_controller_vsync_state();
    DecDCTReset(0);
    timeout = 0xF0;
    SetDefDispEnv(&env[0], 0, 0, 320, timeout);
    SetDefDispEnv(&env[1], 0, timeout, 320, timeout);
    (env[1].isrgb24 = 1);
    env[0].isrgb24 = 1;

    /*
     * Five MDEC cinematics; movie_index selects one (0..4). The frame count
     * matches each movie's BS stream length and is used by movie_init as the
     * stop condition. The CD resource index is the per-movie BS file at base
     * 0x16A0 (so resources 0x16A0..0x16A4).
     *
     *   index | resource | frames
     *   ------+----------+--------
     *     0   | 0x16A0   |  2098
     *     1   | 0x16A1   |  2473
     *     2   | 0x16A2   |  1318
     *     3   | 0x16A3   |  5368
     *     4   | 0x16A4   |   898   (shortest - also the default fallthrough)
     */

    switch ((u16)(movie_index & 0xFFFF))
    {
    case 0:
        frame_count = 2098;
        init_flags = MOVIE_INIT_USE_CD_AUDIO;
        break;

    case 1:
        frame_count = 2473;
        init_flags = MOVIE_INIT_USE_CD_AUDIO;
        break;

    case 2:
        frame_count = 1318;
        init_flags = MOVIE_INIT_USE_CD_AUDIO;
        break;

    case 3:
        frame_count = 5368;
        init_flags = MOVIE_INIT_USE_CD_AUDIO;
        break;

    case 4:

    default:
        frame_count = 898;
        init_flags = MOVIE_INIT_USE_CD_AUDIO;
        break;
    }

    resource_idx = (movie_index & 0xFFFF) + MOVIE_RESOURCE_BASE;
    movie_init(resource_idx, init_flags, frame_count, 0);
    VSync(0);
    update_controllers();
    audio_fade_vol = AUDIO_FADE_DISARMED;
    retry_limit = 5;
    state = VOL_MOVIE_STATE;
    end_state_match = END_STATE_DONE;

    while (TRUE)
    {
        error_status = cdrom_get_error_status();

        while ((error_status != 0) && (error_status != retry_limit))
        {
            set_controller_vsync_interval(1);
            VSync(0);
            update_controllers();
            cdrom_process_state();
            error_status = cdrom_get_error_status();
        }

        while (state->frame_ready == 0)
        {
            timeout = 0x2000;

            while (TRUE)
            {
                movie_update();

                if (state->frame_ready != 0)
                {
                    break;
                }

                if (state->end_state == end_state_match)
                {
                    reset_controller_vsync_state();
                    cdrom_reset();
                    DrawSync(0);
                    VSync(0);
                    SetDispMask(0);
                    return;
                }

                movie_service_video_ops();

                if (--timeout == 0)
                {
                    break;
                }
            };

            if (timeout == 0)
            {
                cdrom_process_state();
            }
        }

        state->frame_ready = 0;
        set_controller_vsync_interval(4);
        movie_idx_u16 = (u16)(movie_index & 0xFFFF);
        VSync(0);
        p_disp_env = &env[0];
        if (state->chunk_idx == 0)
        {
            p_disp_env = &env[1];
        }
        PutDispEnv(p_disp_env);
        SetDispMask(1);
        update_controllers();
        cdrom_process_state();

        idx = movie_idx_u16;
        if ((idx < MOVIE_SKIPPABLE_MAX) && ((SCD_REGS)->device_type < SCD_DEVICE_STATE_OK))
        {
            buttons = (SCD_REGS)->pressed_buttons;
            if (((idx != 0) ? ((buttons & MOVIE1_SKIP_MASK) != 0) : ((buttons & MOVIE0_SKIP_MASK) != 0)) != 0)
            {
                if (g_cdAudioReady == 0)
                {
                    break;
                }

                if (audio_fade_vol == AUDIO_FADE_DISARMED)
                {
                    audio_fade_vol = AUDIO_FADE_INITIAL;
                }
            }
        }

        if ((g_cdAudioReady != 0) && (audio_fade_vol != AUDIO_FADE_DISARMED))
        {
            akao_cmd_e4_set_cd_volume(audio_fade_vol);

            if (audio_fade_vol == 0)
            {
                break;
            }

            audio_fade_vol -= AUDIO_FADE_STEP;
        }

        if (state->end_state == end_state_match)
        {
            break;
        }
    }

    reset_controller_vsync_state();
    cdrom_reset();
    DrawSync(0);
    VSync(0);
    SetDispMask(0);
}

/**
 * @brief Stage a movie stream for playback into the global @ref MovieState.
 *
 * Allocates VRAM rectangles, sets up the VLC table / MDEC output buffers /
 * audio data buffer, registers the DecDCT-out and DrawSync callbacks, then
 * issues the initial CD read. Two layout paths exist depending on
 * @ref g_gpuMode (standard vs BreakDraw/dynamic-alloc_base).
 *
 * @param resource_index CD resource id of the BS stream (e.g. 0x16A0..0x16A4).
 * @param flags          Bits 0..6 select the GPU transfer path; bit 7 selects
 *                       CD/XA audio.
 * @param total_frames   Frame-count stop condition; set into MovieState.total_frames.
 * @param init_buffer_idx Initial active chunk index (0 or 1). Path B uses it to
 *                      seed rects[2] from rects[init_buffer_idx].
 *
 * @note `MovieState.gpu_mode` and the `g_gpuMode` global are *the same byte* at
 *       0x801ED590 (offset 0x90 in MovieState). The write at the top of this
 *       function and the `if (g_gpuMode == 0)` branch read the same storage.
 *
 * @note In shipped play, `movie_play` always passes flags = 0x80, so
 *       gpu_mode = 0 (path A taken every time) and use_cd_audio = 1.
 *       Path B is dead in production; preserved for matching, which is also why
 *       its read of an uninitialised rects[0].x (`>= 0x300` guard) is inert.
 *
 * @note Path-A memory map (RAM addresses are literals in this function):
 *           0x80147000  video_table_base   (50 x 32   = 0x640)
 *           0x80147640  video_data_base    (50 x 2016)
 *           0x80160000  audio_data_base    (16 x 2048 = 0x8000)
 *           0x80168000  vlc_table          (0x11000)
 *           0x80179000  vlc_input_buf[0]   (0x14000)
 *           0x8018D000  vlc_input_buf[1]   (0x14000)
 *           0x801A1000  mdec_output_buf[0] (0x2D00 = 24 x 240 x 2)
 *           0x801A3D00  mdec_output_buf[1] (0x2D00)
 *       Sizes match rects[2] = 24-wide x 240-tall macroblock decode column.
 *
 * @note Path-B memory map (fixed buffers plus AllocInfo::alloc_base):
 *           0x80147000             video_table_base   (30 x 32 = 0x3C0)
 *           video_table_base+0x3C0 video_data_base
 *           0x80156000             audio_data_base
 *           0x8015E000             vlc_input_buf[0]   (0x11000)
 *           0x8016F000             vlc_input_buf[1]
 *           alloc_base             vlc_table
 *           alloc_base+0x11000     mdec_output_buf[0] (0x1E00 = 16 x 240 x 2)
 *           alloc_base+0x12E00     mdec_output_buf[1]
 *       rects[2] is 16 wide here.
 *
 * @see https://decomp.me/scratch/g5PtA (100%)
 */
void movie_init(s32 resource_index, s32 flags, s32 total_frames, s32 init_buffer_idx)
{
    AllocInfo* alloc_info = g_allocInfo;
    MovieState* ms;

    MOVIE_STATE->gpu_mode = flags & 0x7F;
    if (flags & 0x80)
    {
        MOVIE_STATE->use_cd_audio = 1;
    }
    else
    {
        MOVIE_STATE->use_cd_audio = 0;
    }
    if (MOVIE_STATE->gpu_mode == 0)
    {

        MOVIE_STATE->video_table_base = (VideoSectorEntry*)0x80147000;
        MOVIE_STATE->audio_data_base = (AudioSector*)0x80160000;
        MOVIE_STATE->vlc_table = (void*)0x80168000;
        MOVIE_STATE->vlc_input_buf[0] = (void*)0x80179000;
        MOVIE_STATE->vlc_input_buf[1] = (void*)0x8018D000;
        MOVIE_STATE->mdec_output_buf[0] = (u_long*)0x801A1000;
        MOVIE_STATE->mdec_output_buf[1] = (u_long*)0x801A3D00;
        MOVIE_STATE->rects[1].x = 0;
        MOVIE_STATE->rects[0].x = 0;
        MOVIE_STATE->rects[0].y = 0;
        MOVIE_STATE->rects[1].y = 0xF0;
        MOVIE_STATE->rects[2].h = 0xF0;
        MOVIE_STATE->rects[1].w = 0x1E0;
        MOVIE_STATE->rects[0].w = 0x1E0;
        MOVIE_STATE->rects[2].w = 0x18;
        MOVIE_STATE->video_ring_capacity = STANDARD_VIDEO_RING_SLOTS;
        MOVIE_STATE->rects[1].h = 0xF0;
        MOVIE_STATE->rects[0].h = 0xF0;
        MOVIE_STATE->rects[2].x = 0;
        MOVIE_STATE->rects[2].y = 0;
        MOVIE_STATE->audio_ring_capacity = AUDIO_RING_SLOTS;
        MOVIE_STATE->video_data_base =
            (VideoVlcPayload*)&MOVIE_STATE->video_table_base[STANDARD_VIDEO_RING_SLOTS];
        VOL_MOVIE_STATE->chunk_idx = 0;
    }
    else
    {
        MOVIE_STATE->video_table_base = (VideoSectorEntry*)0x80147000;
        MOVIE_STATE->audio_data_base = (AudioSector*)0x80156000;
        MOVIE_STATE->vlc_table = alloc_info->alloc_base;
        MOVIE_STATE->vlc_input_buf[0] = (void*)0x8015E000;
        MOVIE_STATE->vlc_input_buf[1] = (void*)0x8016F000;
        MOVIE_STATE->mdec_output_buf[0] = (u_long*)(alloc_info->alloc_base + 0x11000);
        MOVIE_STATE->mdec_output_buf[1] = (u_long*)(alloc_info->alloc_base + 0x12E00);
        if (((s16)MOVIE_STATE->rects[0].x) >= 0x300)
        {
            MOVIE_STATE->rects[1].x = 0x200;
            MOVIE_STATE->rects[1].y = 0;
        }
        else
        {
            MOVIE_STATE->rects[1].x = (u16)(MOVIE_STATE->rects[0].x + MOVIE_STATE->rects[0].w);
            MOVIE_STATE->rects[1].y = MOVIE_STATE->rects[0].y;
        }
        MOVIE_STATE->rects[1].w = MOVIE_STATE->rects[0].w;
        MOVIE_STATE->rects[1].h = MOVIE_STATE->rects[0].h;
        MOVIE_STATE->rects[2].h = MOVIE_STATE->rects[0].h;
        MOVIE_STATE->rects[2].x = MOVIE_STATE->rects[init_buffer_idx].x;
        MOVIE_STATE->rects[2].y = (unsigned short)MOVIE_STATE->rects[init_buffer_idx].y;
        MOVIE_STATE->rects[2].w = 0x10;
        MOVIE_STATE->video_ring_capacity = ALTERNATE_VIDEO_RING_SLOTS;
        MOVIE_STATE->audio_ring_capacity = AUDIO_RING_SLOTS;
        MOVIE_STATE->video_data_base =
            (VideoVlcPayload*)&MOVIE_STATE->video_table_base[ALTERNATE_VIDEO_RING_SLOTS];
        VOL_MOVIE_STATE->chunk_idx = init_buffer_idx;
    }

    ms = MOVIE_STATE;

    ms->resource_index = resource_index;
    ms->current_frame = 0;
    ms->total_frames = total_frames;
    ms->input_buf_idx = 0;
    ms->vlc_retry_count = 0;
    ms->mdec_retry_pending = 0;
    ms->busy = 0;
    ms->draw_sync_target = 0;
    ms->out_buf_idx = 0;
    ms->pending_vram_upload = 0;
    ms->pending_mdec_decode = 0;
    ms->mdec_busy = 0;
    ms->frame_ready = 0;
    ms->end_of_stream = 0;
    ms->end_state = 0;
    ms->audio_stream_state = 0;
    ms->video_write_idx = 0;
    ms->video_read_idx = 0;
    ms->video_ring_size = 0;
    ms->audio_write_idx = 0;
    ms->audio_read_idx = 0;
    ms->audio_ring_size = 0;
    ms->audio_buffered_count = 0;
    ms->frame_number = 0;
    ms->continuation_type = 0;
    ms->sectors_remaining = 0;
    ms->last_video_frame = (u32)(-1);
    ms->last_consumed_video_frame = (u32)(-1);
    ms->last_audio_frame = (u32)(-1);
    ms->last_consumed_audio_frame = (u32)(-1);
    ms->dec_dct_out_callback.address = DecDCToutCallback(&movie_mdec_out_callback);
    ms->draw_sync_callback.address = DrawSyncCallback(&draw_sync_callback);
    if (ms->use_cd_audio != 0)
    {
        akao_cmd_e8_start_xa_stream((u32)ms->audio_data_base, (u32)(ms->audio_ring_capacity << 0xB));
        akao_cmd_e4_set_cd_volume(0x7F);
    }
    else
    {
        akao_cmd_c8(0x7FFF);
        akao_xa_setup_panning(0xA0);
    }
    cdrom_wait_queue_empty();
    cdrom_queue_command(CdlReadS, (s16)resource_index, NULL, cd_sector_callback);

    /* Re-establish the state base after queuing the CD callback. */
    ms = MOVIE_STATE;

    if (g_gpuMode == 0)
    {
        VSync(0);
        SetDispMask(0);
        ClearImage(&ms->rects[0], 0, 0, 0);
        ClearImage(&ms->rects[1], 0, 0, 0);
        DecDCTvlcBuild(ms->vlc_table);
        DrawSync(0);
    }
}

/**
 * @brief Per-tick movie pump: feeds the MDEC and advances the audio queue.
 *
 * Three responsibilities:
 *   1. If a previous MDEC submit was deferred, retry it now that the MDEC is idle.
 *   2. Pull the next BS frame from the video ring, decode VLC, and submit to MDEC.
 *   3. Drain the audio ring into the AKAO XA stream and update playback position.
 *
 * @see https://decomp.me/scratch/NpM84 (100%)
 */
void movie_update(void)
{
    s32 audio_capacity;

    VideoVlcPayload* vlc_payload;   /* raw bitstream for the next video frame */
    VideoSectorEntry* entry_header; /* 32-byte sector header for the same frame */

    s32 vlc_complete = 0;
    MovieState* state = MOVIE_STATE;

    if (g_mdecRetryPending != 0)
    {
        if ((MOVIE_STATE->mdec_busy == 0) && (state->frame_ready == 0))
        {
            MOVIE_STATE->mdec_busy = 1;
            DecDCTin(MOVIE_STATE->vlc_input_buf[MOVIE_STATE->input_buf_idx], (MOVIE_STATE->gpu_mode & 0xFFFFu) == 0);
            {
                s32 pixel_count = ((s16)MOVIE_STATE->rects[2].w) * ((s16)MOVIE_STATE->rects[2].h);
                s32 word_count = pixel_count + (((u32)pixel_count) >> 31);
                DecDCTout((MOVIE_STATE->mdec_output_buf[MOVIE_STATE->out_buf_idx]), word_count >> 1);
            }
            MOVIE_STATE->mdec_retry_pending = 0;
        }
    }

    if ((g_mdecRetryPending == 0) & 0xFFFFu)
    {
        u8 retry_count = MOVIE_STATE->vlc_retry_count;
        if (retry_count != 0)
        {
            retry_count--;
            MOVIE_STATE->vlc_retry_count = retry_count;
            if ((retry_count & 0xFF) == 0)
            {
                DecDCTvlcSize2(0);
            }
            if (DecDCTvlc2(0, 0, MOVIE_STATE->vlc_table) == 0)
            {
                vlc_complete = 1;
                MOVIE_STATE->vlc_retry_count = 0;
            }
        }
        else if (get_next_video_entry(&vlc_payload, &entry_header) != 0)
        {
            MOVIE_STATE->current_frame = entry_header->header.frame_number;

            if ((entry_header->header.frame_number >= MOVIE_STATE->total_frames) &&
                (MOVIE_STATE->end_state == END_STATE_RUNNING))
            {
                MOVIE_STATE->end_state = END_STATE_NEAR_END;
            }

            MOVIE_STATE->input_buf_idx = 1 - MOVIE_STATE->input_buf_idx;

            if (MOVIE_STATE->gpu_mode == 0)
            {
                DecDCTvlcSize2(0x1000);
                MOVIE_STATE->vlc_retry_count = 3;
            }
            else
            {
                DecDCTvlcSize2(0x16AA);
                MOVIE_STATE->vlc_retry_count = 1;
            }
            if (DecDCTvlc2((u_long*)vlc_payload, MOVIE_STATE->vlc_input_buf[MOVIE_STATE->input_buf_idx],
                           MOVIE_STATE->vlc_table) == 0)
            {
                vlc_complete = 1;
                MOVIE_STATE->vlc_retry_count = 0;
            }
        }
        else if ((MOVIE_STATE->end_of_stream != 0) && (MOVIE_STATE->mdec_busy == 0))
        {
            MOVIE_STATE->end_state = END_STATE_DONE;
        }
    }

    if (vlc_complete != 0)
    {
        /* Reused for two short-lived values to preserve register allocation. */
        s32 temp_flag;
        advance_video_read();

        if ((MOVIE_STATE->mdec_busy == 0) && (temp_flag = MOVIE_STATE->frame_ready == 0))
        {
            MOVIE_STATE->mdec_busy = 1;
            DecDCTin(MOVIE_STATE->vlc_input_buf[MOVIE_STATE->input_buf_idx], MOVIE_STATE->gpu_mode == 0);
            {
                s32 pixel_count = ((s16)MOVIE_STATE->rects[2].w) * ((s16)MOVIE_STATE->rects[2].h);
                temp_flag = ((u32)pixel_count) >> 31;
                DecDCTout(MOVIE_STATE->mdec_output_buf[MOVIE_STATE->out_buf_idx],
                          (pixel_count + temp_flag) >> 1);
            }
        }
        else
        {
            g_mdecRetryPending = 1;
        }
    }

    /* Match-sensitive state-base reloads preserve the original schedule. */
    state = MOVIE_STATE;
    if (g_cdAudioReady != 0)
    {
        /* Reusing vlc_payload for the audio entry preserves the 0x28-byte frame. */
        if (get_next_audio_entry((void*)&vlc_payload) != 0)
        {
            /* This otherwise-dead assignment preserves the original spill. */
            entry_header = (VideoSectorEntry*)vlc_payload;
            MOVIE_STATE->current_frame = ((AudioSector*)vlc_payload)->header.frame_number;

            if ((((AudioSector*)vlc_payload)->header.frame_number > MOVIE_STATE->total_frames) &&
                (MOVIE_STATE->end_state < END_STATE_DONE))
            {
                MOVIE_STATE->end_state = END_STATE_DONE;
            }
            akao_xa_advance_frame();
        }
        state = MOVIE_STATE;
        if (g_audioStreamState == 2)
        {

            audio_capacity = MOVIE_STATE->audio_ring_capacity;

            if ((s32)MOVIE_STATE->audio_buffered_count >= ((s32)(audio_capacity >> 1)))
            {
                akao_cmd_98_9a_9c_9e(3);
                MOVIE_STATE->audio_stream_state = 0;
            }
        }

        if ((MOVIE_STATE->audio_write_idx != MOVIE_STATE->audio_read_idx) ||
            (MOVIE_STATE->last_audio_frame != MOVIE_STATE->last_consumed_audio_frame))
        {
            s32 position = akao_xa_get_position();
            if (((position != -1) && (MOVIE_STATE->audio_buffered_count != 0)) &&
                (MOVIE_STATE->audio_read_idx != (position * 2)))
            {
                advance_audio_read();
            }
        }
    }
}

/**
 * @brief MDEC-output completion callback.
 *
 * Invoked when the MDEC finishes writing a decoded macroblock buffer.
 * Either uploads the result to VRAM immediately (LoadImage / LoadImage2)
 * or defers the upload by setting @ref MovieState::pending_vram_upload.
 *
 * @see https://decomp.me/scratch/HVkZ6 (100%)
 */
void movie_mdec_out_callback(void)
{
    volatile MovieState* base = VOL_MOVIE_STATE;
    s32 temp;
    int zero_literal = 0; /* Required to keep zero live across both transfer paths. */

    if (g_gpuMode == 0)
    {
        if (((u8)g_cdStatusByte3) == 1)
        {
            cdrom_verify_recovery();
        }
        temp = DrawSync(1);
        if (temp < 2)
        {
            LoadImage(&MOVIE_STATE->rects[2], base->mdec_output_buf[base->out_buf_idx]);
            base->draw_sync_target = temp + 1;
        }
        else
        {
            base->pending_vram_upload = 1U;
        }
    }
    else
    {
        temp = (s32)BreakDraw();
        if (temp != (-1))
        {
            LoadImage2(&MOVIE_STATE->rects[2], base->mdec_output_buf[base->out_buf_idx]);
            if (temp != zero_literal)
            {
                DrawOTag((u_long*)temp);
            }
        }
        else
        {
            LoadImage(&MOVIE_STATE->rects[2], base->mdec_output_buf[base->out_buf_idx]);
        }
    }

    if (MOVIE_STATE->pending_vram_upload == zero_literal)
    {
        movie_schedule_next_decode();
        return;
    }
    MOVIE_STATE->mdec_busy = 1;
}

/**
 * @brief Advance the decode position and schedule the next MDEC decode.
 *
 * Steps the rect[2] frame position by its width. If still inside the current
 * chunk, kicks off DecDCTout immediately (or sets @ref MovieState::pending_mdec_decode
 * if the GPU is busy). Otherwise toggles to the other chunk, resets the frame
 * position, and signals @ref MovieState::frame_ready.
 *
 * @see https://decomp.me/scratch/E7XCZ (100%)
 */
void movie_schedule_next_decode(void)
{
    u16 next_out_buf_idx;
    u16 cur_frame_pos;
    u16 frame_step;
    u16 new_frame_pos;
    s32 new_frame_pos_signed;
    s32 chunk_end;
    u32 decode_size;
    s32 decode_word_count;

    next_out_buf_idx = 1 - MOVIE_STATE->out_buf_idx;

    cur_frame_pos = MOVIE_STATE->rects[2].x;
    frame_step = MOVIE_STATE->rects[2].w;

    new_frame_pos = cur_frame_pos + frame_step;

    MOVIE_STATE->rects[2].x = new_frame_pos;

    new_frame_pos_signed = (s16)new_frame_pos;

    VOL_MOVIE_STATE->out_buf_idx = next_out_buf_idx;

    chunk_end =
        MOVIE_STATE->rects[(u8)VOL_MOVIE_STATE->chunk_idx].x + MOVIE_STATE->rects[(u8)VOL_MOVIE_STATE->chunk_idx].w;

    if (new_frame_pos_signed < chunk_end)
    {
        if (MOVIE_STATE->draw_sync_target < 2U)
        {
            decode_size = ((s16)frame_step) * MOVIE_STATE->rects[2].h;
            decode_word_count = ((int)(decode_size + (decode_size >> 31))) >> 1;
            DecDCTout(MOVIE_STATE->mdec_output_buf[MOVIE_STATE->out_buf_idx], decode_word_count);
            VOL_MOVIE_STATE->mdec_busy = 2;
        }
        else
        {
            MOVIE_STATE->mdec_busy = 1;
            VOL_MOVIE_STATE->pending_mdec_decode = 1;
        }
    }
    else
    {
        /* advance to the next chunk and reset the frame position */
        MOVIE_STATE->chunk_idx = 1 - MOVIE_STATE->chunk_idx;
        MOVIE_STATE->rects[2].x = MOVIE_STATE->rects[(u8)VOL_MOVIE_STATE->chunk_idx].x;
        MOVIE_STATE->rects[2].y = MOVIE_STATE->rects[(u8)VOL_MOVIE_STATE->chunk_idx].y;
        VOL_MOVIE_STATE->frame_ready = 1;
        VOL_MOVIE_STATE->mdec_busy = 0;
        if (VOL_MOVIE_STATE->end_state == END_STATE_NEAR_END)
        {
            MOVIE_STATE->end_state = END_STATE_DONE;
        }
    }
}

/**
 * @brief Service pending video output operations.
 *
 * Two flags gate the two halves:
 *   - pending_vram_upload: a decoded frame is ready; DMA it into VRAM
 *     (LoadImage) and kick off the MDEC decode of the next frame.
 *   - pending_mdec_decode: new BS bitstream data is staged; feed it to the
 *     MDEC (DecDCTout).
 *
 * gpu_mode selects the transfer path:
 *   - 0: wait for DrawSync then use LoadImage (standard DMA).
 *   - non-zero: interrupt the current draw via BreakDraw then use LoadImage2.
 *
 * @see https://decomp.me/scratch/JTTFr (100%)
 */
void movie_service_video_ops(void)
{
    volatile MovieState* G = VOL_MOVIE_STATE;
    s32 word_count;
    u_long* break_draw_result;
    if (!G->pending_vram_upload)
    {
        if (!G->pending_mdec_decode)
        {
            return;
        }
    }
    if (VOL_MOVIE_STATE->gpu_mode == 0)
    {
        if (DrawSync(1) >= 2)
        {
            return;
        }
        if (VOL_MOVIE_STATE->pending_vram_upload)
        {
            MOVIE_STATE->busy = 1;
            if (VOL_MOVIE_STATE->pending_vram_upload)
            {

                LoadImage(&MOVIE_STATE->rects[2], G->mdec_output_buf[VOL_MOVIE_STATE->out_buf_idx]);
                VOL_MOVIE_STATE->draw_sync_target = DrawSync(1) + 1;
                VOL_MOVIE_STATE->pending_vram_upload = 0;
                movie_schedule_next_decode();
            }
            VOL_MOVIE_STATE->busy = 0;
        }
        G = VOL_MOVIE_STATE;
        if (G->pending_mdec_decode)
        {
            MOVIE_STATE->busy = 1;
            if (G->pending_mdec_decode)
            {
                s32 temp;

                /* word count = (width * height) / 2, rounded toward zero for signed values */
                temp = ((s32)G->rects[2].w) * ((s32)G->rects[2].h);
                word_count = temp + (((u32)temp) >> 31);
                DecDCTout(G->mdec_output_buf[G->out_buf_idx], word_count >> 1);
                G->pending_mdec_decode = 0;
            }
            MOVIE_STATE->busy = 0;
        }
    }
    else
    {
        /* BreakDraw path: interrupt the current draw primitive list to upload immediately */
        if (G->pending_vram_upload)
        {
            MOVIE_STATE->busy = 1;
            if (G->pending_vram_upload)
            {
                s32 bd;

                break_draw_result = BreakDraw();
                bd = (s32)break_draw_result;
                if (bd != (-1))
                {
                    LoadImage2(&MOVIE_STATE->rects[2], G->mdec_output_buf[G->out_buf_idx]);
                    if (bd != 0)
                    {
                        /* Resume the interrupted OTag list */
                        DrawOTag((u_long*)bd);
                    }
                    movie_schedule_next_decode();
                    G->pending_vram_upload = 0;
                }
            }
            g_busy = 0;
        }
    }
}

/**
 * @brief CD sector-arrival callback; invoked from the CD-ROM ISR per sector.
 *
 * Reads the 8-word (32-byte) sector header, classifies the sector as video
 * (type 0x8001) or audio, checks whether the corresponding ring buffer has
 * room, then copies the 504-word (2016-byte) payload into the buffer and
 * advances the write index. Multi-sector frames are handled via
 * @ref g_sectorsRemaining: when 0 the sector is the first (header) sector;
 * when non-zero we are reading continuation sectors for the same frame and
 * skip the ring-capacity check.
 *
 * @note The CD subsystem passes transfer counters to callbacks; this handler
 *       intentionally ignores them and returns a boolean continuation sentinel.
 *
 * @return 1 to keep streaming, 0 when the stream has ended or should pause.
 *
 * @see https://decomp.me/scratch/5flHR (100%)
 */
s32 cd_sector_callback(void)
{
    VideoSectorEntry sector_header;
    s32 audio_read_idx_l;
    s32 has_room;
    s32 has_more_frames;
    u8* payload_dst;
    SectorEntry* sector_hdr_ptr;
    s32 audio_write_next;
    s32 video_write_idx_l;
    s32 video_read_idx_l;

    has_room = 0;

    if (MOVIE_STATE->sectors_remaining == 0)
    {
        volatile MovieState* vms;
        u32* hdr_words;
        while (CdGetSector(&sector_header, CD_HEADER_WORDS) == 0);

        vms = VOL_MOVIE_STATE;
        if (((u32)sector_header.header.frame_number) > MOVIE_STATE->total_frames)
        {
            VOL_MOVIE_STATE->end_of_stream = 1;
            return 0;
        }

        vms->frame_number = sector_header.header.frame_number;

        if (sector_header.header.chunk_sector_idx != 0)
        {
            return 1;
        }

        if (sector_header.header.sector_type == SECTOR_TYPE_VIDEO)
        {

            video_write_idx_l = vms->video_write_idx;
            video_read_idx_l = vms->video_read_idx;

            if (((video_write_idx_l == video_read_idx_l) &&
                 (vms->last_video_frame == vms->last_consumed_video_frame)) ||
                ((video_write_idx_l != video_read_idx_l) && (video_read_idx_l < vms->video_write_idx)))
            {

                if (vms->video_ring_capacity < (vms->video_write_idx + sector_header.header.sector_count))
                {
                    if (video_read_idx_l >= sector_header.header.sector_count)
                    {
                        has_room = 1;
                        vms->video_ring_size = vms->video_write_idx;
                        vms->video_write_idx = 0;
                    }
                }
                else
                {
                    has_room = 1;
                }
            }
            else if (video_write_idx_l != video_read_idx_l)
            {
                if (video_read_idx_l >= (vms->video_write_idx + sector_header.header.sector_count))
                {
                    has_room = 1;
                }
            }

            if (has_room != 0)
            {

                s32 write_index;
                u8* sector_ptr;

                sector_ptr = MOVIE_STATE->video_data_base[MOVIE_STATE->video_write_idx].data;
                while (CdGetSector(sector_ptr, CD_PAYLOAD_WORDS) == 0);

                /* Bulk-copy the raw 32-byte CD header as 8 u32 words. */
                hdr_words = (u32*)&sector_header;

                write_index = MOVIE_STATE->video_write_idx;
                sector_ptr = (u8*)&MOVIE_STATE->video_table_base[write_index];

                ((u32*)sector_ptr)[0] = hdr_words[0];
                ((u32*)sector_ptr)[1] = hdr_words[1];
                ((u32*)sector_ptr)[2] = hdr_words[2];
                ((u32*)sector_ptr)[3] = hdr_words[3];
                ((u32*)sector_ptr)[4] = hdr_words[4];
                ((u32*)sector_ptr)[5] = hdr_words[5];
                ((u32*)sector_ptr)[6] = hdr_words[6];
                ((u32*)sector_ptr)[7] = hdr_words[7];

                MOVIE_STATE->sectors_remaining = sector_header.header.sector_count - 1;
                if (!(MOVIE_STATE->sectors_remaining & 0xFFFF))
                {
                    u32 total_frames;

                    total_frames = MOVIE_STATE->total_frames;
                    VOL_MOVIE_STATE->video_write_idx = VOL_MOVIE_STATE->video_write_idx + 1;

                    VOL_MOVIE_STATE->last_video_frame = VOL_MOVIE_STATE->frame_number;

                    has_more_frames = VOL_MOVIE_STATE->frame_number < total_frames;

                    goto check_end_of_stream;
                }
                else
                {
                    MOVIE_STATE->continuation_type = CONTINUATION_VIDEO;
                    MOVIE_STATE->chunk_sector_idx = 1U;
                }
            }
        }
        else
        {
            MovieState* ms;
            s32 audio_write_idx_l;
            audio_write_idx_l = vms->audio_write_idx;
            audio_read_idx_l = vms->audio_read_idx;

            if (((audio_write_idx_l == audio_read_idx_l) &&
                 (vms->last_audio_frame == vms->last_consumed_audio_frame)) ||
                ((audio_write_idx_l != audio_read_idx_l) && (audio_read_idx_l < vms->audio_write_idx)))
            {
                if (vms->audio_ring_capacity < (vms->audio_write_idx + sector_header.header.sector_count))
                {
                    if (audio_read_idx_l >= sector_header.header.sector_count)
                    {
                        has_room = 1;
                        vms->audio_ring_size = vms->audio_write_idx;
                        vms->audio_write_idx = 0;
                    }
                }
                else
                {
                    has_room = 1;
                }
            }
            else if ((audio_write_idx_l != audio_read_idx_l) &&
                     (audio_read_idx_l >= (vms->audio_write_idx + sector_header.header.sector_count)))
            {
                has_room = 1;
            }

            if (has_room != 0)
            {
                u8* sector_ptr;

                sector_ptr = MOVIE_STATE->audio_data_base[VOL_MOVIE_STATE->audio_write_idx].payload;
                while (CdGetSector(sector_ptr, CD_PAYLOAD_WORDS) == 0);

                hdr_words = (u32*)&sector_header;
                sector_ptr = (u8*)&MOVIE_STATE->audio_data_base[VOL_MOVIE_STATE->audio_write_idx];
                ((u32*)sector_ptr)[0] = hdr_words[0];
                ((u32*)sector_ptr)[1] = hdr_words[1];
                ((u32*)sector_ptr)[2] = hdr_words[2];
                ((u32*)sector_ptr)[3] = hdr_words[3];
                ((u32*)sector_ptr)[4] = hdr_words[4];
                ((u32*)sector_ptr)[5] = hdr_words[5];
                ((u32*)sector_ptr)[6] = hdr_words[6];
                ((u32*)sector_ptr)[7] = hdr_words[7];
                MOVIE_STATE->sectors_remaining = sector_header.header.sector_count - 1;
                if (!(MOVIE_STATE->sectors_remaining & 0xFFFF))
                {
                    VOL_MOVIE_STATE->audio_write_idx = VOL_MOVIE_STATE->audio_write_idx + 1;
                    VOL_MOVIE_STATE->last_audio_frame = VOL_MOVIE_STATE->frame_number;

                    if (VOL_MOVIE_STATE->frame_number > MOVIE_STATE->total_frames)
                    {
                        return 0;
                    }
                }
                else
                {
                    MOVIE_STATE->continuation_type = CONTINUATION_AUDIO;
                    MOVIE_STATE->chunk_sector_idx = 1U;
                }
            }
            ms = MOVIE_STATE;
            if (g_audioStreamState == 1)
            {
                ms->audio_stream_state = 2;
            }
            return 1;
        }
    }
    else if (MOVIE_STATE->continuation_type == CONTINUATION_VIDEO)
    {
        sector_hdr_ptr =
            &MOVIE_STATE->video_table_base[VOL_MOVIE_STATE->video_write_idx + MOVIE_STATE->chunk_sector_idx].header;
        while (CdGetSector(sector_hdr_ptr, CD_HEADER_WORDS) == 0);

        if (((sector_hdr_ptr->sector_type == SECTOR_TYPE_VIDEO) &&
             (sector_hdr_ptr->frame_number == MOVIE_STATE->frame_number)) &&
            (sector_hdr_ptr->chunk_sector_idx == MOVIE_STATE->chunk_sector_idx))
        {
            payload_dst =
                MOVIE_STATE
                    ->video_data_base[VOL_MOVIE_STATE->video_write_idx + (MOVIE_STATE->chunk_sector_idx & 0xFFFF)]
                    .data;
            while (CdGetSector(payload_dst, CD_PAYLOAD_WORDS) == 0);

            MOVIE_STATE->sectors_remaining = MOVIE_STATE->sectors_remaining - 1;
            if (!(MOVIE_STATE->sectors_remaining))
            {
                u32 offset;
                u32 total_frames;
                total_frames = MOVIE_STATE->total_frames;

                offset = 1;
                VOL_MOVIE_STATE->video_write_idx =
                    (VOL_MOVIE_STATE->video_write_idx + offset) + MOVIE_STATE->chunk_sector_idx;

                VOL_MOVIE_STATE->last_video_frame = VOL_MOVIE_STATE->frame_number;
                has_more_frames = ((u32)sector_hdr_ptr->frame_number) < total_frames;

            check_end_of_stream:
                if (has_more_frames == 0)
                {
                    return 0;
                }

                /* Dead store preserved for codegen match. */
                has_more_frames = (u32)sector_hdr_ptr->frame_number;
                return 1;
            }
            else
            {
                MOVIE_STATE->chunk_sector_idx = (u16)(MOVIE_STATE->chunk_sector_idx + 1);
            }
        }
        else
        {
            MOVIE_STATE->frame_number = (u32)sector_hdr_ptr->frame_number;
            MOVIE_STATE->sectors_remaining = 0U;

            if (MOVIE_STATE->total_frames > ((u32)sector_hdr_ptr->frame_number))
            {
                return 1;
            }

            VOL_MOVIE_STATE->end_of_stream = 1;
            return 0;
        }
    }
    else
    {
        sector_hdr_ptr =
            &MOVIE_STATE->audio_data_base[VOL_MOVIE_STATE->audio_write_idx + MOVIE_STATE->chunk_sector_idx].header;
        while (CdGetSector(sector_hdr_ptr, CD_HEADER_WORDS) == 0);

        if (((sector_hdr_ptr->sector_type == SECTOR_TYPE_AUDIO) &&
             (sector_hdr_ptr->frame_number == MOVIE_STATE->frame_number)) &&
            (sector_hdr_ptr->chunk_sector_idx == MOVIE_STATE->chunk_sector_idx))
        {
            payload_dst =
                MOVIE_STATE
                    ->audio_data_base[VOL_MOVIE_STATE->audio_write_idx + (MOVIE_STATE->chunk_sector_idx & 0xFFFF)]
                    .payload;
            while (CdGetSector(payload_dst, CD_PAYLOAD_WORDS) == 0);

            MOVIE_STATE->sectors_remaining = MOVIE_STATE->sectors_remaining - 1;
            if (!(MOVIE_STATE->sectors_remaining & 0xFFFF))
            {
                audio_write_next = VOL_MOVIE_STATE->audio_write_idx + 1;
                VOL_MOVIE_STATE->audio_write_idx = audio_write_next + MOVIE_STATE->chunk_sector_idx;

                MOVIE_STATE->last_audio_frame = VOL_MOVIE_STATE->frame_number;
                if (((u32)sector_hdr_ptr->frame_number) > MOVIE_STATE->total_frames)
                {
                    return 0;
                }
            }
            else
            {
                MOVIE_STATE->chunk_sector_idx = (u16)(MOVIE_STATE->chunk_sector_idx + 1);
            }
        }
        else
        {
            MOVIE_STATE->frame_number = (u32)sector_hdr_ptr->frame_number;
            MOVIE_STATE->sectors_remaining = 0U;
            if (((u32)sector_hdr_ptr->frame_number) <= MOVIE_STATE->total_frames)
            {
                return 1;
            }

            VOL_MOVIE_STATE->end_of_stream = 1;
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Find the next unqueued audio ring entry and return its address.
 *
 * Walks the audio ring forward from audio_read_idx skipping the
 * already-buffered entries, wrapping at the ring size. Charges the entry's
 * sector count to @ref MovieState::audio_buffered_count.
 *
 * @param out_entry Receives a pointer to the entry's 2048-byte CD sector
 *                 in @ref MovieState::audio_data_base. Untouched if no entry
 *                 is available.
 *
 * @return 1 if an entry was produced, 0 if the ring is empty or fully queued.
 *
 * @see https://decomp.me/scratch/I2Ddr (100%)
 */
s32 get_next_audio_entry(AudioSector** out_entry)
{
    s32 next_idx;
    AudioSector* entry;

    if ((MOVIE_STATE->audio_write_idx == MOVIE_STATE->audio_read_idx) &&
        (MOVIE_STATE->last_audio_frame == MOVIE_STATE->last_consumed_audio_frame))
    {
        return 0;
    }

    if ((VOL_MOVIE_STATE->audio_write_idx <= MOVIE_STATE->audio_read_idx) &&
        (MOVIE_STATE->audio_read_idx == MOVIE_STATE->audio_ring_size))
    {
        MOVIE_STATE->audio_read_idx = 0;

        if (MOVIE_STATE->audio_write_idx == 0 &&
            (MOVIE_STATE->last_audio_frame == MOVIE_STATE->last_consumed_audio_frame))
        {
            return 0;
        }
    }

    /* Skip entries already queued to the audio pipeline. */
    next_idx = MOVIE_STATE->audio_read_idx + MOVIE_STATE->audio_buffered_count;

    if ((MOVIE_STATE->audio_read_idx >= MOVIE_STATE->audio_write_idx) && (next_idx >= VOL_MOVIE_STATE->audio_ring_size))
    {
        next_idx -= MOVIE_STATE->audio_ring_size;
    }

    if ((next_idx == MOVIE_STATE->audio_write_idx) && (MOVIE_STATE->audio_buffered_count != 0))
    {
        return 0;
    }

    entry = &MOVIE_STATE->audio_data_base[next_idx];
    MOVIE_STATE->audio_buffered_count += entry->header.sector_count;
    *out_entry = entry;
    return 1;
}

/**
 * @brief DrawSync completion callback; flushes deferred VRAM upload / MDEC submit.
 *
 * If the GPU is idle (g_busy == 0), services @ref MovieState::pending_vram_upload
 * (LoadImage + schedule next decode) and @ref MovieState::pending_mdec_decode
 * (DecDCTout for the staged buffer).
 *
 * @see https://decomp.me/scratch/TApbR (100%)
 */
void draw_sync_callback(void)
{
    s16 width;
    s16 height;
    volatile MovieState* movie_state = VOL_MOVIE_STATE;

    if (g_busy != 0)
    {
        return;
    }

    movie_state->draw_sync_target = 0;

    if (movie_state->pending_vram_upload != 0)
    {

        LoadImage(&MOVIE_STATE->rects[2], MOVIE_STATE->mdec_output_buf[movie_state->out_buf_idx]);
        movie_schedule_next_decode();
        movie_state->pending_vram_upload = 0;
    }

    if (movie_state->pending_mdec_decode != 0)
    {
        width = movie_state->rects[2].w;
        height = movie_state->rects[2].h;

        DecDCTout(MOVIE_STATE->mdec_output_buf[movie_state->out_buf_idx], (width * height) / 2);
        movie_state->pending_mdec_decode = 0;
    }
}

/**
 * @brief Resolve pointers to the next available video ring entry.
 *
 * Wraps video_read_idx if it has reached video_ring_size. On success returns
 * pointers to the entry's VLC data buffer and 32-byte header.
 *
 * @param out_vlc_data     Receives the 2016-byte VLC data buffer pointer.
 * @param out_entry_header Receives the 32-byte entry header pointer.
 *
 * @return 1 if an entry is available, 0 otherwise (ring empty and last frame consumed).
 *
 * @see https://decomp.me/scratch/OJvsJ (100%)
 */
s32 get_next_video_entry(VideoVlcPayload** out_vlc_data, VideoSectorEntry** out_entry_header)
{
    s32 read_idx;
    s32 write_idx;

    if ((MOVIE_STATE->video_write_idx == MOVIE_STATE->video_read_idx) &&
        (MOVIE_STATE->last_video_frame == MOVIE_STATE->last_consumed_video_frame))
    {
        return 0;
    }

    write_idx = VOL_MOVIE_STATE->video_write_idx;
    read_idx = VOL_MOVIE_STATE->video_read_idx;

    if ((read_idx >= write_idx) && (read_idx == MOVIE_STATE->video_ring_size))
    {
        MOVIE_STATE->video_read_idx = 0;

        if ((MOVIE_STATE->video_write_idx == 0) &&
            (MOVIE_STATE->last_video_frame == MOVIE_STATE->last_consumed_video_frame))
        {
            return 0;
        }
    }

    *out_entry_header = &MOVIE_STATE->video_table_base[MOVIE_STATE->video_read_idx];
    *out_vlc_data = &MOVIE_STATE->video_data_base[MOVIE_STATE->video_read_idx];
    return 1;
}

/**
 * @brief Advance video_read_idx past the entry currently being consumed.
 *
 * Reads sector_count from the entry header to step the index, wraps to 0
 * when the new index hits video_ring_size, and records the consumed frame
 * number in @ref MovieState::last_consumed_video_frame.
 *
 * @see https://decomp.me/scratch/SUBK5 (100%)
 */
void advance_video_read(void)
{
    SectorEntry* entry;
    s32 next_index;

    entry = &MOVIE_STATE->video_table_base[MOVIE_STATE->video_read_idx].header;
    next_index = MOVIE_STATE->video_read_idx + entry->sector_count;

    if ((MOVIE_STATE->video_read_idx >= MOVIE_STATE->video_write_idx) && (next_index == MOVIE_STATE->video_ring_size))
    {
        next_index = 0;
    }

    MOVIE_STATE->last_consumed_video_frame = entry->frame_number;
    MOVIE_STATE->video_read_idx = next_index;
}

/**
 * @brief Advance audio_read_idx past the entry currently being consumed.
 *
 * Reads sector_count from the entry header to step the index, decrements
 * @ref MovieState::audio_buffered_count, wraps to 0 when the new index hits
 * video_ring_size (note: not audio_ring_size - see note below), and records
 * the consumed frame number in @ref MovieState::last_consumed_audio_frame.
 *
 * @note Comparing the audio next_index against video_ring_size (not
 *       audio_ring_size) appears to be an original-game bug; preserved
 *       verbatim to keep the asm matching.
 *
 * @see https://decomp.me/scratch/6Xjsu (100%)
 */
void advance_audio_read(void)
{
    SectorEntry* entry;
    s32 next_index;

    entry = &MOVIE_STATE->audio_data_base[MOVIE_STATE->audio_read_idx].header;
    next_index = MOVIE_STATE->audio_read_idx + entry->sector_count;

    MOVIE_STATE->audio_buffered_count = MOVIE_STATE->audio_buffered_count - entry->sector_count;

    if ((MOVIE_STATE->audio_read_idx >= MOVIE_STATE->audio_write_idx) && (next_index == MOVIE_STATE->video_ring_size))
    {
        next_index = 0;
    }

    MOVIE_STATE->last_consumed_audio_frame = entry->frame_number;
    MOVIE_STATE->audio_read_idx = next_index;
}
