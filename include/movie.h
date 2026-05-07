#ifndef _MOVIE_H
#define _MOVIE_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libpress.h"
#include "psyq/libcd.h"

/*
 * TODO(investigate): SRC_801ED600 — system-wide flag block at 0x801ED600.
 * Used by movie_play to gate playback (`unk0 < 3 && (unk2 & 0xFF0F)`),
 * also referenced as `D_801ED600` in checkps.h and title.h. Likely save-data
 * flags or region/mode bits. The bitmasks 0x400A and 0xFF0F applied to unk4
 * suggest packed event/cinema-flag groups. Decompile its writers to learn
 * the field semantics.
 */
typedef struct
{
    u_char unk0;
    u_char _pad1;
    u_short unk2;
    u_short unk4;
} SRC_801ED600;

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
    u8* videoTableBase;            // unk0
    u8* videoDataBase;             // unk4
    struct AudioSector* audioDataBase; // unk8 — array of 2048-byte CD sectors (header + payload)
    u32 vlcTable;       // unkC / table
    u32* vlcInputBuf[2];
    u32* mdecOutputBuf[2]; // unk18 / ptr18[0]

    // ---- VRAM destination rectangles ----
    // rects[0] : frame A, rects[1] : frame B, rects[2] : decode rect
    RECT rects[3]; // offsets 32..55
    // The third rectangle's width/height also serve as frame dimensions:
    // s16 frameWidth  = rects[2].w;   (offset 52)
    // s16 frameHeight = rects[2].h;   (offset 54)

    // ---- callback handles ----
    u32 decDCToutCallback; // unk38
    u32 drawSyncCallback;  // unk3C

    u8 pad_40[4]; // unused padding

    // ---- stream metadata ----
    u32 resourceIndex;     // CD resource index
    u32 currentFrame;      // current frame counter (starts at 0)
    u32 totalFrames;       // total frames in movie stream
    s32 videoRingCapacity; // 0x50 — video ring buffer capacity
    s32 audioRingCapacity; // 0x54 — audio ring buffer capacity

    // ---- ring buffer indices ----
    s32 videoWriteIdx;      // 0x58 — next slot to write into video ring
    s32 videoReadIdx;       // 0x5C — next slot to read from video ring
    s32 videoRingSize;      // 0x60 — video ring wrap point (set to old writeIdx on ring wrap)
    s32 audioWriteIdx;      // 0x64
    s32 audioReadIdx;       // 0x68
    s32 audioRingSize;      // 0x6C — audio ring wrap point
    u32 audioBufferedCount; // 0x70 — cumulative sector count queued but not yet consumed
    u32 frameNumber;        // 0x74 — frame number of sector currently being read
    u32 continuationType;   // 0x78 — 0=video continuation, non-zero=audio continuation

    // ---- chunk-sector tracking (offsets 0x7C..0x7F) ----
    u16 chunkSectorIdx;         // 0x7C — sector index within current multi-sector frame
    u16 sectorsRemaining;       // 0x7E — sectors left to read for the current frame chunk
    u32 lastVideoFrame;         // 0x80 — frame number of last video sector written
    u32 lastConsumedVideoFrame; // 0x84

    // ---- audio frame tracking (both structs) ----
    u32 lastAudioFrame;         // offset 136..139
    u32 lastConsumedAudioFrame; // offset 140..143

    // ---- status bytes (offsets 144..159) ----
    u8 gpuMode;          // 0 = DrawSync/LoadImage, non‑zero = BreakDraw/LoadImage2 path
    s8 interlaceMode;    // 1 if interlaced mode (from first struct; second struct calls this _unk91)
    u8 unk92;            // 0x92 (was field92 — flipped s8→u8; only ever stored)
    u8 inputBufIdx;      // which vlcInputBuf[] holds current VLC-decoded input (toggled each frame)
    s8 vlcRetryCount;    // countdown: retry DecDCTvlc2 this many vsync ticks
    s8 mdecRetryPending; // MDEC decode ready but busy; retry on next tick
    s8 busy;             // non‑zero while DMA/GPU operation is in flight
    s8 draw_sync_target; // 0x97 — DrawSync target value (set by mdec_out_callback / service_video_ops)
    s8 chunkIdx;         // initial active chunk index (0 or 1)
    u8 outBufIdx;        // which mdecOutputBuf[] receives the next DecDCTout output (0 or 1)
    u8 pending_vram_upload; // 0x9A — decoded frame is ready, needs LoadImage to VRAM
    u8 pending_mdec_decode; // 0x9B — bitstream staged, needs DecDCTout kicked
    s8 mdecBusy;         // non‑zero while MDEC/DMA operation is in flight
    s8 frame_ready;      // 0x9D — set by movie_schedule_next_decode when a chunk boundary is reached; consumed by movie_play
    u8 endOfStream;      // 0x9E — set when frameNumber >= totalFrames
    u8 endState;         // 1 = near end, 2 = stream fully ended
} MovieState;

/* Movie playback control block lives at a fixed RAM address.
 * Macro is token-equivalent to the cast so codegen is unchanged.
 * Use the (volatile MovieState*) cast directly when a one-shot volatile
 * access is required — wrapping that in this macro would silently drop the
 * volatile qualifier. */
#define MOVIE_STATE ((MovieState*)0x801ED500)


typedef struct
{
    u8 pad[0x38];
    u32 allocBase; /* base address for movie buffer allocations */
} AllocInfo;

typedef u32 SectorBuffer[8];

typedef struct Entry
{
    u8 _pad0[6];
    u16 sectorCount;
    u8 _pad1[2048 - 8];
} Entry;

/* Header layout for a sector-table entry: 6 bytes preamble, then sector count
 * and the source frame number. Used for both video (videoTableBase, 32-byte
 * stride) and audio (audioDataBase, 2048-byte stride) ring entries. */
typedef struct
{
    u8 pad[6];
    u16 sectorCount;
    s32 frameNumber;
} SectorEntry;

/* One PSX CD sector (2048 bytes) of audio ring data: a SectorEntry header
 * followed by filler bytes. The actual XA payload is written by
 * movie_cd_sector_callback at offset 0x20 within the sector. */
typedef struct AudioSector
{
    SectorEntry header;
    u8 _rest[2048 - 12]; /* 12 == sizeof(SectorEntry) */
} AudioSector;

extern u_char g_cdAudioReady;
extern u8 g_cdStatusByte3;
extern void* g_allocInfo;      /* pointer to AllocInfo block; unk38 is the buffer base address */
extern u8 g_gpuMode;           /* 0=DrawSync/LoadImage path; non-zero=BreakDraw/LoadImage2 path (at 0x801ED590) */
extern u8 g_busy;              /* non-zero while a DMA/GPU operation is in flight (at 0x801ED596) */
extern u8 g_mdecRetryPending;  /* MDEC decode ready but MDEC was busy; retry on next tick (at 0x801ED595) */
extern u8 g_audioStreamState;  /* CD audio state: 0=idle, 1=sector arrived, 2=pipeline primed (at 0x801ED592) */
extern u16 g_sectorsRemaining; /* sectors left to read for the current multi-sector frame (at 0x801ED57E) */
extern void movie_mdec_out_callback(void);
extern void movie_draw_sync_callback(void);
extern s32 movie_cd_sector_callback(void);

extern void cdrom_process_state(void);
extern void cdrom_verify_recovery(void);
extern s32 cdrom_get_error_status(void);
extern void cdrom_reset(void);
extern void func_800157DC(void);
extern void func_800157B0(u_long arg0);
extern void func_800158E0(void);
extern void movie_init(s32 resourceIndex, s32 flags, s32 totalFrames, s32 initBufferIdx);
extern void movie_update(void);
extern void movie_service_video_ops(void);
/* AKAO XA-streaming helpers (see config/symbols/shared_symbol_addrs.txt). */
extern void akao_cmd_c8(u32 arg0);                       /* AKAO cmd 0xC8 (raw param) */
extern void akao_xa_setup_panning(u32 sampleRate);       /* writes panning/sample-rate table */
extern void akao_cmd_e8_start_xa_stream(u32 addr, u32 lenBytes); /* AKAO cmd 0xE8 */
extern void akao_cmd_e4_set_cd_volume(s32 vol);          /* AKAO cmd 0xE4 (vol & 0x7F << 8) */
extern void akao_xa_advance_frame(u32 frameNum);         /* increments audio frame counters */
extern s32 akao_xa_get_position(void);                   /* returns SPU/XA position */
extern void movie_schedule_next_decode(void);

#endif