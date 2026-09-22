#include "wmap_view_effects.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_80139244;
extern s32 D_801ADAF4;
extern s32 D_801B2B90;
extern s32 D_801B2B94;
extern void func_80096554(void);
extern void func_80096890(void);
extern void func_80096B40(void);
extern void func_80097234(void);

/** @brief Register four callbacks, set effect color and flags, and begin a one-tick delay. */
void func_800961F4(void)
{
    func_8006CAC0(&func_80096890);
    func_8006CAC0(&func_80096B40);
    func_8006CAC0(&func_80097234);
    D_80139244 = 1;
    func_8006CAC0(&func_80096554);
    func_8006683C(0x701040);
    D_801ADAF4 = 4;
    D_801B2B94 = 1;
    D_801B2B90 += 1;
}
