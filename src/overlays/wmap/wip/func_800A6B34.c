#include "common.h"

extern void func_8006D0F0(s32 a0, s32* a1, s32* a2);
extern void func_8005EB68(s32 start_x, s32 start_y, s32 end_x, s32 end_y, s32* out_x, s32* out_y);
extern void func_800A7D40(void);
extern u8 D_8019D248[];
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_80182DF8;
extern s32 D_801B2E48;

/**
 * @brief World-map step handler: seed a pathfinding move for the actor, populate its
 *        motion record, and advance the step counter.
 * @note Best match ~93% (gcc280_g0); residual is post-call store scheduling.
 */
void func_800A6B34(void)
{
    u8* base;
    u16 a;
    u16 b;

    func_8006D0F0(0x10, &D_800DCEF8, &D_800DCF00);
    base = D_8019D248;
    func_8005EB68(*(s32*)(base + 0x36C), *(s32*)(base + 0x370), D_800DCEF8, D_800DCF00,
                  (s32*)(base + 0x390), (s32*)(base + 0x410));
    *(s32*)(base + 0x384) = D_800DCEF8;
    *(s32*)(base + 0x388) = D_800DCF00;
    a = *(u16*)(base + 0x394);
    *(s32*)(base + 0x380) = 1;
    *(s32*)(base + 0x38C) = 1;
    D_80182DF8 = 1;
    *(u16*)(base + 0x374) = a;
    *(u16*)(base + 0x37C) = ((s16)a - 1) * 0xA0;
    b = *(u16*)(base + 0x414);
    *(u16*)(base + 0x376) = b;
    *(u16*)(base + 0x37E) = ((s16)b - 1) * 0xA0;
    D_801B2E48 += 1;
    func_800A7D40();
}
