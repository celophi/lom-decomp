#include "common.h"

extern void func_800652A8(s32 a0, s32 a1);
extern void func_8006CAC0(void (*fn)(void));
extern void func_80075704(void);
extern s32 D_801B2570;
extern s32 D_801B2574;

/** @brief World-map step handler: register a callback and advance the step counter. */
void func_80075274(void)
{
    func_800652A8(0x10, 0x80);
    func_8006CAC0(func_80075704);
    D_801B2574 = 8;
    D_801B2570 += 1;
}
