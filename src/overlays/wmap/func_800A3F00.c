#include "common.h"

extern s32 D_8013B208;
extern s32 D_801ADAF4;
extern s32 D_801B2DD8;
extern s32 D_801B2DDC;
extern s32 func_8006683C(s32 a0);
extern s32 func_800652A8(s32 a0, s32 a1);
extern void func_8006CAC0(void* callback);
extern void func_800A4D64(void);

/**
 * @brief Set up a tinted world-map sub-scene and register its handler.
 */
void func_800A3F00(void)
{
    D_8013B208 = 1;
    func_8006683C(0x702540);
    D_801ADAF4 = 8;
    func_800652A8(0x2B, 0x80);
    func_8006CAC0(func_800A4D64);
    D_801B2DDC = 0x20;
    D_801B2DD8 += 1;
}
