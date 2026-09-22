#include "wmap_effect_primitives.h"
/* Partial WMAP decompilation: 99.642860% (gcc280_g0). */
#include "common.h"

typedef struct
{
    s16 field_00;
    s16 field_02;
    s16 field_04;
    s16 field_06;
} WmapPair;

extern s32* D_8011CF40;
extern s32 D_80139268;
extern s32 D_801B25DC;
extern volatile WmapPair D_801B2678;
extern s32 D_801B30C0;
extern s32 D_801B30C4;

extern void func_800675F0(s32*, s32, s32, s32, s32, s32, s32, s32, s32, s32);

void func_800B6FEC(void)
{
    s32 value;
    s32 timer;
    volatile WmapPair* position;

    func_8006AEE0();
    func_800675F0(D_8011CF40, (D_80139268 / 0x10) & 3, 0xA, 0x36, 0x7880, 1, D_801B25DC, 0, 0, -1);
    D_80139268 += 8;
    value = D_801B25DC + 2;
    D_801B25DC = value;
    if (value >= 0x82)
    {
        D_801B25DC = 0x81;
    }
    position = &D_801B2678;
    (void)position->field_04;
    timer = D_801B30C4 - 1;
    D_801B30C4 = timer;
    if (timer == 0)
    {
        D_801B30C0++;
    }
}
