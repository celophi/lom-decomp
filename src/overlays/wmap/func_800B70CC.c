/* Partial WMAP decompilation: 98.454544% (gcc280_g0). */
#include "common.h"

typedef struct
{
    s16 field_00;
    s16 field_02;
    s16 field_04;
    s16 field_06;
} WmapPair;

extern s32* D_8011CF40;
extern volatile s32 D_80139268;
extern s32 D_801B25DC;
extern volatile WmapPair D_801B2678;
extern s32 D_801B30C0;
extern s32 D_801B30C4;

extern void func_8006AEE0(void);
extern void func_800675F0(s32*, s32, s32, s32, s32, s32, s32, s32, s32, s32);

void func_800B70CC(void)
{
    s32 value;
    s32 timer;
    s32 angle;
    volatile WmapPair* position;

    func_8006AEE0();
    func_800675F0(D_8011CF40, (D_80139268 / 0x10) & 3, 0xA, 0x36, 0x7880, 1, D_801B25DC, 0, 0, -1);
    value = D_801B25DC - 4;
    D_801B25DC = value;
    if (value < 0)
    {
        D_801B25DC = 0;
    }
    angle = D_80139268 + 8;
    timer = D_801B30C4 - 1;
    position = &D_801B2678;
    (void)position->field_04;
    D_80139268 = angle;
    D_801B30C4 = timer;
    if (timer == 0)
    {
        D_801B30C0++;
    }
}
