#include "common.h"

extern s32 D_801B25D8;
extern s32 D_801B2E20;
extern s32 D_801B2E24;
extern void func_8006B328(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5,
                          s32 a6, s32 a7, s32 a8, s32 a9, s32 a10, s32 a11,
                          s32 a12, s32 a13, s32 a14, s32 a15, s32 a16);

/** @brief World-map effect spawn: submit a request and tick a refcount. */
void func_800A5318(void)
{
    s32 c;

    func_8006B328(0x50, 0x8C, 2, -1, 0x14, 4, 0x168, 0x19, -0x32, 0x64, -0x32,
                  0x64, 0xB4, 0x81, 0x81, 8, 1);
    D_801B25D8 += 8;
    c = D_801B2E24 - 1;
    D_801B2E24 = c;
    if (c == 0)
    {
        D_801B2E20 += 1;
    }
}
