#include "common.h"

typedef struct {
    union { u16 h; struct { u8 unk0; u8 unk1; } b; } u0;
    u8 pad2[0x268 - 2];
} Entry268;

typedef struct {
    u8 pad0[0x2A];
    s16 unk2A;
    u8 pad2C[0x54 - 0x2C];
} Rec54;

typedef struct {
    u8 pad0[0xC];
    s32 unkC;
    u8 pad10[0x178 - 0x10];
    s32 unk178;
    u8 pad17C[0x23C - 0x17C];
} State23C;

extern s32 D_801227C8;
extern s32 D_80122908;
extern Entry268 D_800FD818[];
extern Rec54 D_800FDF58[];
extern State23C D_80105AE0[];
void func_8006809C(void);

void func_800A7384(void)
{
    s32 i;

    D_801227C8 = 0;
    i = 0;
    do {
        if ((D_800FD818[i].u0.b.unk0 & 1) && D_800FDF58[i].unk2A == 0x8E) {
            D_800FDF58[i].unk2A = 0;
        }
        i++;
    } while (i < 3);

    func_8006809C();

    i = 0;
    do {
        D_80105AE0[i].unkC = 0;
        D_80105AE0[i].unk178 &= ~0x20;
        i++;
    } while (i < 3);
    D_80122908 = 0;
}
