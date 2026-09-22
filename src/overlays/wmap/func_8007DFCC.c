#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 func_8006683C(s32);
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern s32 D_801B2748;
extern s32 D_801B274C;
extern void func_8007E734(void);

/** @brief Register a callback, set world-map state and color, and begin a two-tick delay. */
void func_8007DFCC(void)
{
    func_8006CAC0(&func_8007E734);
    D_801ADAE0 = 1;
    D_801ADAF4 = 8;
    func_8006683C(0x203050);
    D_801B274C = 2;
    D_801B2748 += 1;
}
