#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8013B208;
extern s32 D_801B2A70;
extern s32 D_801B2A74;
extern void func_8009061C(void);

/**
 * @brief Set up a world-map sub-scene: request assets and register its handler.
 */
void func_8008FF54(void)
{
    D_8013B208 = 1;
    func_800652A8(0x26, 0x80);
    func_8006CAC0(func_8009061C);
    D_801B2A74 = 0x20;
    D_801B2A70 += 1;
}
