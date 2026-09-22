#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8013B208;
extern s32 D_801B28B8;
extern s32 D_801B28BC;
extern void func_80086354(void);

/**
 * @brief Set up a world-map sub-scene: request assets and register its handler.
 */
void func_80085CA8(void)
{
    D_8013B208 = 1;
    func_800652A8(0x20, 0x80);
    func_8006CAC0(func_80086354);
    D_801B28BC = 8;
    D_801B28B8 += 1;
}
