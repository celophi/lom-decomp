#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8013B208;
extern s32 D_801ADAF4;
extern s32 D_801B2B30;
extern s32 D_801B2B34;
extern s32 func_8006683C(s32 a0);
extern void func_800951B0(void);

/**
 * @brief Set up a tinted world-map sub-scene and register its handler.
 */
void func_800940D0(void)
{
    D_8013B208 = 1;
    func_8006683C(0x701040);
    D_801ADAF4 = 9;
    func_800652A8(0x2A, 0x80);
    func_8006CAC0(func_800951B0);
    D_801B2B34 = 0x14;
    D_801B2B30 += 1;
}
