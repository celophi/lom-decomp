#include "common.h"

extern void *D_8013A184;
extern s32 D_80125538;
extern u8 D_800D9268;
extern s32 D_801B2550;
extern s32 D_801B2554;
extern void func_80074044(void);

/** @brief World-map step handler: init substate block and advance. */
void func_80073FD0(void)
{
    u8 *base = &D_800D9268;

    D_8013A184 = &D_80125538;
    *(u8 *)(base + 0x2BDA) = 0xF;
    *(s16 *)(base + 0x2BE4) = -1;
    *(s16 *)(base + 0x2BD6) = 0;
    *(s16 *)(base + 0x2BE2) = 0;
    *(s16 *)(base + 0x2BF6) = 0x81;
    *(s16 *)(base + 0x2BF8) = 0x81;
    D_801B2554 = 0x40;
    D_801B2550 += 1;
    func_80074044();
}
