#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 func_8006683C(s32);
extern s32 D_801ADAF4;
extern s32 D_801B29B0;
extern s32 D_801B29B4;
extern void func_8008C39C(void);
extern void func_8008C4C8(void);

/** @brief Register two callbacks, set world-map color and state, and begin an eight-tick delay. */
void func_8008B524(void)
{
    func_8006CAC0(&func_8008C4C8);
    func_8006CAC0(&func_8008C39C);
    func_8006683C(0x103056);
    D_801ADAF4 = 4;
    D_801B29B4 = 8;
    D_801B29B0 += 1;
}
