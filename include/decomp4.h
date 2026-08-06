#ifndef _DECOMP4_H
#define _DECOMP4_H

#include "common.h"
#include "akao_driver.h" /* provides AkaoChannelState, AkaoDriverFlags, SfxControl and most externs */

extern long GetRCnt(unsigned long spec);

/* Externs not covered by akao_driver.h */
extern s16 g_akao_cdvol_current;
extern s32 g_akao_cdvol_step;
extern s32 D_8004F754;
extern s16 D_8003D47C;
extern s32 g_akao_xa_pan_current;
extern s32 g_akao_masterpan_step;
extern s32 g_akao_mastervol_step;

/**
 * @brief One slot in @c g_sfx_channels (size 0x118). Shares the same overall
 *        size as @ref AkaoChannelState but is accessed through a different
 *        set of fields by the SFX update path, so it is typed separately.
 */
typedef struct
{
    u8 _pad00[0x28];
    u32 field_28;            /* 0x28 - flags; tested against 0x02000000 */
    u8 _pad2C[0x58 - 0x2C];
    u32 field_58;            /* 0x58 - tick counter */
    u8 _pad5C[0x66 - 0x5C];
    u16 field_66;            /* 0x66 - countdown */
    u16 field_68;            /* 0x68 - countdown */
    u8 _pad6A[0x118 - 0x6A];
} SfxChannel;

/**
 * @brief Shared channel header accessed by akao_release_channels.
 *        @c is_sfx_channel selects seq-channel mode (0) vs SFX-channel mode
 *        (non-0). @c flags is cleared unconditionally at the end of that
 *        function.
 */
typedef struct AkaoSFXState
{
    u8  _pad00[0x34]; /* 0x00 - 0x33 */
    u32 flags;        /* 0x34 - same flag word as AkaoChannelState::flags */
    u8  _pad38[0x2C]; /* 0x38 - 0x63 */
    u16 is_sfx_channel; /* 0x64 - same field as AkaoChannelState::is_sfx_channel */
} AkaoSFXState;

/** @brief 4-sample ring buffer of timer deltas used for profiling. */
typedef struct
{
    s32 unk0;  /* 0x00 */
    s32 unk4;  /* 0x04 */
    s32 unk8;  /* 0x08 */
    s32 unkC;  /* 0x0C */
} TimingRing;

extern s32 D_8004D408;
extern TimingRing D_8003D160;
extern u8 g_akao_master_vol_scalar;

#endif