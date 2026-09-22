#include "wmap_sequence_runtime.h"
#include "common.h"
#include "sdk/libgte.h"

extern SVECTOR D_800DCEB8;
extern VECTOR D_80139200;
extern SVECTOR D_80139210;
extern VECTOR D_80139968;
extern s32 D_8013B29C;
extern s32 D_801B2EF8;
extern s32 D_801B2EFC;
extern void func_8006C0EC(void);

/** @brief Initialize the sequence transforms and start a forty-tick countdown. */
void func_800AEE98(void)
{
    D_8013B29C = 1;
    func_8006CBD8(&func_8006C0EC);
    D_80139210.vx = 5;
    D_80139210.vy = 0;
    D_80139210.vz = 0;
    D_80139968.vx = 0;
    D_80139968.vy = 2;
    D_80139968.vz = 0;
    D_800DCEB8.vx = 0xFA;
    D_800DCEB8.vy = 0;
    D_800DCEB8.vz = 0;
    D_80139200.vx = 0;
    D_80139200.vy = 0x64;
    D_80139200.vz = 0;
    D_801B2EFC = 0x28;
    D_801B2EF8 += 1;
}
