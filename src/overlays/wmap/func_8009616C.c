#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8013B208;
extern s32 D_801B2B90;
extern s32 D_801B2B94;
extern void func_80097068(void);

/**
 * @brief Set up a world-map sub-scene: request assets and register its handler.
 */
void func_8009616C(void)
{
    D_8013B208 = 1;
    func_800652A8(0x30, 0x80);
    func_8006CAC0(func_80097068);
    D_801B2B94 = 0x28;
    D_801B2B90 += 1;
}
