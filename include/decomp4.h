#ifndef _DECOMP4_H
#define _DECOMP4_H

#include "common.h"

extern long GetRCnt(unsigned long spec);

extern s16 D_8003EC6A;
extern s32 D_8003EC60;
extern s32 D_8003EC70;
extern short D_8003EC64;
extern s32 D_8003EC68;
extern s32 D_8004F754;
extern s16 D_8003D47C;
extern s32 D_8004F7A0;
extern s16 D_8003EC42;
extern s32 D_8003EC3C;
extern s32 D_8003EC78;
extern s16 D_8003EC40;
extern s32 D_8003EC38;
extern s32 D_8003EC74;
extern u8 D_80049130[];

extern u8 g_akao_xa_tracker[];
extern u8 g_sfx_channels[];

typedef struct
{
    u8 _pad0[8];
    s32 unk8;
} AkaoDriverFlags;

/**
 * @brief AKAO sequencer / SFX channel state block. Used for the song channel
 *        at @c g_akao_seq_channel0, the secondary slot at @c g_akao_seq_channel1, the
 *        backing storage at @c g_akao_seq_channels, and each entry of
 *        @c g_sfx_channels (size 0x118 bytes).
 */
typedef struct AkaoChannelState
{
    u32 flags;       /* 0x00 */
    u32 unk4;        /* 0x04 */
    u8 _pad08[0x0C]; /* 0x08 – 0x13 */
    u32 unk14;       /* 0x14 */
    u32 unk18;       /* 0x18 */
    u32 unk1C;       /* 0x1C */
    u32 unk20;       /* 0x20 (low 16 bits + high 16 bits = unk22) */
    u32 unk24;       /* 0x24 */
    u32 unk28;       /* 0x28 */
    u8 _pad2C[0x1C]; /* 0x2C – 0x47 */
    u32 unk48;       /* 0x48 */
    u32 unk4C;       /* 0x4C */
    u8 _pad50[0x0A]; /* 0x50 – 0x59 */
    s16 unk5A;       /* 0x5A */
    u16 unk5C;       /* 0x5C */
    u16 unk5E;       /* 0x5E */
    u32 unk60; /* 0x60 – 0x63 */
    u16 unk64;       /* 0x64 */
    u16 unk66;       /* 0x66 */
    u16 unk68;       /* 0x68 */
    u16 unk6A;       /* 0x6A */
    u16 unk6C;       /* 0x6C */
    u8 _pad6E[0xAA]; /* 0x6E – 0x117 */
} AkaoChannelState;  /* total = 0x118 */

/**
 * @brief One slot in @c g_sfx_channels (size 0x118). Shares the same overall
 *        size as @ref AkaoChannelState but is accessed through a different
 *        set of fields by the SFX update path, so it is typed separately.
 */
typedef struct
{
    u8 _pad00[0x28];
    u32 field_28;            /* 0x28 — flags; tested against 0x02000000 */
    u8 _pad2C[0x58 - 0x2C];
    u32 field_58;            /* 0x58 — tick counter */
    u8 _pad5C[0x66 - 0x5C];
    u16 field_66;            /* 0x66 — countdown */
    u16 field_68;            /* 0x68 — countdown */
    u8 _pad6A[0x118 - 0x6A];
} SfxChannel;

/** @brief SFX channel control bitfields (size 0x20). */
typedef struct
{
    u32 unk0;         /* 0x00 — active-channel bitmask */
    u8 _pad04[0x04];  /* 0x04 – 0x07 */
    u32 unk8;         /* 0x08 */
    u32 unkC;         /* 0x0C */
    u8 _pad10[0x06];  /* 0x10 – 0x15 */
    u16 unk16;        /* 0x16 — tick step */
    u32 unk18;        /* 0x18 — tick accumulator */
} SfxControl;

/** @brief 4-sample ring buffer of timer deltas used for profiling. */
typedef struct
{
    s32 unk0;  /* 0x00 */
    s32 unk4;  /* 0x04 */
    s32 unk8;  /* 0x08 */
    s32 unkC;  /* 0x0C */
} TimingRing;

extern AkaoChannelState* g_akao_seq_channel1;
extern s32 D_8003EC24;
extern SfxControl D_8004D400;
extern s32 D_8004D408;
extern AkaoChannelState g_akao_seq_channels;
extern TimingRing D_8003D160;

extern s32 D_8003EC44;
extern u8 D_8003EC7A;
extern s32 D_8003EC7C;
extern AkaoChannelState* g_akao_seq_channel0;
extern AkaoDriverFlags g_akao_driver_flags;

#endif