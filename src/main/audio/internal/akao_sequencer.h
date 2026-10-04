#ifndef _AKAO_SEQUENCER_H
#define _AKAO_SEQUENCER_H

#include "common.h"
#include "akao_driver.h"
#include <libapi.h>

void akao_copy_bytes(s32* src, s32* dst, u32 num_bytes);
void akao_apply_cdvol_to_spu(void);

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