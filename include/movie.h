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
    u_char _pad0[0x98];
    u_char activeDisplayBuffer;
    u_char _pad99;
    u_char _pad9a;
    u_char _pad9b;
    u_char _pad9c;
    u_char frameReady;
    u_char _pad9e;
    u_char unk9f;
} SRC_801ED500;

typedef struct
{
    u8* unk0;
    u8* unk4;
    u8* unk8;
    u8* unkC;
    u8* unk10;
    u8* unk14;
    u8* unk18;
    u8* unk1C;
    RECT rects[3];
    u32 unk38;
    u32 unk3C;
    u8 pad_40[4];
    u32 unk44;
    u32 unk48;
    u32 unk4C;
    u32 unk50;
    u32 unk54;
    u32 unk58[9];
    u8 pad_7C[2];
    u16 unk7E;
    u32 unk80;
    u32 unk84;
    u32 unk88;
    u32 unk8C;
    s8 unk90;
    s8 unk91;
    s8 unk92;
    s8 unk93;
    s8 unk94;
    s8 unk95;
    s8 unk96;
    s8 unk97;
    s8 unk98;
    s8 unk99;
    s8 unk9A;
    s8 unk9B;
    s8 unk9C;
    s8 unk9D;
    s8 unk9E;
    s8 unk9F;
} UnkState;
typedef struct
{
    u8 pad[0x38];
    u32 unk38;
} AllocInfo;

typedef struct
{
    u8 _pad0[0x0C];
    u32 table;
    u32* ptr10[2];
    u32* ptr18[2];
    u8 _pad20[0x34 - 0x20];
    s16 field34;
    s16 field36;
    u8 _pad38[0x48 - 0x38];
    u32 field48;
    u32 field4C;
    u8 _pad50[0x54 - 0x50];
    u32 field54;
    u8 _pad58[0x64 - 0x58];
    u32 field64;
    u32 field68;
    u8 _pad6C[0x70 - 0x6C];
    u32 field70;
    u8 _pad74[0x88 - 0x74];
    u32 field88;
    u32 field8C;
    u8 field90;
    u8 _unk91;
    u8 field92;
    u8 field93;
    u8 field94;
    u8 field95;
    u8 _unk96;
    u8 _unk97;
    u8 _unk98;
    u8 field99;
    u8 _unk9A;
    u8 _unk9B;
    u8 field9C;
    u8 field9D;
    u8 field9E;
    u8 field9F;
} D_801ED500_t;

typedef struct
{
    u8 pad0[0x97];
    u8 unk97;
    u8 pad98[1];
    u8 unk99;
    u8 unk9A;
    u8 pad9B[1];
    u8 unk9C;
} BaseObj;
typedef struct
{
    u8 pad[0x18];
    u_long* unk18;
} SubObj;

typedef struct
{
    u8 _pad0[0x18];
    u32 unk18[2];
    struct
    {
        s16 a;
        u16 b;
        s16 c;
        u16 _pad;
    } ch[2];
    u16 unk30;
    u16 unk32;
    u16 unk34;
    u16 unk36;
    u8 _pad1[0x97 - 0x38];
    u8 unk97;
    u8 unk98;
    u8 unk99;
    u8 _pad_9a;
    u8 unk9B;
    u8 unk9C;
    u8 unk9D;
    u8 _pad_9e;
    u8 unk9F;
} Struct_801ED500;

typedef struct
{
    u8 _pad0[0x18];
    u32* ptrArray[7];  /* frame buffer pointers, indexed by activeBufferIdx */
    s16 unk34;         /* frame width  (used to compute DCT word count) */
    s16 unk36;         /* frame height (used to compute DCT word count) */
    u8 _pad1[0x90 - 0x38];
    u8 gpuMode;        /* 0 = DrawSync/LoadImage path; non-zero = BreakDraw/LoadImage2 path */
    u8 _pad2[0x96 - 0x91];
    u8 busy;           /* 1 while a GPU/MDEC operation is in flight */
    u8 drawSyncTarget; /* DrawSync(1) count that must be reached before re-uploading */
    u8 _pad3[0x99 - 0x98];
    u8 activeBufferIdx;   /* which ptrArray slot holds the current decoded frame */
    u8 pendingVramUpload; /* set when a decoded frame is ready to be DMAed into VRAM */
    u8 pendingMdecDecode; /* set when new bitstream data is ready to feed to the MDEC */
} GlobalStruct;

typedef u32 SectorBuffer[8];

typedef struct GlobalData
{
    u32 videoTableBase;     /* 0x00 — base of 32-byte video sector-header table */
    u32 videoDataBase;      /* 0x04 — base of video payload buffer (2016-byte stride) */
    u32 audioDataBase;      /* 0x08 — base of audio payload buffer (2048-byte stride) */
    u8 _pad0C[0x40];
    u32 totalFrames;
    s32 videoRingCapacity;  /* 0x50 — max slots in video ring buffer */
    s32 audioRingCapacity;  /* 0x54 — max slots in audio ring buffer */
    s32 videoWriteIdx;      /* 0x58 — next slot to write into video ring */
    s32 videoReadIdx;       /* 0x5C — next slot to read from video ring */
    s32 videoWrapSavedIdx;  /* 0x60 — saved videoWriteIdx before wrapping to 0 */
    s32 audioWriteIdx;      /* 0x64 — next slot to write into audio ring */
    s32 audioReadIdx;       /* 0x68 — next slot to read from audio ring */
    s32 audioWrapSavedIdx;  /* 0x6C — saved audioWriteIdx before wrapping to 0 */
    u8 _pad70[4];
    u32 frameNumber;        /* 0x74 — frame number of sector currently being read */
    u32 continuationType;   /* 0x78 — 0=video continuation, non-zero=audio continuation */
    u16 chunkSectorIdx;     /* 0x7C — sector index within current multi-sector frame */
    u16 sectorsRemaining;   /* 0x7E — sectors left to read for the current frame chunk */
    u32 lastVideoFrame;     /* 0x80 — frame number of last video sector written */
    u32 unk84;
    u32 lastAudioFrame;     /* 0x88 — frame number of last audio sector written */
    u32 unk8C;
    u8 _pad90[2];
    u8 unk92;
    u8 _pad93[11];
    u8 endOfStream;         /* 0x9E — set when frameNumber >= totalFrames */
} GlobalData;

typedef struct Entry
{
    u8 _pad0[6];
    u16 sectorCount;
    u8 _pad1[2048 - 8];
} Entry;
typedef struct Global
{
    u8 _pad0[8];
    u8* audioDataBase;      /* base ptr into audio sector table; entries are 2048 bytes each */
    u8 _pad1[0x64 - 0xC];
    s32 audioWriteIdx;      /* ring buffer write head (sectors loaded from CD) */
    s32 audioReadIdx;       /* ring buffer read head (wraps at audioRingSize) */
    s32 audioRingSize;      /* ring buffer capacity / wrap point for read/next indices */
    s32 audioBufferedCount; /* cumulative sector count of entries queued but not yet consumed */
    u8 _pad2[0x88 - 0x74];
    s32 unk88;
    s32 unk8C;
} Global;

typedef struct
{
    s32 videoTableBase;    /* 0x00 */
    s32 videoDataBase;     /* 0x04 */
    u8 _pad0[0x58 - 8];   /* 0x08 … 0x57 */
    s32 videoWriteIdx;     /* 0x58 */
    s32 videoReadIdx;      /* 0x5C */
    s32 ringCapacity;      /* 0x60 */
    u8 _pad1[0x80 - 0x64]; /* 0x64 … 0x7F */
    s32 unk80;             /* 0x80 */
    s32 lastConsumedVideoFrame; /* 0x84 */
} BaseStruct_80141788;

typedef struct
{
    u8 pad[6];
    u16 sectorCount;
    s32 frameNumber;
} InnerStruct;

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 audioDataBase;    /* 0x08 */
    u8 _pad0[0x58 - 0xC];
    s32 unk58;
    s32 unk5C;
    s32 ringCapacity;     /* 0x60 */
    s32 audioWriteIdx;    /* 0x64 */
    s32 audioReadIdx;     /* 0x68 */
    s32 unk6C;
    s32 audioBufferedCount; /* 0x70 */
    u8 _pad1[0x80 - 0x74];
    s32 unk80;
    s32 unk84;
    s32 unk88;
    s32 frameNumber;
} BaseStruct_801418B0;
typedef struct
{
    u8 pad[6];
    u16 sectorCount;
    s32 frameNumber;
} InnerStruct_801418B0;

extern u_char g_cdAudioReady;
extern u8 g_cdStatusByte3;
extern void* D_80180014;
extern u8 D_801ED590;
extern u8 D_801ED596;
extern u8 D_801ED595;
extern u8 D_801ED592;
extern u16 D_801ED57E;
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
extern void func_80140358(s32 a0, s32 a1, s32 a2, s32 a3);
extern void func_801406E4(void);
extern void movie_service_video_ops(void);
extern void func_80023030(s32 arg0);
extern void func_80140C00(void);

#endif