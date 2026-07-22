#include "movie.h"
#include "pad.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libpress.h"
#include "psyq/libcd.h"

/* The block at 0x801ED600 is the merged-controller SCDRegs (see pad.h).
 * Skip-cinematic checks read the merged controller type, held buttons, and
 * newly pressed buttons from SCDRegs. */

/**
 * @brief Movie playback control block; lives at fixed RAM address 0x801ED500.
 *
 * Aliases the AudioSystem block defined in cd.h. The CD subsystem uses that
 * block to save the DecDCT and DrawSync callbacks that were active before XA
 * audio playback began so `cdrom_reset` can restore them. Movie playback
 * temporarily re-uses the same memory as scratch, which is why the
 * `dec_dct_out_callback` / `draw_sync_callback` fields (offsets 0x38 / 0x3C)
 * here hold the *previous* handlers - `movie_init` captures them via
 * `DecDCToutCallback(&movie_mdec_out_callback)` etc.
 */
typedef struct
{
    // ---- first 32 bytes: 8 pointers (from first struct) ----
    struct VideoSectorEntry* video_table_base; // unk0 - array of 32-byte video sector headers
    struct VideoVlcPayload* video_data_base;   // unk4 - parallel array of 2016-byte VLC payloads
    struct AudioSector* audio_data_base;       // unk8 - array of 2048-byte CD sectors (header + payload)
    void* vlc_table;                           // unkC / VLC decode table; opaque to MovieState consumers
    void* vlc_input_buf[2];
    u_long* mdec_output_buf[2]; // unk18 - MDEC output buffers (consumed by LoadImage/DecDCTout)

    // ---- VRAM destination rectangles ----
    // rects[0] : frame A, rects[1] : frame B, rects[2] : decode rect
    RECT rects[3]; // offsets 32..55
    // The third rectangle's width/height also serve as frame dimensions:
    // s16 frame_width  = rects[2].w;   (offset 52)
    // s16 frame_height = rects[2].h;   (offset 54)

    // ---- callback handles ----
    u32 dec_dct_out_callback; // unk38
    u32 draw_sync_callback;   // unk3C

    u8 pad_40[4]; // unused padding

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

    // ---- status bytes (offsets 144..159) ----
    u8 gpu_mode;            // 0 = DrawSync/LoadImage, non-zero = BreakDraw/LoadImage2 path
    s8 interlace_mode;      // misnomer: actually selects audio source. 1 = AKAO XA streaming
                            // (akao_cmd_e8 + cd_volume); 0 = SPU/synth path (akao_cmd_c8 + panning).
                            // Set from bit 7 of movie_init's `flags` arg.
    u8 unk92;               // 0x92 (was field92 - flipped s8->u8; only ever stored)
    u8 input_buf_idx;       // which vlc_input_buf[] holds current VLC-decoded input (toggled each frame)
    s8 vlc_retry_count;     // countdown: retry DecDCTvlc2 this many vsync ticks
    s8 mdec_retry_pending;  // MDEC decode ready but busy; retry on next tick
    s8 busy;                // non-zero while DMA/GPU operation is in flight
    s8 draw_sync_target;    // 0x97 - DrawSync target value (set by mdec_out_callback / service_video_ops)
    s8 chunk_idx;           // initial active chunk index (0 or 1)
    u8 out_buf_idx;         // which mdec_output_buf[] receives the next DecDCTout output (0 or 1)
    u8 pending_vram_upload; // 0x9A - decoded frame is ready, needs LoadImage to VRAM
    u8 pending_mdec_decode; // 0x9B - bitstream staged, needs DecDCTout kicked
    s8 mdec_busy;           // non-zero while MDEC/DMA operation is in flight
    s8 frame_ready; // 0x9D - set by movie_schedule_next_decode when a chunk boundary is reached; consumed by movie_play
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

/** @brief CD sector geometry consumed by @ref cd_sector_callback. */
#define CD_HEADER_WORDS         8       /**< 32-byte sector header read with CdGetSector. */
#define CD_PAYLOAD_WORDS        0x1F8   /**< 504 u32 = 2016-byte sector payload. */
#define CD_PAYLOAD_BYTE_OFFSET  0x20    /**< Bytes from sector start to payload (size of header). */

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
    u32 alloc_base;
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
extern u8 g_cdStatusByte3;

extern void* g_allocInfo;      /* pointer to AllocInfo block; unk38 is the buffer base address */
extern u8 g_gpuMode;           /* 0=DrawSync/LoadImage path; non-zero=BreakDraw/LoadImage2 path (at 0x801ED590) */
extern u8 g_busy;              /* non-zero while a DMA/GPU operation is in flight (at 0x801ED596) */
extern u8 g_mdecRetryPending;  /* MDEC decode ready but MDEC was busy; retry on next tick (at 0x801ED595) */
extern u8 g_audioStreamState;  /* CD audio state: 0=idle, 1=sector arrived, 2=pipeline primed (at 0x801ED592) */
extern u16 g_sectorsRemaining; /* sectors left to read for the current multi-sector frame (at 0x801ED57E) */

void cdrom_process_state(void);
void cdrom_verify_recovery(void);
s32 cdrom_get_error_status(void);
void cdrom_reset(void);
void update_controllers(void);
void set_controller_vsync_interval(u_long interval);
void reset_controller_vsync_state(void);

/* AKAO XA-streaming helpers (see config/symbols/shared_symbol_addrs.txt). */
void akao_cmd_c8(u32 arg0);                                /* AKAO cmd 0xC8 (raw param) */
void akao_xa_setup_panning(u32 sample_rate);               /* writes panning/sample-rate table */
void akao_cmd_e8_start_xa_stream(u32 addr, u32 len_bytes); /* AKAO cmd 0xE8 */
void akao_cmd_e4_set_cd_volume(s32 vol);                   /* AKAO cmd 0xE4 (vol & 0x7F << 8) */
void akao_xa_advance_frame(void);                          /* increments audio frame counters */
s32 akao_xa_get_position(void);                            /* returns SPU/XA position */

const s32 g_movieOverlayId = 14;

static void movie_init(s32 resource_index, s32 flags, s32 total_frames, s32 init_buffer_idx);
static void movie_update(void);
static void movie_mdec_out_callback(void);
static void movie_schedule_next_decode(void);
static void movie_service_video_ops(void);
static s32 cd_sector_callback(void);
static s32 get_next_audio_entry(AudioSector** out_entry);
static void draw_sync_callback(void);
static s32 get_next_video_entry(VideoVlcPayload** out_vlc_data, VideoSectorEntry** out_entry_header);
static void advance_audio_read(void);
static void advance_video_read(void);

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
    char end_state_match;
    s32 error_status;
    s32 timeout;
    unsigned short movie_idx_u16;
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
     *     3   | 0x16A3   |  5368   (longest - likely ending)
     *     4   | 0x16A4   |   898   (shortest - also the default fallthrough)
     */

    switch ((u16)(movie_index & 0xFFFF))
    {
    case 0:
        frame_count = 2098;
        init_flags = 0x80;
        break;

    case 1:
        frame_count = 2473;
        init_flags = 0x80;
        break;

    case 2:
        frame_count = 1318;
        init_flags = 0x80;
        break;

    case 3:
        frame_count = 5368;
        init_flags = 0x80;
        break;

    case 4:

    default:
        frame_count = 898;
        init_flags = 0x80;
        break;
    }

    resource_idx = (movie_index & 0xFFFF) + 0x16A0;
    movie_init(resource_idx, init_flags, frame_count, 0);
    VSync(0);
    update_controllers();
    audio_fade_vol = AUDIO_FADE_DISARMED;
    retry_limit = 5;
    state = (MovieState*)0x801ED500;
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
 * @param flags         Bit 0..6 -> MovieState.gpu_mode. Bit 7 -> MovieState.interlace_mode
 *                      (despite the name, this selects the *audio source*, not
 *                      GPU interlacing - see @note below).
 * @param total_frames   Frame-count stop condition; set into MovieState.total_frames.
 * @param init_buffer_idx Initial active chunk index (0 or 1). Path B uses it to
 *                      seed rects[2] from rects[init_buffer_idx].
 *
 * @note `MovieState.gpu_mode` and the `g_gpuMode` global are *the same byte* at
 *       0x801ED590 (offset 0x90 in MovieState). The write at the top of this
 *       function and the `if (g_gpuMode == 0)` branch read the same storage.
 *
 * @note In shipped play, `movie_play` always passes flags = 0x80, so
 *       gpu_mode = 0 (path A taken every time) and interlace_mode = 1 (XA audio).
 *       Path B is dead in production; preserved for matching, which is also why
 *       its read of an uninitialised rects[0].x (`>= 0x300` guard) is inert.
 *
 * @note `interlace_mode` controls audio source, not display interlacing:
 *         1 -> akao_cmd_e8_start_xa_stream + cd_volume   (real CD-XA streaming)
 *         0 -> akao_cmd_c8(0x7FFF) + xa_setup_panning    (SPU/synth path)
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
 * @note Path-B memory map (relative to AllocInfo::alloc_base):
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
static void movie_init(s32 resource_index, s32 flags, s32 total_frames, s32 init_buffer_idx)
{
    AllocInfo* alloc_info = g_allocInfo;
    MovieState* ms;

    MOVIE_STATE->gpu_mode = (s8)(flags & 0x7F);
    if (flags & 0x80)
    {
        MOVIE_STATE->interlace_mode = 1;
    }
    else
    {
        MOVIE_STATE->interlace_mode = 0;
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
        MOVIE_STATE->video_ring_capacity = 0x32;
        MOVIE_STATE->rects[1].h = 0xF0;
        MOVIE_STATE->rects[0].h = 0xF0;
        MOVIE_STATE->rects[2].x = 0;
        MOVIE_STATE->rects[2].y = 0;
        MOVIE_STATE->audio_ring_capacity = 0x10;
        MOVIE_STATE->video_data_base = (void*)0x80147640;
        VOL_MOVIE_STATE->chunk_idx = 0;
    }
    else
    {
        MOVIE_STATE->video_table_base = (VideoSectorEntry*)0x80147000;
        MOVIE_STATE->audio_data_base = (AudioSector*)0x80156000;
        MOVIE_STATE->vlc_table = (void*)alloc_info->alloc_base;
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
        MOVIE_STATE->video_ring_capacity = 0x1E;
        MOVIE_STATE->audio_ring_capacity = 0x10;
        MOVIE_STATE->video_data_base = (VideoVlcPayload*)(0x3C0 + ((u32)MOVIE_STATE->video_table_base));
        VOL_MOVIE_STATE->chunk_idx = (s8)init_buffer_idx;
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
    ms->unk92 = 0;
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
    ms->dec_dct_out_callback = (u32)DecDCToutCallback(&movie_mdec_out_callback);
    ms->draw_sync_callback = DrawSyncCallback(&draw_sync_callback);
    if (ms->interlace_mode != 0)
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
    cdrom_queue_command(0x1B, (s16)resource_index, (void*)0, &cd_sector_callback);

    // reload?
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
static void movie_update(void)
{
    s32 audio_capacity; /* audio_ring_capacity reload */

    VideoVlcPayload* vlc_payload;   /* raw bitstream for the next video frame */
    VideoSectorEntry* entry_header; /* 32-byte sector header for the same frame */
    AudioSector* audio_entry;       /* next audio ring entry (header + payload) */

    s32 tmp = 0; /* vlc-complete flag, then audio_buffered_count compare */
    MovieState* combined = MOVIE_STATE;

    if (g_mdecRetryPending != 0)
    {
        if ((MOVIE_STATE->mdec_busy == 0) && (combined->frame_ready == 0))
        {
            MOVIE_STATE->mdec_busy = 1;
            DecDCTin(MOVIE_STATE->vlc_input_buf[MOVIE_STATE->input_buf_idx], (MOVIE_STATE->gpu_mode & 0xFFFFu) == 0);
            {
                s32 temp = ((s16)MOVIE_STATE->rects[2].w) * ((s16)MOVIE_STATE->rects[2].h);
                s32 word_count = temp + (((unsigned)temp) >> 31);
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
                tmp = 1;
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
                tmp = 1;
                MOVIE_STATE->vlc_retry_count = 0;
            }
        }
        else if ((MOVIE_STATE->end_of_stream != 0) && (MOVIE_STATE->mdec_busy == 0))
        {
            MOVIE_STATE->end_state = END_STATE_DONE;
        }
    }

    if (tmp != 0)
    {
        s32 field_9d_zero_flag;
        advance_video_read();

        if ((MOVIE_STATE->mdec_busy == 0) && (field_9d_zero_flag = MOVIE_STATE->frame_ready == 0))
        {
            MOVIE_STATE->mdec_busy = 1;
            DecDCTin(MOVIE_STATE->vlc_input_buf[MOVIE_STATE->input_buf_idx], MOVIE_STATE->gpu_mode == 0);
            {
                s32 temp = ((s16)MOVIE_STATE->rects[2].w) * ((s16)MOVIE_STATE->rects[2].h);
                field_9d_zero_flag = ((unsigned)temp) >> 31;
                DecDCTout(MOVIE_STATE->mdec_output_buf[MOVIE_STATE->out_buf_idx], (temp + field_9d_zero_flag) >> 1);
            }
        }
        else
        {
            g_mdecRetryPending = 1;
        }
    }

    combined = MOVIE_STATE;
    if (g_cdAudioReady != 0)
    {
        if (get_next_audio_entry((void*)&vlc_payload) != 0)
        {
            entry_header = (VideoSectorEntry*)vlc_payload;
            MOVIE_STATE->current_frame = ((AudioSector*)vlc_payload)->header.frame_number;

            if ((((AudioSector*)vlc_payload)->header.frame_number > MOVIE_STATE->total_frames) &&
                (MOVIE_STATE->end_state < END_STATE_DONE))
            {
                MOVIE_STATE->end_state = END_STATE_DONE;
            }
            akao_xa_advance_frame();
        }
        combined = MOVIE_STATE;
        if (g_audioStreamState == 2)
        {

            audio_capacity = MOVIE_STATE->audio_ring_capacity;

            if ((s32)MOVIE_STATE->audio_buffered_count >= ((s32)(audio_capacity >> 1)))
            {
                akao_cmd_98_9a_9c_9e(3);
                MOVIE_STATE->unk92 = 0;
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
static void movie_mdec_out_callback(void)
{
    volatile MovieState* base = (volatile MovieState*)0x801ED500;
    s32 temp;
    MovieState* bp_high;
    int zero_literal;

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
            base->draw_sync_target = (s8)(temp + 1);
        }
        else
        {
            base->pending_vram_upload = 1U;
        }
        bp_high = (MovieState*)0x801e0000;
    }
    else
    {
        temp = (s32)BreakDraw();
        zero_literal = 0;
        if (temp != (-1))
        {
            LoadImage2(&MOVIE_STATE->rects[2], base->mdec_output_buf[base->out_buf_idx]);
            if (temp != zero_literal)
            {
                DrawOTag((u_long*)temp);
            }
            bp_high = (MovieState*)0x801e0000;
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
static void movie_schedule_next_decode(void)
{
    unsigned short next_out_buf_idx;
    u16 cur_frame_pos;
    u16 frame_step;
    u16 new_frame_pos;
    u16* new_var;
    s32 new_frame_pos_signed;
    s32 chunk_end;
    u32 decode_size;
    int decode_word_count;

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
static void movie_service_video_ops(void)
{
    volatile MovieState* G = (volatile MovieState*)0x801ED500;
    int word_count;
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
 * @return 1 to keep streaming, 0 when the stream has ended or should pause.
 *
 * @see https://decomp.me/scratch/5flHR (100%)
 */
static s32 cd_sector_callback(void)
{
    VideoSectorEntry sector_header;
    s32 audio_read_idx_l;
    s32 has_room;
    s32 has_more_frames;
    u32* audio_base_reload; /* load-bearing dead store; preserves original codegen */
    void* payload_dst;
    SectorEntry* sector_hdr_ptr;
    int audio_write_next;
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

                /* &video_data_base[write_idx]; stride = 8 * 252 = 2016 = sizeof(VideoVlcPayload). */
                sector_ptr = (u8*)MOVIE_STATE->video_data_base + ((8 * MOVIE_STATE->video_write_idx) * 252);
                while (CdGetSector(sector_ptr, CD_PAYLOAD_WORDS) == 0);

                /* Bulk-copy the raw 32-byte CD header as 8 u32 words. */
                hdr_words = (u32*)&sector_header;

                write_index = MOVIE_STATE->video_write_idx;
                /* &video_table_base[write_idx]; stride = 32 = sizeof(VideoSectorEntry). */
                sector_ptr = (u8*)MOVIE_STATE->video_table_base + (write_index << 5);

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

                /* &audio_data_base[write_idx].payload : skip the 32-byte header and write the XA payload. */
                sector_ptr = ((u8*)MOVIE_STATE->audio_data_base + (VOL_MOVIE_STATE->audio_write_idx << 0xB)) +
                             CD_PAYLOAD_BYTE_OFFSET;
                while (CdGetSector(sector_ptr, CD_PAYLOAD_WORDS) == 0);

                hdr_words = (u32*)&sector_header;
                audio_base_reload = (u32*)(*MOVIE_STATE).audio_data_base;
                /* &audio_data_base[write_idx]; stride = 2048 = sizeof(AudioSector). */
                sector_ptr = (u32*)((u8*)MOVIE_STATE->audio_data_base + (VOL_MOVIE_STATE->audio_write_idx << 0xB));
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
                ms->unk92 = 2;
            }
            return 1;
        }
    }
    else if (MOVIE_STATE->continuation_type == CONTINUATION_VIDEO)
    {
        /* &video_table_base[write_idx + chunk_sector_idx]; stride = 32 = sizeof(VideoSectorEntry). */
        sector_hdr_ptr = (SectorEntry*)((u8*)MOVIE_STATE->video_table_base +
                                        ((VOL_MOVIE_STATE->video_write_idx + MOVIE_STATE->chunk_sector_idx) << 5));
        while (CdGetSector(sector_hdr_ptr, CD_HEADER_WORDS) == 0);

        if (((sector_hdr_ptr->sector_type == SECTOR_TYPE_VIDEO) &&
             (sector_hdr_ptr->frame_number == MOVIE_STATE->frame_number)) &&
            (sector_hdr_ptr->chunk_sector_idx == MOVIE_STATE->chunk_sector_idx))
        {
            /* &video_data_base[write_idx + chunk_sector_idx]; stride = 2 * 1008 = 2016 = sizeof(VideoVlcPayload). */
            payload_dst = (u8*)MOVIE_STATE->video_data_base +
                          ((2 * (VOL_MOVIE_STATE->video_write_idx + (MOVIE_STATE->chunk_sector_idx & 0xFFFF))) * 1008);
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
        /* &audio_data_base[write_idx + chunk_sector_idx]; stride = 2048 = sizeof(AudioSector). */
        sector_hdr_ptr = (SectorEntry*)((u8*)MOVIE_STATE->audio_data_base +
                                        ((VOL_MOVIE_STATE->audio_write_idx + MOVIE_STATE->chunk_sector_idx) << 0xB));
        while (CdGetSector(sector_hdr_ptr, CD_HEADER_WORDS) == 0);

        if (((sector_hdr_ptr->sector_type == SECTOR_TYPE_AUDIO) &&
             (sector_hdr_ptr->frame_number == MOVIE_STATE->frame_number)) &&
            (sector_hdr_ptr->chunk_sector_idx == MOVIE_STATE->chunk_sector_idx))
        {
            payload_dst = ((u8*)MOVIE_STATE->audio_data_base +
                           ((VOL_MOVIE_STATE->audio_write_idx + (MOVIE_STATE->chunk_sector_idx & 0xFFFF)) << 0xB)) +
                          CD_PAYLOAD_BYTE_OFFSET;
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
static s32 get_next_audio_entry(AudioSector** out_entry)
{
    s32 next_idx;
    AudioSector* entry;

    /* Return immediately if nothing is available: ring empty and no secondary data pending */
    if ((MOVIE_STATE->audio_write_idx == MOVIE_STATE->audio_read_idx) &&
        (MOVIE_STATE->last_audio_frame == MOVIE_STATE->last_consumed_audio_frame))
    {
        return 0;
    }

    /* Wrap read_idx back to 0 when it reaches the end of the ring */
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

    /* Look past already-buffered entries to find the next one to queue */
    next_idx = MOVIE_STATE->audio_read_idx + MOVIE_STATE->audio_buffered_count;

    /* Wrap next_idx if it overflows the ring */
    if ((MOVIE_STATE->audio_read_idx >= MOVIE_STATE->audio_write_idx) && (next_idx >= VOL_MOVIE_STATE->audio_ring_size))
    {
        next_idx -= MOVIE_STATE->audio_ring_size;
    }

    /* All loaded entries are already queued; nothing new to dispatch */
    if ((next_idx == MOVIE_STATE->audio_write_idx) && (MOVIE_STATE->audio_buffered_count != 0))
    {
        return 0;
    }

    /* Resolve entry: each entry occupies one 2048-byte CD sector in the audio data buffer */
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
static void draw_sync_callback(void)
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
static s32 get_next_video_entry(VideoVlcPayload** out_vlc_data, VideoSectorEntry** out_entry_header)
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
static void advance_video_read(void)
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
static void advance_audio_read(void)
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
