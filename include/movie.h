#ifndef _MOVIE_H
#define _MOVIE_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libpress.h"
#include "psyq/libcd.h"

typedef struct
{
    u_char unk0;
    u_char _pad1;
    u_short unk2;
    u_short unk4;
} SRC_801ED600;

typedef struct
{
    // ---- first 32 bytes: 8 pointers (from first struct) ----
    u8* videoTableBase; // unk0
    u8* videoDataBase;  // unk4
    u8* audioDataBase;  // unk8
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
    s8 unk97;            // first struct: unk97 ; second struct: _unk97
    s8 chunkIdx;         // initial active chunk index (0 or 1) from first struct; second struct calls this _unk98
    u8 outBufIdx;        // which mdecOutputBuf[] receives the next DecDCTout output (0 or 1)
    u8 unk9A;            // 0x9A — pendingVramUpload (read directly by mdec_out_callback / service_video_ops; needs lbu)
    u8 unk9B;            // 0x9B — pendingMdecDecode (read directly by service_video_ops; needs lbu)
    s8 mdecBusy;         // non‑zero while MDEC/DMA operation is in flight
    s8 field9D;          // second struct: field9D ; first struct: unk9D
    u8 endOfStream;      // 0x9E — set when frameNumber >= totalFrames
    u8 endState;         // 1 = near end, 2 = stream fully ended
} CombinedState;

typedef struct
{
    u8 pad[0x38];
    u32 allocBase; /* base address for movie buffer allocations */
} AllocInfo;

typedef struct
{
    u8 pad[0x18];
    u_long* unk18;
} SubObj;

typedef u32 SectorBuffer[8];

typedef struct Entry
{
    u8 _pad0[6];
    u16 sectorCount;
    u8 _pad1[2048 - 8];
} Entry;

typedef struct
{
    u8 pad[6];
    u16 sectorCount;
    s32 frameNumber;
} InnerStruct;

typedef struct
{
    u8 pad[6];
    u16 sectorCount;
    s32 frameNumber;
} InnerStruct_801418B0;

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
extern void func_80023030(s32 arg0);
extern void movie_schedule_next_decode(void);

#endif