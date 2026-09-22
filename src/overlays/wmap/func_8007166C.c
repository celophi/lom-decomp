#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_80139244;
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern s32 D_801B24C0;
extern s32 D_801B24C4;
extern void func_80071B48(void);
extern void func_8007228C(void);

/** @brief Register two callbacks, set sequence flags, and begin a one-tick delay. */
void func_8007166C(void)
{
    func_8006CAC0(&func_8007228C);
    D_801ADAF4 = 2;
    D_80139244 = 1;
    func_8006CAC0(&func_80071B48);
    D_801ADAE0 = 1;
    D_801B24C4 = 1;
    D_801B24C0 += 1;
}
