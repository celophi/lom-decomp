#ifndef _AKAO_SEQUENCER_H
#define _AKAO_SEQUENCER_H

#include "common.h"
#include "akao_driver.h"
#include <libapi.h>

void akao_copy_bytes(s32* source, s32* destination, u32 count);
void akao_apply_cdvol_to_spu(void);
void akao_release_channels(AkaoChannelState* channel, u32 release_mask);
void akao_sfx_release_channels(AkaoChannelState* channel, u32 release_mask);
void akao_channel_set_articulation(AkaoChannelState* channel, s32 articulation_index);

/* Externs not covered by akao_driver.h */
extern s16 g_akao_cdvol_current;
extern s32 g_akao_cdvol_step;
extern s32 g_akao_masterpan_step;
extern s32 g_akao_mastervol_step;

/** @brief Root-counter ticks taken by the last four driver ticks, oldest first. */
typedef struct
{
    s32 samples[4];
} TimingRing;

/** @brief Driver tick timing history kept by akao_irq_handler. */
extern TimingRing g_akao_irq_timing;
extern u8 g_akao_master_vol_scalar;

#endif