#include "wmap_view_effects.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

extern s32 D_8011D4FC;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern s32 D_80139244;
extern s32 D_801ADAF4;
extern WmapValueRecord D_80139290[][6];
extern s32 D_801B2EF8;
extern s32 D_801B2EFC;
extern void func_800AFC74(void);

/** @brief Register a callback, update the selected map cell, and begin a 80-tick delay. */
void func_800AF4C0(void)
{
    func_8006CAC0(&func_800AFC74);
    D_80139244 = 0;
    D_801ADAF4 = 15;
    func_8006683C(0x606070);
    D_801B2EFC = 0x50;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B2EF8 += 1;
}
