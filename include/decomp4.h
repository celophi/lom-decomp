#ifndef _DECOMP4_H
#define _DECOMP4_H

#include "common.h"

extern long GetRCnt(unsigned long spec);

extern s16 g_akao_cdvol_current;
extern s32 g_akao_cdvol_step;
extern s32 g_akao_cdvol_tick;
extern short g_akao_cdvol_fade_ticks;
extern s32 g_akao_cdvol_acc;
extern s32 D_8004F754;
extern s16 D_8003D47C;
extern s32 D_8004F7A0;
extern s16 D_8003EC42;
extern s32 D_8003EC3C;
extern s32 D_8003EC78;
extern s16 g_akao_mastervol_fade_ticks;
extern s32 g_akao_mastervol_step;
extern s32 g_akao_mastervol_acc;
extern u8 g_akao_seq_channels[];

extern u8 g_akao_xa_tracker[];
extern u8 g_sfx_channels[];

typedef struct
{
    u32 unk0; /* 0x00 */
    u32 unk4; /* 0x04 */
    u32 unk8; /* 0x08 */
} AkaoDriverFlags;

/**
 * @brief AKAO sequencer / SFX channel state block. Used for the song channel
 *        at @c g_akao_seq_channel0, the secondary slot at @c g_akao_seq_channel1, the
 *        backing storage at @c g_akao_seq_master_state, each entry of the
 *        per-channel array @c g_akao_seq_channels, and each entry of
 *        @c g_sfx_channels (size 0x118 bytes).
 */
typedef struct AkaoChannelState
{
    u32 flags;       /* 0x00 */
    u32 unk4;        /* 0x04 */
    u32 unk8;        /* 0x08 */
    u32 unkC;        /* 0x0C */
    u32 unk10;       /* 0x10 */
    u32 unk14;       /* 0x14 */
    u32 unk18;       /* 0x18 */
    u32 unk1C;       /* 0x1C */
    u32 unk20;       /* 0x20 */
    u32 unk24;       /* 0x24 */
    u32 unk28;       /* 0x28 */
    s32 unk2C;       /* 0x2C */
    s32 unk30;       /* 0x30 */
    u8* unk34;       /* 0x34 */
    u8 _pad38[4];    /* 0x38 - 0x3B */
    u32 unk3C;       /* 0x3C */
    u32 unk40;       /* 0x40 */
    u32 unk44;       /* 0x44 */
    u32 unk48;       /* 0x48 */
    u32 unk4C;       /* 0x4C */
    s32 unk50;       /* 0x50 */
    u8 _pad54[6];    /* 0x54 - 0x59 */
    s16 unk5A;       /* 0x5A */
    u16 unk5C;       /* 0x5C */
    u16 unk5E;       /* 0x5E */
    u32 unk60;       /* 0x60 */
    u16 unk64;       /* 0x64 */
    u16 unk66;       /* 0x66 */
    u16 unk68;       /* 0x68 */
    u16 unk6A;       /* 0x6A */
    u16 unk6C;       /* 0x6C */
    u8 _pad6E[4];    /* 0x6E - 0x71 */
    u16 unk72;       /* 0x72 */
    u16 unk74[11];   /* 0x74 - 0x89 */
    u16 unk8A;       /* 0x8A */
    u16 unk8C;       /* 0x8C */
    u8 _pad8E[6];    /* 0x8E - 0x93 */
    u16 unk94;       /* 0x94 */
    u16 unk96;       /* 0x96 */
    u16 unk98;       /* 0x98 */
    u16 unk9A;       /* 0x9A */
    u16 unk9C;       /* 0x9C */
    u16 unk9E;       /* 0x9E */
    u8 _padA0[2];    /* 0xA0 - 0xA1 */
    u16 unkA2;       /* 0xA2 */
    u16 unkA4;       /* 0xA4 */
    u8 _padA6[2];    /* 0xA6 - 0xA7 */
    u16 unkA8;       /* 0xA8 */
    u16 unkAA;       /* 0xAA */
    s16 unkAC;       /* 0xAC */
    u16 unkAE;       /* 0xAE */
    u8 _padB0[6];    /* 0xB0 - 0xB5 */
    u16 unkB6;       /* 0xB6 */
    u16 unkB8;       /* 0xB8 */
    u8 _padBA[2];    /* 0xBA - 0xBB */
    u16 unkBC;       /* 0xBC */
    u16 unkBE;       /* 0xBE */
    u8 _padC0[24];   /* 0xC0 - 0xD7 */
    u16 unkD8;       /* 0xD8 */
    u16 unkDA;       /* 0xDA */
    u16 unkDC;       /* 0xDC */
    u16 unkDE;       /* 0xDE */
    u8 _padE0[10];   /* 0xE0 - 0xE9 */
    u16 unkEA;       /* 0xEA */
    s16 unkEC;       /* 0xEC */
    u16 unkEE;       /* 0xEE */
    u16 unkF0;       /* 0xF0 */
    s16 unkF2;       /* 0xF2 */
    u16 unkF4;       /* 0xF4 */
    u16 unkF6;       /* 0xF6 */
    u8 _padF8[4];    /* 0xF8 - 0xFB */
    u32 unkFC;       /* 0xFC */
    s32 unk100;      /* 0x100 */
    s32 unk104;      /* 0x104 */
    s32 unk108;      /* 0x108 */
    u8 _pad10C[2];   /* 0x10C - 0x10D */
    u16 unk10E;      /* 0x10E */
    u16 unk110;      /* 0x110 */
    u8 _pad112[6];   /* 0x112 - 0x117 */
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

/**
 * @brief Shared channel header accessed by func_8002B580.
 *        Offset 0x64 selects seq-channel mode (0) vs SFX-channel mode (non-0).
 *        Offset 0x34 is cleared unconditionally at the end of that function.
 */
typedef struct AkaoSFXState
{
    u8  _pad00[0x34]; /* 0x00 - 0x33 */
    u32 unk34;        /* 0x34 */
    u8  _pad38[0x2C]; /* 0x38 - 0x63 */
    u16 unk64;        /* 0x64 */
} AkaoSFXState;

#include "akao_driver.h"

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
extern SfxControl g_akao_sfx_control;
extern s32 D_8004D408;
extern AkaoChannelState g_akao_seq_master_state;
extern TimingRing D_8003D160;

extern s32 D_8003EC44;
extern u8 g_akao_master_vol_scalar;
extern s32 g_akao_driver_mode_flags;
extern AkaoChannelState* g_akao_seq_channel0;
extern AkaoDriverFlags g_akao_driver_flags;
extern u8 g_akao_articulation_slots[];

#endif