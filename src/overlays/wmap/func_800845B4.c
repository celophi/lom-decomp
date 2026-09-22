#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8013B208;
extern s32 D_801ADAF4;
extern s32 D_801B2870;
extern s32 D_801B2874;
extern void func_8006683C(s32 arg);
extern void func_800652A8(s32 a, s32 b);
extern void func_8008489C(void);
extern void func_80084D30(void);

/** @brief World-map state entry: load resources, register callbacks, advance. */
void func_800845B4(void)
{
    D_8013B208 = 1;
    func_8006683C(0x304010);
    D_801ADAF4 = 4;
    func_800652A8(0x1F, 0x80);
    func_8006CAC0(func_8008489C);
    func_8006CAC0(func_80084D30);
    D_801B2874 = 8;
    D_801B2870 += 1;
}
