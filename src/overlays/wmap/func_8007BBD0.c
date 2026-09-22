#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_80139244;
extern s32 D_801B26E8;
extern s32 D_801B26EC;
extern void func_8007C5F8(void);

/**
 * @brief Set up a world-map sub-scene: request assets and register its handler.
 */
void func_8007BBD0(void)
{
    D_80139244 = 1;
    func_800652A8(0x1B, 0x80);
    func_8006CAC0(func_8007C5F8);
    D_801B26EC = 8;
    D_801B26E8 += 1;
}
