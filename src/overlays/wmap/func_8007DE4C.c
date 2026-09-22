#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8013B208;
extern s32 D_801ADAF4;
extern s32 D_801B2748;
extern s32 D_801B274C;
extern void func_8007E2DC(void);

/**
 * @brief Set up a tinted world-map sub-scene and register its handler.
 */
void func_8007DE4C(void)
{
    D_8013B208 = 1;
    func_8006683C(0x102045);
    D_801ADAF4 = 4;
    func_800652A8(0x1C, 0x80);
    func_8006CAC0(func_8007E2DC);
    D_801B274C = 0x18;
    D_801B2748 += 1;
}
