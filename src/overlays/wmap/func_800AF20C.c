#include "common.h"
#include "sdk/libgte.h"

extern void func_8006CAC0(void (*callback)(void));
extern SVECTOR D_800DCEB8;
extern VECTOR D_80139200;
extern SVECTOR D_80139210;
extern VECTOR D_80139968;
extern s32 D_801B2EF8;
extern s32 D_801B2EFC;
extern void func_800AFB1C(void);

/** @brief Register an effect callback, initialize four vectors, and advance the sequence. */
void func_800AF20C(void)
{
    func_8006CAC0(&func_800AFB1C);
    D_800DCEB8.vx = 0;
    D_800DCEB8.vy = 0;
    D_800DCEB8.vz = 0;
    D_80139200.vx = 0;
    D_80139200.vy = 0;
    D_80139200.vz = 0;
    D_80139210.vx = 0xFA;
    D_80139210.vy = 0;
    D_80139210.vz = 0;
    D_80139968.vx = 0;
    D_80139968.vy = 0x64;
    D_80139968.vz = 0;
    D_801B2EFC = 1;
    D_801B2EF8 += 1;
}
