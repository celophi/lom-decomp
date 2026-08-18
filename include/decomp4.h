#ifndef _DECOMP4_H
#define _DECOMP4_H

#include "common.h"
#include "akao_driver.h" /* provides AkaoChannelState, AkaoDriverFlags, SfxControl and most externs */

extern long GetRCnt(unsigned long spec);

/* Externs not covered by akao_driver.h */
extern s16 g_akao_cdvol_current;
extern s32 g_akao_cdvol_step;
extern s32 D_8004F754[];
extern s16 D_8003D47C;
extern s32 g_akao_xa_pan_current[];
extern s32 g_akao_masterpan_step;
extern s32 g_akao_mastervol_step;

/** @brief 4-sample ring buffer of timer deltas used for profiling. */
typedef struct
{
    s32 unk0;  /* 0x00 */
    s32 unk4;  /* 0x04 */
    s32 unk8;  /* 0x08 */
    s32 unkC;  /* 0x0C */
} TimingRing;

extern s32 D_8004D408[1];
extern TimingRing D_8003D160;
extern u8 g_akao_master_vol_scalar;

#endif