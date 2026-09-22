#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8013B208;
extern s32 D_801ADAF4;
extern s32 D_801B2AD8;
extern s32 D_801B2ADC;
extern s32 func_8006683C(s32 a0);
extern s32 func_800652A8(s32 a0, s32 a1);
extern void func_800931DC(void);

/**
 * @brief Set up a tinted world-map sub-scene and register its handler.
 */
void func_80091E98(void)
{
    D_8013B208 = 1;
    func_8006683C(0x404045);
    D_801ADAF4 = 8;
    func_800652A8(0x28, 0x80);
    func_8006CAC0(func_800931DC);
    D_801B2ADC = 0x1E;
    D_801B2AD8 += 1;
}
