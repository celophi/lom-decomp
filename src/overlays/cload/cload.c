#include "common.h"

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

extern s32 D_80122988;
extern s32 D_80146918;
extern s32 D_8014A920;
extern u8 D_8014A988[];
extern s32 D_8015A310;
extern s32 D_8015A320;
extern s32 D_8015A328;
extern u8 D_8015A350[];
extern s32 D_80162350;
extern s32 D_80162358;
extern s32 D_80162364;
extern s32 D_80162368;
extern s32 D_80162370;

/**
 * @see decomp.me (100%) TODO
 */
s32 func_801400C4(void)
{
    RECT rect;

    D_80162350 = 0xFF;
    D_8014A920 = 0;
    func_80143DE4();
    func_80143324();
    func_8014033C();
    D_80162358 = 0;
    func_8014485C();
    D_8015A328 = 0;
    rect.x = 0x140;
    rect.y = 0;
    rect.w = 0x40;
    rect.h = 0x100;
    func_8001990C(&rect, 0, 0, 0);
    func_80145CD8();
    D_80162370 = 0;
    D_8015A320 = 0;
    D_80162364 = 0;
    D_8015A310 = 0;
    D_80162368 = 0;
    D_80146918 = 0;
    func_800AA02C();
    func_801404D4();
    func_8014019C();
    return D_80162358;
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8014019C(void)
{
    RECT rect;
    u8 *p;
    u8 *q;
    s32 flag;
    s32 temp;

    func_80019788(0);
    func_8002054C(0);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0x140;
    rect.h = 0x1D8;
    func_8001990C(&rect, 0, 0, 0);
    p = D_8014A988;
    flag = 0;
    func_80019C74(p + 0x40, 0x1000);
    func_80019C74(p + 0x7D04, 0x1000);
    func_80019FB8(p + 0x4040);
    func_800157DC();
    func_800196F0(1);
    do
    {
        q = p + 0x40;
        func_80019C74(q, 0x1000);
        *(u8 **)(p + 0x40B8) = D_8015A350 + (flag << 14);
        func_800A9E78();
        temp = D_80122988 & 0xF000;
        if (temp != 0)
        {
            D_80122988 = temp;
        }
        func_80067BBC(p);
        if (func_80140448(p) != 0)
        {
            break;
        }
        func_80019788(0);
        func_800157B0(2);
        func_8002054C(2);
        func_8001990C(p + 0x40B0, 0, 0, 0);
        flag = 0;
        if (p == D_8014A988)
        {
            p += 0x7CC4;
            flag = 1;
        }
        else
        {
            p = D_8014A988;
        }
        func_80019FB8(p + 0x4040);
        func_80019DEC(p + 0x4054);
        func_80019D7C(q + 0x3FFC);
        func_800157DC();
        func_800122C0();
    } while (1);
    func_800158E0();
    func_8002054C(0);
}
