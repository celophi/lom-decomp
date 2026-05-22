#ifndef _AKAO_DRIVER_H
#define _AKAO_DRIVER_H

#include "common.h"
#include "akao.h"

/** @brief AKAO driver state flags (size 0x0C). */
typedef struct
{
    u32 unk0; /* 0x00 */
    u32 unk4; /* 0x04 */
    u32 unk8; /* 0x08 */
} AkaoDriverFlags;

/** @brief SFX channel control bitfields (size 0x28). */
typedef struct
{
    u32 unk0;      /* 0x00 -- active-channel bitmask */
    s32 unk4;      /* 0x04 */
    u32 unk8;      /* 0x08 */
    u32 unkC;      /* 0x0C */
    u32 unk10;     /* 0x10 */
    u8 _pad14[2];  /* 0x14 - 0x15 */
    u16 unk16;     /* 0x16 -- tick step */
    u32 unk18;     /* 0x18 -- tick accumulator */
    u32 unk1C;     /* 0x1C */
    u32 unk20;     /* 0x20 */
    u32 unk24;     /* 0x24 */
} SfxControl;

/**
 * @brief Per-stream state for the AKAO XA/CD audio tracker.
 *
 * Drives an XA-streamed sequence: akao_streaming/XA tick code maintains the
 * frame counters in the 0x20..0x3C region, while akao_tick_fades runs a
 * per-channel pan fade out of the 0x40..0x48 block, writing the result to the
 * SPU voices identified by @c unk10.
 */
typedef struct
{
    u8   pad0[0x08]; /* 0x00 - 0x07 */
    u32  unk8;       /* 0x08 - flags; tested against 0x01000000 */
    s32  unkC;       /* 0x0C - XA stream active flag (gates the pan fade) */
    s32  unk10;      /* 0x10 - base SPU voice index for the streamed pair */
    u8   pad1[0x0C]; /* 0x14 - 0x1F */
    s32  unk20;      /* 0x20 */
    s32  unk24;      /* 0x24 - frame counter */
    s32  unk28;      /* 0x28 */
    u8   pad2[0x08]; /* 0x2C - 0x33 */
    s32  unk34;      /* 0x34 */
    s32  unk38;      /* 0x38 - per-frame index */
    s32  unk3C;      /* 0x3C - frame limit (arg1 >> 12) */
    s32  unk40;      /* 0x40 - pan accumulator (seeded to 0x7F00) */
    s32  unk44;      /* 0x44 - pan step */
    s32  unk48;      /* 0x48 - pan fade-tick countdown */
} AkaoXaTracker; /* size 0x4C */

extern s32 g_akao_spu_xfer_pending;
extern u8 g_akao_articulation_slots[];
extern u8 g_sfx_channels[];
extern s32 g_akao_driver_mode_flags;
extern s32 D_8003EC6C;
extern s32 g_akao_seq_pending_ticks;
extern AkaoXaTracker g_akao_xa_tracker;
extern s16 g_akao_cdvol_fade_ticks;
extern s32 g_akao_masterpan_acc;
extern s16 g_akao_masterpan_fade_ticks;
extern s32 g_akao_mastervol_acc;
extern s16 g_akao_mastervol_fade_ticks;
extern s32 g_akao_cdvol_tick;
extern s32 g_akao_cdvol_acc;
extern s32 g_akao_pending_channels;
extern AkaoChannelState* g_akao_seq_channel1;
extern AkaoChannelState *g_akao_seq_channel0;
extern void *D_8003EC58;
extern u8 D_8004C2D0[];
extern SfxControl g_akao_sfx_control;
extern u8 D_8004F830[];
extern AkaoDriverFlags g_akao_driver_flags;
extern u8 D_8004D388[];
extern u8 D_8003EC30[];
extern u8 g_akao_seq_channels[];
extern AkaoChannelState g_akao_seq_master_state;
extern char g_akao_spu_malloc_table[];
extern char g_akao_spu_zero_primer[];
extern s32 g_akao_rcnt2_event;
extern s32 g_akao_bank_prog_base;
extern s32 g_akao_bank_region_b;
extern s32 g_akao_bank_region_c;

extern s32 akao_check_magic(AkaoSeqHeader *hdr);
extern void func_80028E34(int, volatile short, void *, int);
extern void akao_irq_handler(void);

// Fix the off() helper to accept any pointer type
inline static u8* off(void* p, int o)
{
    return (u8*)p + o;
}


#endif