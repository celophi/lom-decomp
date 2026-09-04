#include "common.h"

typedef struct {
    u8 pad0[0x2A];
    s16 unk2A;
    u8 pad2C[0x54-0x2C];
} Entry;

extern Entry D_800FDF58[];
extern s32 D_800FE754;
extern s32 D_800F229C;
extern s32 D_8010AE78;
extern s32 D_801229F8;

s32 func_8006751C(s32 arg0);
void func_80098DD4(Entry *arg0);

void func_8009184C(void)
{
    s32 a;
    s32 b;

    if ((D_800FE754 == 0) && (D_800FDF58[0].unk2A == 0) &&
        (D_800F229C == 0) && (D_8010AE78 == 0) && (D_801229F8 & 0x220))
    {
        a = func_8006751C(0);
        b = func_8006751C(1);
        if ((D_800FE754 == 0) && (a == -1) && (b == a))
        {
            func_80098DD4(D_800FDF58);
        }
    }
}
