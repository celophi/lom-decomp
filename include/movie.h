#ifndef _MOVIE_H
#define _MOVIE_H

#include "common.h"
#include "pad.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libpress.h"
#include "psyq/libcd.h"

/* The block at 0x801ED600 is the merged-controller SCDRegs (see pad.h).
 * Skip-cinematic checks read SCDRegs.deviceState (port active),
 * SCDRegs.buttonData (raw merged buttons), and SCDRegs.unk4 (a derived button
 * word; see SCDRegs comment in pad.h). */

/*
 * TODO(investigate): MovieState aliases the AudioSystem block defined in cd.h
 * (both at 0x801ED500). The CD subsystem uses this block to save the DecDCT
 * and DrawSync callbacks that were active before XA audio playback began so
 * `cdrom_reset` can restore them. Movie playback temporarily re-uses the same
 * memory as scratch, which is why `decDCToutCallback` / `drawSyncCallback`
 * (offsets 0x38 / 0x3C) here hold the *previous* handlers — `movie_init`
 * captures them via `DecDCToutCallback(&movie_mdec_out_callback, ...)` etc.
 * Reconcile these two views (union, or pick one canonical struct) once the
 * full audio-system layout is understood.
 */
typedef struct
{
    // ---- first 32 bytes: 8 pointers (from first struct) ----
    struct VideoSectorEntry* video_table_base; // unk0 - array of 32-byte video sector headers
    struct VideoVlcPayload* video_data_base;   // unk4 - parallel array of 2016-byte VLC payloads
    struct AudioSector* audio_data_base;       // unk8 - array of 2048-byte CD sectors (header + payload)
    void* vlc_table;     // unkC / VLC decode table; opaque to MovieState consumers
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
    u32 resource_index;       // CD resource index
    u32 current_frame;        // current frame counter (starts at 0)
    u32 total_frames;         // total frames in movie stream
    s32 video_ring_capacity;  // 0x50 - video ring buffer capacity
    s32 audio_ring_capacity;  // 0x54 - audio ring buffer capacity

    // ---- ring buffer indices ----
    s32 video_write_idx;       // 0x58 - next slot to write into video ring
    s32 video_read_idx;        // 0x5C - next slot to read from video ring
    s32 video_ring_size;       // 0x60 - video ring wrap point (set to old write_idx on ring wrap)
    s32 audio_write_idx;       // 0x64
    s32 audio_read_idx;        // 0x68
    s32 audio_ring_size;       // 0x6C - audio ring wrap point
    u32 audio_buffered_count;  // 0x70 - cumulative sector count queued but not yet consumed
    u32 frame_number;          // 0x74 - frame number of sector currently being read
    u32 continuation_type;     // 0x78 - 0=video continuation, non-zero=audio continuation

    // ---- chunk-sector tracking (offsets 0x7C..0x7F) ----
    u16 chunk_sector_idx;          // 0x7C - sector index within current multi-sector frame
    u16 sectors_remaining;         // 0x7E - sectors left to read for the current frame chunk
    u32 last_video_frame;          // 0x80 - frame number of last video sector written
    u32 last_consumed_video_frame; // 0x84

    // ---- audio frame tracking (both structs) ----
    u32 last_audio_frame;          // offset 136..139
    u32 last_consumed_audio_frame; // offset 140..143

    // ---- status bytes (offsets 144..159) ----
    u8 gpu_mode;           // 0 = DrawSync/LoadImage, non-zero = BreakDraw/LoadImage2 path
    s8 interlace_mode;     // misnomer: actually selects audio source. 1 = AKAO XA streaming
                           // (akao_cmd_e8 + cd_volume); 0 = SPU/synth path (akao_cmd_c8 + panning).
                           // Set from bit 7 of movie_init's `flags` arg.
    u8 unk92;              // 0x92 (was field92 - flipped s8->u8; only ever stored)
    u8 input_buf_idx;      // which vlc_input_buf[] holds current VLC-decoded input (toggled each frame)
    s8 vlc_retry_count;    // countdown: retry DecDCTvlc2 this many vsync ticks
    s8 mdec_retry_pending; // MDEC decode ready but busy; retry on next tick
    s8 busy;               // non-zero while DMA/GPU operation is in flight
    s8 draw_sync_target;   // 0x97 - DrawSync target value (set by mdec_out_callback / service_video_ops)
    s8 chunk_idx;          // initial active chunk index (0 or 1)
    u8 out_buf_idx;        // which mdec_output_buf[] receives the next DecDCTout output (0 or 1)
    u8 pending_vram_upload; // 0x9A - decoded frame is ready, needs LoadImage to VRAM
    u8 pending_mdec_decode; // 0x9B - bitstream staged, needs DecDCTout kicked
    s8 mdec_busy;          // non-zero while MDEC/DMA operation is in flight
    s8 frame_ready;        // 0x9D - set by movie_schedule_next_decode when a chunk boundary is reached; consumed by movie_play
    u8 end_of_stream;      // 0x9E - set when frame_number >= total_frames
    u8 end_state;          // 1 = near end, 2 = stream fully ended (END_STATE_*)
} MovieState;

/* MovieState::endState sentinel values. Token-equivalent to the literals
 * they replace, so codegen is unchanged. */
#define END_STATE_RUNNING  0
#define END_STATE_NEAR_END 1
#define END_STATE_DONE     2

/* Skip-cinematic gating used by movie_play. */
#define MOVIE_SKIPPABLE_MAX  2       /* only movies with idx < this are skippable */
#define MOVIE0_SKIP_MASK     0xFF0F  /* movie 0 (intro/logo): broad — any non-bit-4..7 button */
#define MOVIE1_SKIP_MASK     0x400A  /* movie 1: narrow specific combination */
#define SCD_DEVICE_STATE_OK  3       /* deviceState < this means controller is usable */

/* Audio fade-out ramp during a skip-triggered exit.
 *   armed by setting audioFadeVol = AUDIO_FADE_INITIAL,
 *   stepped down by AUDIO_FADE_STEP each outer-loop iteration,
 *   exits the loop when it reaches 0. */
#define AUDIO_FADE_DISARMED  (-1)
#define AUDIO_FADE_INITIAL   0x70
#define AUDIO_FADE_STEP      0x10

/* Movie playback control block lives at a fixed RAM address.
 * Macro is token-equivalent to the cast so codegen is unchanged.
 * Use the (volatile MovieState*) cast directly when a one-shot volatile
 * access is required — wrapping that in this macro would silently drop the
 * volatile qualifier. */
#define MOVIE_STATE ((MovieState*)0x801ED500)
#define VOL_MOVIE_STATE ((volatile MovieState*)0x801ED500)


typedef struct
{
    u8 pad[0x38];
    u32 alloc_base; /* base address for movie buffer allocations */
} AllocInfo;

typedef u32 SectorBuffer[8];

/* Header layout for a sector-table entry: 6 bytes preamble, then sector count
 * and the source frame number. Used for both video (video_table_base, 32-byte
 * stride) and audio (audio_data_base, 2048-byte stride) ring entries. */
typedef struct
{
    u8 pad[6];
    u16 sector_count;
    s32 frame_number;
} SectorEntry;

/* One PSX CD sector (2048 bytes) of audio ring data:
 *   - bytes 0x00..0x0B: SectorEntry header (sector_count, frame_number).
 *   - bytes 0x0C..0x1F: remaining 20 bytes of the CD-XA sector header
 *     (copied verbatim from the CD by movie_cd_sector_callback).
 *   - bytes 0x20..0x7FF: XA audio payload (2016 bytes). */
typedef struct AudioSector
{
    SectorEntry header;             /* 12 bytes */
    u8 _hdr_remainder[0x20 - 12];   /* 20 bytes - rest of the 32-byte CD header */
    u8 payload[2048 - 0x20];        /* 2016 bytes XA */
} AudioSector;

/* One video-ring table entry: 32 bytes (the full sector header copied as
 * 8 u32 words by movie_cd_sector_callback). The first 12 bytes are the
 * SectorEntry header; the remaining 20 bytes hold sector metadata. The
 * actual VLC payload lives in a parallel buffer (video_data_base, 2016-byte
 * stride). */
typedef struct VideoSectorEntry
{
    SectorEntry header;
    u8 _rest[32 - 12];
} VideoSectorEntry;

/* One slot of the video VLC payload buffer: 2016 bytes of raw bitstream
 * data. video_data_base is a parallel array of these, indexed by the same
 * read/write indices as video_table_base. */
typedef struct VideoVlcPayload
{
    u8 data[2016];
} VideoVlcPayload;

extern u_char g_cdAudioReady;
extern u8 g_cdStatusByte3;
extern void* g_allocInfo;      /* pointer to AllocInfo block; unk38 is the buffer base address */
extern u8 g_gpuMode;           /* 0=DrawSync/LoadImage path; non-zero=BreakDraw/LoadImage2 path (at 0x801ED590) */
extern u8 g_busy;              /* non-zero while a DMA/GPU operation is in flight (at 0x801ED596) */
extern u8 g_mdecRetryPending;  /* MDEC decode ready but MDEC was busy; retry on next tick (at 0x801ED595) */
extern u8 g_audioStreamState;  /* CD audio state: 0=idle, 1=sector arrived, 2=pipeline primed (at 0x801ED592) */
extern u16 g_sectorsRemaining; /* sectors left to read for the current multi-sector frame (at 0x801ED57E) */

/* Public entry point: the only symbol exported from MOVIE.BIN to the rest of
 * the game. All other movie_* functions are static to movie.c. */
extern void movie_play(s32 movie_index);

extern void cdrom_process_state(void);
extern void cdrom_verify_recovery(void);
extern s32 cdrom_get_error_status(void);
extern void cdrom_reset(void);
extern void func_800157DC(void);
extern void func_800157B0(u_long arg0);
extern void func_800158E0(void);
/* AKAO XA-streaming helpers (see config/symbols/shared_symbol_addrs.txt). */
extern void akao_cmd_c8(u32 arg0);                       /* AKAO cmd 0xC8 (raw param) */
extern void akao_xa_setup_panning(u32 sample_rate);      /* writes panning/sample-rate table */
extern void akao_cmd_e8_start_xa_stream(u32 addr, u32 len_bytes); /* AKAO cmd 0xE8 */
extern void akao_cmd_e4_set_cd_volume(s32 vol);          /* AKAO cmd 0xE4 (vol & 0x7F << 8) */
extern void akao_xa_advance_frame(void);                 /* increments audio frame counters */
extern s32 akao_xa_get_position(void);                   /* returns SPU/XA position */

#endif