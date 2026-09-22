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
extern WmapValueRecord D_80139290[][6];
extern s32 D_801B24C0;
extern s32 D_801B24C4;
extern void func_800723E0(void);

/** @brief Register a callback, update the selected map cell, and begin a 68-tick delay. */
void func_800718CC(void)
{
    func_8006CAC0(&func_800723E0);
    D_80139244 = 0;
    D_801B24C4 = 0x44;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B24C0 += 1;
}
