#include "common.h"

extern void akao_cmd_c2(s32, s32, s32, s32);
extern void func_800A7B78(void);
extern s32 D_800DCEC8[];
extern s32 D_801398D0;
extern s32 D_80139950[];
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_801B2E40;

/** @brief Start audio, compute the coordinate delta, and advance the sequence. */
void func_800A7AF4(void)
{
    akao_cmd_c2(0, 0x1E, 0x30, 0x7F);
    D_801398D0 = 2;
    D_80182D68 = D_800DCEC8[0] - D_80139950[0];
    D_80182D78 = D_800DCEC8[1] - D_80139950[1];
    D_801B2E40 += 1;
    func_800A7B78();
}
