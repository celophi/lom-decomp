#include "common.h"

extern s32 D_801B25E0;
extern s32 D_801B2F80;
extern s32 D_801B2F84;
extern void func_8006B328(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5,
                          s32 a6, s32 a7, s32 a8, s32 a9, s32 a10, s32 a11,
                          s32 a12, s32 a13, s32 a14, s32 a15, s32 a16);

/** @brief World-map effect spawn: submit a request and tick a refcount. */
void func_800B1220(void)
{
    s32 c;

    func_8006B328(0x6E, 0x9B, 2, -1, 3, 2, 0x168, 8, -0x78, 0xF0, -0x78,
                  0xF0, 0x64, 0x81, 0x81, 4, 1);
    D_801B25E0 += 8;
    c = D_801B2F84 - 1;
    D_801B2F84 = c;
    if (c == 0)
    {
        D_801B2F80 += 1;
    }
}
