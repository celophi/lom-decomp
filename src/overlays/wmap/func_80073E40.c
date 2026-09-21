#include "common.h"

extern void *D_8013A17C;
extern s32 D_80127538;
extern u8 D_800D9268;
extern s32 D_801B2548;
extern s32 D_801B254C;
extern void func_80073EB4(void);

/** @brief World-map step handler: init substate block and advance. */
void func_80073E40(void)
{
    u8 *base = &D_800D9268;

    D_8013A17C = &D_80127538;
    *(u8 *)(base + 0x2BAE) = 0xF;
    *(s16 *)(base + 0x2BB8) = -1;
    *(s16 *)(base + 0x2BAA) = 0;
    *(s16 *)(base + 0x2BB6) = 0;
    *(s16 *)(base + 0x2BCA) = 0x80;
    *(s16 *)(base + 0x2BCC) = 0;
    D_801B254C = 0x98;
    D_801B2548 += 1;
    func_80073EB4();
}
