#include "common.h"

typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;
} Rec87564;

typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} State87564;

extern State87564 D_80105AE0[];
extern Rec87564 *D_8010A01C;

void func_80087564(Rec87564 *arg0)
{
    D_8010A01C = arg0;
    func_800B2198(D_80105AE0[arg0->unk3A].unk14, D_80105AE0);
}

extern s32 D_8010A030;

/**
 * @return Value of D_8010A030.
 * @see decomp.me (100%) N/A -- trivial 4-instruction leaf function, no scratch needed.
 */
s32 func_800875B4(void)
{
    return D_8010A030;
}
