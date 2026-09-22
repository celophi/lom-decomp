/* Partial WMAP decompilation: 95.600000% (gcc280_g0). */
#include "common.h"

/* Best so far: 95.6%. The target computes the packet slot address as
 * base + D_8011D530*40 + D_8011D510*240 with DIRECT byte scaling
 * (a*5<<3, b*15<<4) AND schedules the D_801B2400 counter load across the
 * packet store. A byte-pointer cast gives the direct scaling but creates an
 * alias barrier (counter load can't cross the cast store) -> 35%. The u32[]
 * index form below clears aliasing (correct schedule) but factors the *4 into
 * one extra shift -> 95.6% (1 insn off). Needs a form that yields both. */
extern s32 D_8013B20C;
extern u32 D_80139290[];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2400;

void func_800700B0(void)
{
    D_8013B20C = 0;
    D_80139290[D_8011D530 * 10 + D_8011D510 * 60] = D_8011D4FC | 0x100;
    D_801B2400 += 1;
}
